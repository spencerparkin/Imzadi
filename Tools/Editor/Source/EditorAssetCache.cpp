#include "App.h"
#include "Frame.h"
#include <wx/msgdlg.h>
#include "EditorAssetCache.h"
#include "Assets/RenderMesh.h"
#include "Assets/SkinnedRenderMesh.h"
#include "Assets/Buffer.h"
#include "Assets/Texture.h"
#include "Assets/SkinWeights.h"
#include "Assets/Skeleton.h"
#include "Math/AxisAlignedBoundingBox.h"

EditorAssetCache::EditorAssetCache()
{
	this->importScene = nullptr;
	this->importMesh = nullptr;
	this->rigAndAnimate = false;
}

/*virtual*/ EditorAssetCache::~EditorAssetCache()
{
}

/*virtual*/ void EditorAssetCache::Clear()
{
	AssetCache::Clear();

	for (auto asset : this->generatedAssetArray)
		asset->Unload();

	this->generatedAssetArray.clear();
}

void EditorAssetCache::BeginImport(const aiScene* scene, bool rigAndAnimate)
{
	this->importScene = scene;
	this->importMesh = nullptr;
	this->rigAndAnimate = rigAndAnimate;
}

void EditorAssetCache::EndImport()
{
	this->importScene = nullptr;
	this->importMesh = nullptr;
}

/*virtual*/ Imzadi::Asset* EditorAssetCache::FindAsset(const std::string& assetFile, std::string& error, std::string* key /*= nullptr*/)
{
	if (!this->importScene)
		return AssetCache::FindAsset(assetFile, error, key);

	std::string assetType = assetFile;
	std::string assetName;
	int colonOffset = assetFile.find_first_of(':');
	if (colonOffset >= 0)
	{
		assetType = assetFile.substr(0, colonOffset);
		assetName = assetFile.substr(colonOffset + 1, assetFile.size() - colonOffset - 1);
	}

	if (assetType == "Mesh")
	{
		this->importMesh = nullptr;
		for (int i = 0; i < this->importScene->mNumMeshes; i++)
		{
			const aiMesh* mesh = this->importScene->mMeshes[i];
			if (std::string(mesh->mName.C_Str()) == assetName)
			{
				this->importMesh = mesh;
				break;
			}
		}

		if (!this->importMesh)
		{
			error = std::format("No mesh with name {} found.", assetName.c_str());
			return nullptr;
		}

		rapidjson::Document jsonDoc;
		if (!this->GenerateJsonForMesh(jsonDoc, error))
			return nullptr;

		Imzadi::RenderMeshAsset* renderMeshAsset = nullptr;
		if (this->rigAndAnimate)
			renderMeshAsset = new Imzadi::SkinnedRenderMesh();
		else
			renderMeshAsset = new Imzadi::RenderMeshAsset();

		if (renderMeshAsset->Load(jsonDoc, error, this))
			this->generatedAssetArray.push_back(renderMeshAsset);
		else
		{
			delete renderMeshAsset;
			renderMeshAsset = nullptr;
		}
		
		this->importMesh = nullptr;
		return renderMeshAsset;
	}
	else if (assetType == "Texture")
	{
		rapidjson::Document jsonDoc;
		if (!this->GenerateJsonForTexture(jsonDoc, error))
			return nullptr;

		auto texture = new Imzadi::Texture();
		if (texture->Load(jsonDoc, error, this))
			this->generatedAssetArray.push_back(texture);
		else
		{
			delete texture;
			texture = nullptr;
		}

		return texture;
	}
	else if (assetType == "VertexBuffer")
	{
		rapidjson::Document jsonDoc;
		if (!this->GenerateJsonForVertexBuffer(jsonDoc, error))
			return nullptr;

		auto buffer = new Imzadi::Buffer();
		if (buffer->Load(jsonDoc, error, this))
			this->generatedAssetArray.push_back(buffer);
		else
		{
			delete buffer;
			buffer = nullptr;
		}

		return buffer;
	}
	else if (assetType == "IndexBuffer")
	{
		rapidjson::Document jsonDoc;
		if (!this->GenerateJsonForIndexBuffer(jsonDoc, error))
			return nullptr;

		auto buffer = new Imzadi::Buffer();
		if (buffer->Load(jsonDoc, error, this))
			this->generatedAssetArray.push_back(buffer);
		else
		{
			delete buffer;
			buffer = nullptr;
		}

		return buffer;
	}
	else if (assetType == "SkinWeights")
	{
		// TODO: Load existing weights if there already is one?
		auto skinWeights = new Imzadi::SkinWeights();
		return skinWeights;
	}
	else if (assetType == "Skeleton")
	{
		// TODO: Load existing skeleton if there already is one?
		auto skeleton = new Imzadi::Skeleton();
		skeleton->MakeBasicBiped();
		skeleton->UpdateCachedTransforms(Imzadi::BoneTransformType::BIND_POSE);
		skeleton->ResetCurrentPose();
		return skeleton;
	}
	else if (assetType == "Animation")
	{
		//...
	}

	return AssetCache::FindAsset(assetFile, error, key);
}

bool EditorAssetCache::GenerateJsonForMesh(rapidjson::Document& jsonDoc, std::string& error)
{
	if (!this->importMesh || !this->importScene)
		return false;
	
	Imzadi::AxisAlignedBoundingBox aabb;
	aabb.minCorner = this->SpaceConvert(this->importMesh->mAABB.mMin);
	aabb.maxCorner = this->SpaceConvert(this->importMesh->mAABB.mMax);

	rapidjson::Value boundingBoxValue;
	Imzadi::Asset::SaveBoundingBox(boundingBoxValue, aabb, &jsonDoc);

	jsonDoc.SetObject();
	jsonDoc.AddMember("bounding_box", boundingBoxValue, jsonDoc.GetAllocator());
	jsonDoc.AddMember("primitive_type", rapidjson::Value().SetString("TRIANGLE_LIST"), jsonDoc.GetAllocator());
	jsonDoc.AddMember("shader", rapidjson::Value().SetString("Shaders/Standard.shader", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("shadow_shader", rapidjson::Value().SetString("Shaders/StandardShadow.shader", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("texture", rapidjson::Value().SetString("Texture", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("index_buffer", rapidjson::Value().SetString("IndexBuffer", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("vertex_buffer", rapidjson::Value().SetString("VertexBuffer", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	
	if (this->rigAndAnimate)
	{
		jsonDoc.AddMember("skeleton", rapidjson::Value().SetString("Skeleton", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
		jsonDoc.AddMember("skin_weights", rapidjson::Value().SetString("SkinWeights", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
		jsonDoc.AddMember("position_offset", rapidjson::Value().SetInt(0), jsonDoc.GetAllocator());
		jsonDoc.AddMember("normal_offset", rapidjson::Value().SetInt(20), jsonDoc.GetAllocator());
	}

	return true;
}

bool EditorAssetCache::GenerateJsonForTexture(rapidjson::Document& jsonDoc, std::string& error)
{
	if (!this->importMesh || !this->importScene)
		return false;

	if (this->importScene->mNumMaterials <= this->importMesh->mMaterialIndex)
		return false;

	aiMaterial* material = this->importScene->mMaterials[this->importMesh->mMaterialIndex];
	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 1)
		return false;

	aiString texturePath;
	if (aiReturn_SUCCESS != material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
		return false;

	jsonDoc.SetObject();
	jsonDoc.AddMember("flip_vertical", rapidjson::Value().SetBool(true), jsonDoc.GetAllocator());
	jsonDoc.AddMember("image_file", rapidjson::Value().SetString(texturePath.C_Str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());

	return true;
}

bool EditorAssetCache::GenerateJsonForVertexBuffer(rapidjson::Document& jsonDoc, std::string& error)
{
	if (!this->importMesh)
		return false;

	if (this->importMesh->mNumVertices == 0)
		return false;

	if (this->importMesh->mNumUVComponents[0] != 2)
		return false;

	rapidjson::Value vertexBufferValue;
	vertexBufferValue.SetArray();

	for (int i = 0; i < this->importMesh->mNumVertices; i++)
	{
		Imzadi::Vector3 position = this->SpaceConvert(this->importMesh->mVertices[i]);
		Imzadi::Vector2 texCoords = this->TexCoordConvert(importMesh->mTextureCoords[0][i]);
		Imzadi::Vector3 normal = this->SpaceConvert(this->importMesh->mNormals[i]);

		if (!normal.Normalize())
			return false;

		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.x), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.y), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.z), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(texCoords.x), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(texCoords.y), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.x), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.y), jsonDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.z), jsonDoc.GetAllocator());
	}

	jsonDoc.SetObject();
	jsonDoc.AddMember("bind", rapidjson::Value().SetString("vertex", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("stride", rapidjson::Value().SetInt(8), jsonDoc.GetAllocator());
	jsonDoc.AddMember("type", rapidjson::Value().SetString("float", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("buffer", vertexBufferValue, jsonDoc.GetAllocator());

	if (this->rigAndAnimate)
	{
		jsonDoc.AddMember("usage", rapidjson::Value().SetString("dynamic", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
		jsonDoc.AddMember("bare_buffer", rapidjson::Value().SetBool(true), jsonDoc.GetAllocator());
	}

	return true;
}

bool EditorAssetCache::GenerateJsonForIndexBuffer(rapidjson::Document& jsonDoc, std::string& error)
{
	if (!this->importMesh)
		return false;

	if (this->importMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
		return false;

	if (this->importMesh->mNumFaces == 0)
		return false;

	rapidjson::Value indexBufferValue;
	indexBufferValue.SetArray();

	for (int i = 0; i < this->importMesh->mNumFaces; i++)
	{
		const aiFace* face = &this->importMesh->mFaces[i];
		if (face->mNumIndices != 3)
			return false;

		for (int j = 0; j < face->mNumIndices; j++)
		{
			unsigned int index = face->mIndices[j];
			indexBufferValue.PushBack(rapidjson::Value().SetUint(index), jsonDoc.GetAllocator());

			if (index != static_cast<unsigned int>(static_cast<unsigned short>(index)))
				return false;
		}
	}

	jsonDoc.SetObject();
	jsonDoc.AddMember("bind", rapidjson::Value().SetString("index", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("stride", rapidjson::Value().SetInt(1), jsonDoc.GetAllocator());
	jsonDoc.AddMember("type", rapidjson::Value().SetString("ushort", jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
	jsonDoc.AddMember("buffer", indexBufferValue, jsonDoc.GetAllocator());

	return true;
}

Imzadi::Vector3 EditorAssetCache::SpaceConvert(const aiVector3D& vector)
{
	// I *think* AssImp uses the same coordinate system convensions that I do.
	return Imzadi::Vector3(
		vector.x,
		vector.y,
		vector.z
	);
}

Imzadi::Vector2 EditorAssetCache::TexCoordConvert(const aiVector3D& vector)
{
	return Imzadi::Vector2(
		vector.x,
		vector.y
	);
}