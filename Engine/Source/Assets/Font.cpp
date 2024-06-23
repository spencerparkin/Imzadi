#include "Font.h"
#include "Log.h"

using namespace Imzadi;

Font::Font()
{
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
		}

		this->charInfoArray.push_back(charInfo);
	}

	// TODO: Load and own font shader too?

	return true;
}

/*virtual*/ bool Font::Unload()
{
	this->charInfoArray.clear();
	this->textureAtlas.Set(nullptr);
	return true;
}