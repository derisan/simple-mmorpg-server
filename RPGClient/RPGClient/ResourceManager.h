#pragma once

class ResourceManager
{
public:
	using race_type = int16;

	static void LoadAssets();
	static String GetTexName(race_type race);
};

