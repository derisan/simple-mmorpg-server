#pragma once

class ResourceManager
{
public:
	using race_type = int16;

	static void LoadAssets();
	static const Texture* GetTexture(race_type race);

private:
	static std::unordered_map<race_type, Texture> sTextures;
};

