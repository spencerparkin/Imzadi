#include "Texture.h"
#include "Game.h"
#include "Log.h"

using namespace Imzadi;

Texture::Texture()
{
	::memset(&this->textureResourceDesc, 0, sizeof(this->textureResourceDesc));
	this->textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
}

/*virtual*/ Texture::~Texture()
{
}

/*virtual*/ bool Texture::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	// TODO: All this code here is a rough sketch for now and needs to be revisited when ready.

	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected JSON doc to be an object.");
		return false;
	}

	if (!jsonDoc.HasMember("width") || !jsonDoc.HasMember("height"))
	{
		IMZADI_LOG_ERROR("No \"width\" and \"height\" members found.");
		return false;
	}

	if (!jsonDoc["width"].IsUint() || !jsonDoc["height"].IsUint())
	{
		IMZADI_LOG_ERROR("Expected \"width\" and \"height\" members to be unsigned integers.");
		return false;
	}

	this->textureResourceDesc.Width = jsonDoc["width"].GetUint();
	this->textureResourceDesc.Height = jsonDoc["height"].GetUint();

	if (!jsonDoc.HasMember("format") || !jsonDoc["format"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"format\" member to exist and be a string.");
		return false;
	}

	std::string format = jsonDoc["format"].GetString();
	uint32_t texelSizeBytes = 0;
	if (format == "RGBA")
		texelSizeBytes = 4;
	else if (format == "RGB")
		texelSizeBytes = 3;
	else if (format == "A")
		texelSizeBytes = 1;
	else
	{
		IMZADI_LOG_ERROR("Format \"%s\" not recognized or not yet supported.", format.c_str());
		return false;
	}

	// TODO: Not yet sure how to account for mip-maps.
	this->textureResourceDesc.MipLevels = 1;
	if (jsonDoc.HasMember("num_mips") && jsonDoc["num_mips"].IsUint())
		this->textureResourceDesc.MipLevels = jsonDoc["num_mips"].GetUint();

	this->textureResourceDesc.SampleDesc.Count = 1;
	this->textureResourceDesc.DepthOrArraySize = 1;

	if (!Buffer::Load(jsonDoc, assetCache))
		return false;

	return true;
}

/*virtual*/ bool Texture::GetResourceDesc(D3D12_RESOURCE_DESC& resourceDesc)
{
	resourceDesc = this->textureResourceDesc;
	return true;
}