#include "stdafx.h"
#include "ResourceManager.h"

void ResourceManager::Init()
{
	TextureAsset::Register(U"Tex_Player", ASSET_PATH(Player.png));
	TextureAsset::Register(U"Tex_ShopKeeper", ASSET_PATH(ShopKeeper.png));
	TextureAsset::Register(U"Tex_Enemy1", ASSET_PATH(Enemy1.png));

	TextureAsset::Load(U"Tex_Player");
	TextureAsset::Load(U"Tex_ShopKeeper");
	TextureAsset::Load(U"Tex_Enemy1");
}

String ResourceManager::GetTexName(race_type race)
{
	switch (race)
	{
	case Race::Player: return U"Tex_Player";
	case Race::ShopKeeper: return U"Tex_ShopKeeper";
	case Race::Enemy1: return U"Tex_Enemy1";
	default: return U"Null";
	}
}
