#include "stdafx.h"
#include "ChatManager.h"

#include "ActorManager.h"
#include "PacketManager.h"

std::deque<s3d::String> ChatManager::sChats;
s3d::Rect ChatManager::sChatArea = {};
s3d::String ChatManager::sMyChat = {};

constexpr int32 MAX_MESSAGE = 5;

void ChatManager::Init()
{
	sChatArea = Rect{ 10, 144, 300, 40 };
	auto& font = GetFont();
	font.preload(U"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,!?0123456789");
}

void ChatManager::AddChat(const int32 id, StringView mess)
{
	String teller = U"System";

	if (id >= 0)
	{
		teller = ActorManager::GetActorName(id);
	}

	String newMess = teller + U" : " + mess;
	sChats.push_back(std::move(newMess));

	if (sChats.size() > MAX_MESSAGE)
	{
		sChats.pop_front();
		ClearPrint();

		for (const auto& chat : sChats)
		{
			Print << chat;
		}
	}
	else
	{
		Print << sChats.back();
	}
}

void ChatManager::SendCurrentChat()
{
	if (sMyChat.length() > 0)
	{
		//std::string narrowed = sMyChat.narrow();

		//CS_CHAT_PACKET packet = {};
		//packet.size = sizeof(packet);
		//packet.type = CS_CHAT;
		//packet.target_id = 0;
		//packet.chat_type = 0;
		//CopyMemory(packet.mess, narrowed.data(), narrowed.length());
		//PacketManager::SendPacket(&packet, packet.size);
	}

	sMyChat.clear();
}

void ChatManager::TakeUserChat()
{
	sMyChat.ltrim();
	TextInput::UpdateText(sMyChat);
	const String editingText = TextInput::GetEditingText();
	sChatArea.draw(ColorF{ 0.3 });
	GetFont()(sMyChat + U'|' + editingText).draw(sChatArea.stretched(-5));
}

s3d::Font& ChatManager::GetFont()
{
	static Font font = { 20, Typeface::CJK_Regular_KR };
	return font;
}
