#include "stdafx.h"
#include "ResourceManager.h"

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
