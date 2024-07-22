#include "Font.h"
#include "Log.h"
#include "Game.h"

using namespace Imzadi;

Font::Font()
{
	this->indexBuffer = nullptr;
}

/*virtual*/ Font::~Font()
{
}

/*virtual*/ bool Font::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.HasMember("texture") || !jsonDoc["texture"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"texture\" member of JSON data as a string, but found none such.");
		return false;
	}

	std::string textureAssetFile(jsonDoc["texture"].GetString());

	Reference<Asset> asset;
	if (!assetCache->LoadAsset(textureAssetFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load texture file: %s", textureAssetFile.c_str());
		return false;
	}

	this->textureAtlas.SafeSet(asset.Get());
	if (!this->textureAtlas)
	{
		IMZADI_LOG_ERROR("Whatever loaded from file %s was not a texture.", textureAssetFile.c_str());
		return false;
	}

	if (jsonDoc.HasMember("name") && jsonDoc["name"].IsString())
		this->name = jsonDoc["name"].GetString();
	else
		this->name = "?";

	if (!jsonDoc.HasMember("character_array") || !jsonDoc["character_array"].IsArray())
	{
		IMZADI_LOG_ERROR("Expected \"character_array\" member in the JSON data as an array.");
		return false;
	}

	const rapidjson::Value& characterArrayValue = jsonDoc["character_array"];
	this->charInfoArray.clear();
	for (int i = 0; i < characterArrayValue.Size(); i++)
	{
		const rapidjson::Value& charInfoValue = characterArrayValue[i];
		if (!charInfoValue.IsObject())
		{
			IMZADI_LOG_ERROR("Expected char info array entry to be an object.");
			return false;
		}

		CharacterInfo charInfo{};

		if (!charInfoValue.HasMember("no_glyph") || !charInfoValue["no_glyph"].GetBool())
		{
			if (!charInfoValue.HasMember("min_u") || !charInfoValue.HasMember("min_v") ||
				!charInfoValue.HasMember("max_u") || !charInfoValue.HasMember("max_v"))
			{
				IMZADI_LOG_ERROR("Expected UV members of char info array entry.");
				return false;
			}

			if (!charInfoValue["min_u"].IsFloat() || !charInfoValue["min_v"].IsFloat() ||
				!charInfoValue["max_u"].IsFloat() || !charInfoValue["max_v"].IsFloat())
			{
				IMZADI_LOG_ERROR("Expected UV members of char info array entry to be floats.");
				return false;
			}

			charInfo.minUV.x = charInfoValue["min_u"].GetFloat();
			charInfo.minUV.y = charInfoValue["min_v"].GetFloat();
			charInfo.maxUV.x = charInfoValue["max_u"].GetFloat();
			charInfo.maxUV.y = charInfoValue["max_v"].GetFloat();

			charInfo.width = charInfo.maxUV.x - charInfo.minUV.x;
			charInfo.height = charInfo.maxUV.y - charInfo.minUV.y;
		}

		if (charInfoValue.HasMember("pen_offset_x") && charInfoValue["pen_offset_x"].IsFloat())
			charInfo.penOffset.x = charInfoValue["pen_offset_x"].GetFloat();

		if (charInfoValue.HasMember("pen_offset_y") && charInfoValue["pen_offset_y"].IsFloat())
			charInfo.penOffset.y = charInfoValue["pen_offset_y"].GetFloat();

		if (charInfoValue.HasMember("advance") && charInfoValue["advance"].IsFloat())
			charInfo.advance = charInfoValue["advance"].GetFloat();

		this->charInfoArray.push_back(charInfo);
	}

	if (!assetCache->LoadAsset("Shaders/Text.shader", asset))
	{
		IMZADI_LOG_ERROR("Failed to load text shader.");
		return false;
	}

	this->textShader.SafeSet(asset.Get());
	if (!this->textShader)
	{
		IMZADI_LOG_ERROR("Loaded something other than a shader for the text shader.");
		return false;
	}

	if (!assetCache->LoadAsset("Shaders/LegibleText.shader", asset))
	{
		IMZADI_LOG_ERROR("Failed to load legible text shader.");
		return false;
	}

	this->legibleTextShader.SafeSet(asset.Get());
	if (!this->legibleTextShader)
	{
		IMZADI_LOG_ERROR("Loaded something other than a shader for the legible text shader.");
		return false;
	}

	uint32_t maxCharacters = 1024;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(uint16_t) * 6 * maxCharacters;

	std::vector<uint16_t> indexArray;
	for (int i = 0; i < maxCharacters; i++)
	{
		int j = i * 4;

		indexArray.push_back(j + 0);
		indexArray.push_back(j + 1);
		indexArray.push_back(j + 2);

		indexArray.push_back(j + 0);
		indexArray.push_back(j + 2);
		indexArray.push_back(j + 3);
	}

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = indexArray.data();

	HRESULT result = Game::Get()->GetDevice()->CreateBuffer(&bufferDesc, &subResourceData, &this->indexBuffer);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create index buffer for font asset.  Error code: %d", result);
		return false;
	}

	return true;
}

/*virtual*/ bool Font::Unload()
{
	this->charInfoArray.clear();
	this->textureAtlas.Set(nullptr);
	SafeRelease(this->indexBuffer);
	return true;
}

bool Font::GetCharInfo(char ch, CharacterInfo& info) const
{
	if (!(0 <= ch && ch < this->charInfoArray.size()))
		return false;

	info = this->charInfoArray[ch];
	return true;
}

Shader* Font::GetShader(bool legible)
{
	if (legible)
		return this->legibleTextShader.Get();

	return this->textShader.Get();
}