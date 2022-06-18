#include "stdafx.h"
#include "DBConnection.h"

#include <chrono>

#include "IocpBase.h"
#include "Session.h"

namespace mk
{
	SQLHENV DBConnection::sHenv = SQL_NULL_HANDLE;
	SQLHDBC DBConnection::sHdbc = SQL_NULL_HANDLE;
	SQLHSTMT DBConnection::sHstmt = SQL_NULL_HANDLE;
	HANDLE DBConnection::sIocp = INVALID_HANDLE_VALUE;
	concurrency::concurrent_queue<mk::DBJob> DBConnection::sJobs;
	std::unique_ptr<mk::OverlappedPool> DBConnection::sPool = nullptr;

	std::wstring s2ws(const std::string& s)
	{
		unsigned int len;
		unsigned int slength = static_cast<unsigned int>(s.length()) + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring ret(buf);
		delete[] buf;
		return ret;
	}

	void DBConnection::Init(HANDLE iocpHandle)
	{
		sPool = std::make_unique<mk::OverlappedPool>(3000);

		sIocp = iocpHandle;

		setlocale(LC_ALL, "korean");

		SQLRETURN retcode
			= SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sHenv);
		if (NOT (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
		{
			handleDiagnosticRecord(sHenv, SQL_HANDLE_ENV, retcode);
			MK_ASSERT(false);
		}

		retcode
			= SQLSetEnvAttr(sHenv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
		if (NOT(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
		{
			handleDiagnosticRecord(sHenv, SQL_HANDLE_ENV, retcode);
			MK_ASSERT(false);
		}

		retcode
			= SQLAllocHandle(SQL_HANDLE_DBC, sHenv, &sHdbc);
		if (NOT (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
		{
			handleDiagnosticRecord(sHdbc, SQL_HANDLE_DBC, retcode);
			MK_ASSERT(false);
		}

		retcode
			= SQLConnect(sHdbc, (SQLWCHAR*)L"2016180007_MMORPG", SQL_NTS, 
				(SQLWCHAR*)NULL, 0, NULL, 0);
		if (NOT (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
		{
			handleDiagnosticRecord(sHdbc, SQL_HANDLE_DBC, retcode);
			MK_ASSERT(false);
		}

		retcode
			= SQLAllocHandle(SQL_HANDLE_STMT, sHdbc, &sHstmt);
		if (NOT (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
		{
			handleDiagnosticRecord(sHstmt, SQL_HANDLE_STMT, retcode);
			MK_ASSERT(false);
		}

		MK_SLOG("DB connection complete.");
	}

	void DBConnection::Shutdown()
	{
		SQLFreeStmt(sHstmt, SQL_CLOSE);
		SQLFreeHandle(SQL_HANDLE_STMT, sHstmt);
		SQLFreeHandle(SQL_HANDLE_DBC, sHdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, sHenv);
	}

	void DBConnection::Run()
	{
		using namespace std::chrono;

		while (true)
		{
			DBJob job = {};
			bool bPop = sJobs.try_pop(job);

			if (NOT bPop)
			{
				std::this_thread::sleep_for(33ms);
				continue;
			}

			executeJob(job);
		}
	}

	void DBConnection::PushJob(const int id, DBJobType type)
	{
		DBJob job = { id, type };
		sJobs.push(job);
	}

	void DBConnection::PushOverEx(OVERLAPPEDEX* overex)
	{
		sPool->Push(overex);
	}

	void DBConnection::executeJob(const DBJob& job)
	{
		switch (job.Type)
		{
		case DBJobType::GetUserInfo:
			execGetUserInfo(job);
			break;

		case DBJobType::UpdateUserInfo:
			execUpdateUserInfo(job);
			break;

		default:
			MK_ASSERT(false);
			break;
		}
	}

	void DBConnection::execGetUserInfo(const DBJob& job)
	{
		UserInfo user = {};

		const auto& name = gClients[job.ID]->GetName();
		std::wstring query = L"EXEC GetUserInfo " + s2ws(name);

		SQLRETURN retcode = SQLExecDirect(sHstmt, (SQLWCHAR*)query.data(), SQL_NTS);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			SQLBindCol(sHstmt, 1, SQL_C_WCHAR, user.Name, 20, nullptr);
			SQLBindCol(sHstmt, 2, SQL_C_SHORT, &user.X, 5, nullptr);
			SQLBindCol(sHstmt, 3, SQL_C_SHORT, &user.Y, 5, nullptr);
			SQLBindCol(sHstmt, 4, SQL_C_SHORT, &user.Level, 5, nullptr);
			SQLBindCol(sHstmt, 5, SQL_C_LONG, &user.HP, 10, nullptr);
			SQLBindCol(sHstmt, 6, SQL_C_SHORT, &user.Race, 5, nullptr);
			SQLBindCol(sHstmt, 7, SQL_C_LONG, &user.Exp, 10, nullptr);

			retcode = SQLFetch(sHstmt);
			if (retcode == SQL_NO_DATA)
			{
				auto overex = sPool->Pop();
				ZeroMemory(overex, sizeof(OVERLAPPEDEX));
				overex->OpType = OperationType::DB_LOGIN_NO_INFO;
				PostQueuedCompletionStatus(sIocp, 1, job.ID, &overex->Overlapped);
			}

			else if (retcode == SQL_SUCCESS_WITH_INFO)
			{
				auto overex = sPool->Pop();
				ZeroMemory(overex, sizeof(OVERLAPPEDEX));
				overex->OpType = OperationType::DB_LOGIN_WITH_INFO;
				CopyMemory(overex->SendBuffer, &user, sizeof(user));
				PostQueuedCompletionStatus(sIocp, 1, job.ID, &overex->Overlapped);
			}
		}
		else
		{
			handleDiagnosticRecord(sHstmt, SQL_HANDLE_STMT, retcode);
		}

		SQLCloseCursor(sHstmt);
	}

	void DBConnection::execUpdateUserInfo(const DBJob& job)
	{
		auto actor = gClients[job.ID];

		{
			ReadLockGuard guard = { actor->ActorLock };
			if (NOT actor->IsActive())
			{
				return;
			}
		}

		(actor->ActorLock).ReadLock();
		std::wstring query = L"EXEC UpdateUserInfo "
			+ s2ws(actor->GetName())
			+ L"," + std::to_wstring(actor->GetX())
			+ L"," + std::to_wstring(actor->GetY())
			+ L"," + std::to_wstring(actor->GetLevel())
			+ L"," + std::to_wstring(actor->GetCurrentHP())
			+ L"," + std::to_wstring(actor->GetRace())
			+ L"," + std::to_wstring(actor->GetExp());
		(actor->ActorLock).ReadUnlock();

		SQLRETURN retcode = SQLExecDirect(sHstmt, (SQLWCHAR*)query.data(), SQL_NTS);
		if (NOT (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO))
		{
			handleDiagnosticRecord(sHstmt, SQL_HANDLE_STMT, retcode);
		}
	}

	void DBConnection::handleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode)
	{
		SQLSMALLINT iRec = 0;
		SQLINTEGER iError;
		WCHAR wszMessage[1000];
		WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
		if (retCode == SQL_INVALID_HANDLE) {
			fwprintf(stderr, L"Invalid handle!\n");
			return;
		}
		while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
			(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
			if (wcsncmp(wszState, L"01004", 5)) {
				fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
			}
		}
	}
}
