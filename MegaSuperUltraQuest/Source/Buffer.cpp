#include "Buffer.h"

Buffer::Buffer()
{
}

/*virtual*/ Buffer::~Buffer()
{
}

/*virtual*/ bool Buffer::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	return false;
}

/*virtual*/ bool Buffer::Unload()
{
	return false;
}