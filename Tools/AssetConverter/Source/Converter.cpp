#include "Converter.h"
#include <wx/filename.h>
#include <fstream>
#include "assimp/postprocess.h"
#include "assimp/config.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "AssetCache.h"
#include "Assets/SkinWeights.h"

Converter::Converter()
{
}

/*virtual*/ Converter::~Converter()
{
}

bool Converter::Convert(const wxString& assetFile, wxString& error)
{
	wxFileName fileName(assetFile);
	wxString assetFolder = fileName.GetPath();

	this->importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0);

	// TODO: Can we alter the flags here to retain the animation information and then use it in the conversion process?
	//       The rig information is also not retained.  Even when I retain it, however, I can't find any bone weights.
	const aiScene* scene = importer.ReadFile(assetFile.c_str(), aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_GlobalScale);
	if (!scene)
	{
		error = wxString::Format("%s:\nImport error: %s\n", assetFile.c_str(), importer.GetErrorString());
		return false;
	}

	if (scene->mNumMeshes == 0)
	{
		error = "No meshes found in file: " + assetFile;
		return false;
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		if (!this->ConvertMesh(mesh, scene, assetFolder, error))
			return false;
	}
	
	return true;
}

bool Converter::ConvertMesh(const aiMesh* mesh, const aiScene* scene, const wxString& assetFolder, wxString& error)
{
	wxFileName meshFileName;
	meshFileName.SetPath(assetFolder);
	meshFileName.SetName(mesh->mName.C_Str());
	meshFileName.SetExt(mesh->HasBones() ? ".skinned_render_mesh" : ".render_mesh");

	wxFileName textureFileName;
	textureFileName.SetPath(assetFolder);
	textureFileName.SetName(mesh->mName.C_Str());
	textureFileName.SetExt(".texture");

	wxFileName vertexBufferFileName;
	vertexBufferFileName.SetPath(assetFolder);
	vertexBufferFileName.SetName(wxString(mesh->mName.C_Str()) + "_Vertices");
	vertexBufferFileName.SetExt(".buffer");

	wxFileName indexBufferFileName;
	indexBufferFileName.SetPath(assetFolder);
	indexBufferFileName.SetName(wxString(mesh->mName.C_Str()) + "_Indices");
	indexBufferFileName.SetExt(".buffer");

	rapidjson::Document meshDoc;
	meshDoc.SetObject();

	Imzadi::AxisAlignedBoundingBox aabb;
	aabb.minCorner = this->ConvertVector(mesh->mAABB.mMin);
	aabb.maxCorner = this->ConvertVector(mesh->mAABB.mMax);

	rapidjson::Value boundingBoxValue;
	Imzadi::Asset::SaveBoundingBox(boundingBoxValue, aabb, &meshDoc);

	meshDoc.AddMember("bounding_box", boundingBoxValue, meshDoc.GetAllocator());
	meshDoc.AddMember("primitive_type", rapidjson::Value().SetString("TRIANGLE_LIST"), meshDoc.GetAllocator());
	meshDoc.AddMember("shader", rapidjson::Value().SetString("Shaders/Standard.shader", meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("shadow_shader", rapidjson::Value().SetString("Shaders/StandardShadow.shader", meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("texture", rapidjson::Value().SetString(this->MakeAssetFileReference(textureFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("index_buffer", rapidjson::Value().SetString(this->MakeAssetFileReference(indexBufferFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("vertex_buffer", rapidjson::Value().SetString(this->MakeAssetFileReference(vertexBufferFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

	if (scene->mNumMaterials <= mesh->mMaterialIndex)
	{
		error = "Bad material index";
		return false;
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 1)
	{
		error = "Material does not have exactly one diffuse texture.";
		return false;
	}

	aiString texturePath;
	if (aiReturn_SUCCESS != material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
	{
		error = "Failed to acquire texture path from material.";
		return false;
	}

	rapidjson::Document textureDoc;
	textureDoc.SetObject();
	textureDoc.AddMember("flip_vertical", rapidjson::Value().SetBool(true), textureDoc.GetAllocator());
	textureDoc.AddMember("image_file", rapidjson::Value().SetString(texturePath.C_Str(), textureDoc.GetAllocator()), textureDoc.GetAllocator());

	if (mesh->mNumVertices == 0)
	{
		error = "No vertices found.";
		return false;
	}

	if (mesh->mNumUVComponents[0] != 2)
	{
		error = "Expected exactly 2 UV components in first channel.";
		return false;
	}

	rapidjson::Document verticesDoc;
	verticesDoc.SetObject();

	rapidjson::Value vertexBufferValue;
	vertexBufferValue.SetArray();

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		Imzadi::Vector3 position = this->ConvertVector(mesh->mVertices[i]);
		Imzadi::Vector2 texCoords = this->ConvertTexCoords(mesh->mTextureCoords[0][i]);
		Imzadi::Vector3 normal = this->ConvertVector(mesh->mNormals[i]);

		if (!normal.Normalize())
			return false;

		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.x), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.y), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.z), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(texCoords.x), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(texCoords.y), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.x), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.y), verticesDoc.GetAllocator());
		vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.z), verticesDoc.GetAllocator());
	}

	verticesDoc.AddMember("bind", rapidjson::Value().SetString("vertex", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
	verticesDoc.AddMember("stride", rapidjson::Value().SetInt(8), verticesDoc.GetAllocator());
	verticesDoc.AddMember("type", rapidjson::Value().SetString("float", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
	verticesDoc.AddMember("buffer", vertexBufferValue, verticesDoc.GetAllocator());

	if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
	{
		error = "Only triangle primitive currently supported.";
		return false;
	}

	if (mesh->mNumFaces == 0)
	{
		error = "No faces found.";
		return false;
	}

	rapidjson::Document indicesDoc;
	indicesDoc.SetObject();

	rapidjson::Value indexBufferValue;
	indexBufferValue.SetArray();

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace* face = &mesh->mFaces[i];
		if (face->mNumIndices != 3)
		{
			error = "Expected exactly 3 indices in face.";
			return false;
		}

		for (int j = 0; j < face->mNumIndices; j++)
		{
			unsigned int index = face->mIndices[j];
			indexBufferValue.PushBack(rapidjson::Value().SetUint(index), indicesDoc.GetAllocator());

			if (index != static_cast<unsigned int>(static_cast<unsigned short>(index)))
			{
				error = "Index doesn't fit in an unsigned short.";
				return false;
			}
		}
	}

	indicesDoc.AddMember("bind", rapidjson::Value().SetString("index", indicesDoc.GetAllocator()), indicesDoc.GetAllocator());
	indicesDoc.AddMember("stride", rapidjson::Value().SetInt(1), indicesDoc.GetAllocator());
	indicesDoc.AddMember("type", rapidjson::Value().SetString("ushort", indicesDoc.GetAllocator()), indicesDoc.GetAllocator());
	indicesDoc.AddMember("buffer", indexBufferValue, indicesDoc.GetAllocator());

	if (mesh->HasBones())
	{
		wxFileName skeletonFileName;
		skeletonFileName.SetPath(assetFolder);
		skeletonFileName.SetName(mesh->mName.C_Str());
		skeletonFileName.SetExt(".skeleton");

		wxFileName skinWeightsFileName;
		skinWeightsFileName.SetPath(assetFolder);
		skinWeightsFileName.SetName(mesh->mName.C_Str());
		skinWeightsFileName.SetExt(".skin_weights");

		meshDoc.AddMember("position_offset", rapidjson::Value().SetInt(0), meshDoc.GetAllocator());
		meshDoc.AddMember("normal_offset", rapidjson::Value().SetInt(20), meshDoc.GetAllocator());
		meshDoc.AddMember("skeleton", rapidjson::Value().SetString(this->MakeAssetFileReference(skeletonFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
		meshDoc.AddMember("skin_weights", rapidjson::Value().SetString(this->MakeAssetFileReference(skinWeightsFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

		rapidjson::Document skeletonDoc;
		skeletonDoc.SetObject();

		// TODO: Can we get ahold of a file that imports with bones?  My FBX files don't, even if I rig a character.
		// TODO: Even if I can find such a file, will the bone hierarchy in AssImp format translate to the skeleton structure in my engine?

		rapidjson::Document skinWeightsDoc;
		skinWeightsDoc.SetObject();

		Imzadi::SkinWeights skinWeights;
		skinWeights.SetNumVertices(mesh->mNumVertices);

		// TODO: Do we need the armature data?  aiProcess_PopulateArmatureData is the flag we would pass in to the importer.
		for (int i = 0; i < mesh->mNumBones; i++)
		{
			const aiBone* bone = mesh->mBones[i];

			for (int j = 0; j < bone->mNumWeights; j++)
			{
				const aiVertexWeight* vertexWeight = &bone->mWeights[j];

				if (vertexWeight->mVertexId >= skinWeights.GetNumVertices())
				{
					error = "Vertex weight index out of range.";
					return false;
				}

				std::vector<Imzadi::SkinWeights::BoneWeight>& boneWeightArray = skinWeights.GetBonesWeightsForVertex(vertexWeight->mVertexId);
				Imzadi::SkinWeights::BoneWeight boneWeight;
				boneWeight.boneName = bone->mName.C_Str();
				boneWeight.weight = vertexWeight->mWeight;
				boneWeightArray.push_back(boneWeight);
			}
		}

		std::string engineError;
		if (!skinWeights.Save(skinWeightsDoc, engineError))
		{
			error = engineError;
			return false;
		}

		if (!this->WriteJsonFile(skeletonDoc, skeletonFileName.GetFullPath(), error))
			return false;

		if (!this->WriteJsonFile(skinWeightsDoc, skinWeightsFileName.GetFullPath(), error))
			return false;

		// TODO: What about animations for the mesh?
	}

	if (!this->WriteJsonFile(meshDoc, meshFileName.GetFullPath(), error))
		return false;

	if (!this->WriteJsonFile(textureDoc, textureFileName.GetFullPath(), error))
		return false;

	if (!this->WriteJsonFile(verticesDoc, vertexBufferFileName.GetFullPath(), error))
		return false;

	if (!this->WriteJsonFile(indicesDoc, indexBufferFileName.GetFullPath(), error))
		return false;

	return true;
}

Imzadi::Vector3 Converter::ConvertVector(const aiVector3D& vector)
{
	return Imzadi::Vector3(
		vector.x,
		vector.y,
		vector.z
	);
}

Imzadi::Vector2 Converter::ConvertTexCoords(const aiVector3D& texCoords)
{
	return Imzadi::Vector2(
		texCoords.x,
		texCoords.y
	);
}

bool Converter::WriteJsonFile(const rapidjson::Document& jsonDoc, const wxString& assetFile, wxString& error)
{
	std::ofstream fileStream;
	fileStream.open((const char*)assetFile.c_str(), std::ios::out);
	if (!fileStream.is_open())
	{
		error = "Failed to open (for writing) the file: " + assetFile;
		return false;
	}

	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(stringBuffer);
	if (!jsonDoc.Accept(prettyWriter))
	{
		error = "Failed to generate JSON text from JSON data foe file: " + assetFile;
		return false;
	}

	fileStream << stringBuffer.GetString();
	fileStream.close();
	return true;
}

wxString Converter::MakeAssetFileReference(const wxString& assetFile)
{
	// TODO: Need to get this from somewhere rather than hard-code it.
	wxString assetBaseFolder = R"(E:\ENG_DEV\Imzadi\Games\SearchForTheSacredChaliceOfRixx\Assets)";

	wxFileName fileName(assetFile);
	fileName.MakeRelativeTo(assetBaseFolder);
	wxString relativeAssetPath = fileName.GetFullPath();
	return relativeAssetPath;
}