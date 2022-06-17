#include "stdafx.h"
#include "ResourceManager.h"

void ResourceManager::Init()
{
	TextureAsset::Register(U"Tex_Player", ASSET_PATH(Player.png));
	TextureAsset::Register(U"Tex_ShopKeeper", ASSET_PATH(ShopKeeper.png));
	TextureAsset::Register(U"Tex_Enemy1", ASSET_PATH(Enemy1.png));
	TextureAsset::Register(U"Tex_Enemy2", ASSET_PATH(Enemy2.png));
	TextureAsset::Register(U"Tex_Enemy3", ASSET_PATH(Enemy3.png));
	TextureAsset::Register(U"Tex_Enemy4", ASSET_PATH(Enemy4.png));

	TextureAsset::Load(U"Tex_Player");
	TextureAsset::Load(U"Tex_ShopKeeper");
	TextureAsset::Load(U"Tex_Enemy1");
	TextureAsset::Load(U"Tex_Enemy2");
	TextureAsset::Load(U"Tex_Enemy3");
	TextureAsset::Load(U"Tex_Enemy4");
}

String ResourceManager::GetTexName(race_type race)
{
	switch (race)
	{
	case Race::Player: return U"Tex_Player";
	case Race::ShopKeeper: return U"Tex_ShopKeeper";
	case Race::Enemy1: return U"Tex_Enemy1";
	case Race::Enemy2: return U"Tex_Enemy2";
	case Race::Enemy3: return U"Tex_Enemy3";
	case Race::Enemy4: return U"Tex_Enemy4";
	default: return U"Null";
	}
}
