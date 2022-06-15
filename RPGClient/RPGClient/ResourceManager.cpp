#include "stdafx.h"
#include "ResourceManager.h"

std::unordered_map<ResourceManager::race_type, s3d::Texture> ResourceManager::sTextures;

void ResourceManager::LoadAssets()
{
	TextureAsset::Register(U"Tex_Character", ASSET_PATH(Character.png));
	TextureAsset::Load(U"Tex_Character");
}

String ResourceManager::GetTexName(race_type race)
{
	switch (race)
	{
	case 0: return U"Tex_Character";
	default: return U"Null";
	}
}
