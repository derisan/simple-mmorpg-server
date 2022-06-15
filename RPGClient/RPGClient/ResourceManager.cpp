#include "stdafx.h"
#include "ResourceManager.h"

std::unordered_map<ResourceManager::race_type, s3d::Texture> ResourceManager::sTextures;

void ResourceManager::LoadAssets()
{
	sTextures[0] = Texture{ ASSET_PATH(Character.png) };
}

const Texture* ResourceManager::GetTexture(race_type race)
{
	switch (race)
	{
	case 0: return &sTextures[0];
	default: return nullptr;
	}
}
