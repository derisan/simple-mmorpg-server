#pragma once

class ChatManager
{
public:
	static void Init();
	static void AddChat(const int32 id, StringView mess);
	static void AddChat(const int32 id, std::string_view mess);
	static void SendCurrentChat();
	static void TakeUserChat();

	static Font& GetFont();

private:
	static std::deque<String> sChats;
	static Rect sChatArea;
	static String sMyChat;
};

