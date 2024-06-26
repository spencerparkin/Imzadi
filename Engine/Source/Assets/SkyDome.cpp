#include "SkyDome.h"
#include "RenderObjects/SkyDomeRenderObject.h"
#include "Log.h"

using namespace Imzadi;

SkyDome::SkyDome()
{
}

/*virtual*/ SkyDome::~SkyDome()
{
}

/*virtual*/ bool SkyDome::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!RenderMeshAsset::Load(jsonDoc, assetCache))
		return false;

	if (!jsonDoc.HasMember("cube_texture") || !jsonDoc["cube_texture"].IsString())
	{
		IMZADI_LOG_ERROR("No \"cube_texture\" found in JSON or it wasn't a string.");
		return false;
	}

	std::string cubeTextureFile = jsonDoc["cube_texture"].GetString();
	Reference<Asset> asset;
	if (!assetCache->LoadAsset(cubeTextureFile, asset))
		return false;

	this->cubeTexture.SafeSet(asset.Get());
	if (!this->cubeTexture)
	{
		IMZADI_LOG_ERROR("Whatever loaded for the cube texture wasn't a cube texture.");
		return false;
	}

	return true;
}

/*virtual*/ bool SkyDome::Unload()
{
	RenderMeshAsset::Unload();
	this->cubeTexture.Reset();
	return true;
}

/*virtual*/ bool SkyDome::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	auto skyDomeRenderObj = new SkyDomeRenderObject();
	skyDomeRenderObj->SetSkyDome(this);
	renderObject.Set(skyDomeRenderObj);
	return true;
}