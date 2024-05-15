#include "Texture.h"

Texture::Texture()
{
}

/*virtual*/ Texture::~Texture()
{
}

/*virtual*/ bool Texture::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	return false;
}

/*virtual*/ bool Texture::Unload()
{
	return false;
}