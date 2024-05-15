#include "Shader.h"

Shader::Shader()
{
}

/*virtual*/ Shader::~Shader()
{
}

/*virtual*/ bool Shader::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	return false;
}

/*virtual*/ bool Shader::Unload()
{
	return false;
}