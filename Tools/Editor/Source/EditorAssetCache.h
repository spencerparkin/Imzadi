#pragma once

#include "AssetCache.h"
#include "assimp/scene.h"
#include "assimp/vector3.h"
#include "rapidjson/document.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"

class EditorAssetCache : public Imzadi::AssetCache
{
public:
	EditorAssetCache();
	virtual ~EditorAssetCache();

	void BeginImport(const aiScene* scene);
	void EndImport();

	virtual void Clear() override;

protected:
	virtual Imzadi::Asset* FindAsset(const std::string& assetFile, std::string& error, std::string* key = nullptr) override;

	// Admittedly, JSON is probably the worst format for some (not all) types of assets for a game engine.
	// In particular, vertex buffers should probably be binary and/or compressed, but this works fine for now.
	bool GenerateJsonForMesh(rapidjson::Document& jsonDoc, std::string& error);
	bool GenerateJsonForTexture(rapidjson::Document& jsonDoc, std::string& error);
	bool GenerateJsonForVertexBuffer(rapidjson::Document& jsonDoc, std::string& error);
	bool GenerateJsonForIndexBuffer(rapidjson::Document& jsonDoc, std::string& error);

	Imzadi::Vector3 SpaceConvert(const aiVector3D& vector);
	Imzadi::Vector2 TexCoordConvert(const aiVector3D& vector);

	const aiScene* importScene;
	const aiMesh* importMesh;

	std::vector<Imzadi::Reference<Imzadi::Asset>> generatedAssetArray;
};