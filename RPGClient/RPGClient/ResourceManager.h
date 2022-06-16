#pragma once

class ResourceManager
{
public:
	using race_type = int16;

	static void Init();
	static String GetTexName(race_type race);
};

