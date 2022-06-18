#pragma once

#include <concurrent_queue.h>
#include <sqlext.h>

#include "OverlappedEx.h"

namespace mk
{
	struct UserInfo
	{
		SQLWCHAR Name[20];
		SQLSMALLINT X;
		SQLSMALLINT Y;
		SQLSMALLINT Level;
		SQLINTEGER HP;
		SQLSMALLINT Race;
		SQLINTEGER Exp;
	};

	enum class DBJobType
	{
		NONE,
		GetUserInfo,
		UpdateUserInfo,
	};

	struct DBJob
	{
		int ID = 0;
		DBJobType Type = DBJobType::NONE;
	};

	class DBConnection
	{
	public:
		static void Init(HANDLE iocpHandle);
		static void Shutdown();
		static void Run();
		static void PushJob(const int id, DBJobType type);
		static void PushOverEx(OVERLAPPEDEX* overex);

	private:
		static void executeJob(const DBJob& job);
		static void execGetUserInfo(const DBJob& job);
		static void execUpdateUserInfo(const DBJob& job);

		static void handleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE retCode);

	private:
		static SQLHENV sHenv;
		static SQLHDBC sHdbc;
		static SQLHSTMT sHstmt;
		static HANDLE sIocp;
		static concurrency::concurrent_queue<DBJob> sJobs;
		static std::unique_ptr<OverlappedPool> sPool;
	};
}

