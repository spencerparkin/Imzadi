#include "Converter.h"
#include "Log.h"
#include <wx/filename.h>
#include <fstream>
#include "assimp/postprocess.h"
#include "assimp/config.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "AssetCache.h"
#include "Assets/SkinWeights.h"
#include "Assets/Skeleton.h"

Converter::Converter(const wxString& assetRootFolder)
{
	this->assetRootFolder = assetRootFolder;
}

/*virtual*/ Converter::~Converter()
{
}

bool Converter::Convert(const wxString& assetFile)
{
	LOG("Converting file: %s", (const char*)assetFile.c_str());

	wxFileName fileName(assetFile);
	this->assetFolder = fileName.GetPath();
	LOG("Assets will be dumped in folder: %s", (const char*)this->assetFolder.c_str());

	this->importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0);

	LOG("Calling Ass-Imp to load file: %s", (const char*)assetFile.c_str());
	const aiScene* scene = importer.ReadFile(assetFile.c_str(), aiProcess_GlobalScale);
	if (!scene)
	{
		LOG("Import error: %s", importer.GetErrorString());
		return false;
	}

	Imzadi::Transform nodeToWorld;
	nodeToWorld.SetIdentity();

	LOG("Walking scene graph...");
	return this->ProcessSceneGraph(scene, scene->mRootNode, nodeToWorld);
}

bool Converter::ProcessSceneGraph(const aiScene* scene, const aiNode* node, const Imzadi::Transform& parentNodeToWorld)
{
	LOG("Procesing node: %s", node->mName.C_Str());

	Imzadi::Transform childToParent;
	if (!this->MakeTransform(childToParent, node->mTransformation))
	{
		LOG("Failed to make child-to-parent transfrom from node matrix.");
		return false;
	}

	Imzadi::Transform nodeToWorld = parentNodeToWorld * childToParent;

	if (node->mNumMeshes > 0)
	{
		LOG("Found %d meshe(s).", node->mNumMeshes);

		for (int i = 0; i < node->mNumMeshes; i++)
		{
			LOG("Processing mesh %d of %d.", i + 1, node->mNumMeshes);
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			if (!this->ProcessMesh(scene, node, mesh, nodeToWorld))
			{
				LOG("Mesh processing failed!");
				return false;
			}
		}
	}
	
	for (int i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* childNode = node->mChildren[i];
		if (!this->ProcessSceneGraph(scene, childNode, nodeToWorld))
			return false;
	}

	return true;
}

bool Converter::ProcessMesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh, const Imzadi::Transform& nodeToWorld)
{
	LOG("Processing mesh: %s", mesh->mName.C_Str());

	wxFileName meshFileName;
	meshFileName.SetPath(this->assetFolder);
	meshFileName.SetName(mesh->mName.C_Str());
	meshFileName.SetExt(mesh->HasBones() ? "skinned_render_mesh" : "render_mesh");

	wxFileName textureFileName;
	textureFileName.SetPath(this->assetFolder);
	textureFileName.SetName(mesh->mName.C_Str());
	textureFileName.SetExt("texture");

	wxFileName vertexBufferFileName;
	vertexBufferFileName.SetPath(this->assetFolder);
	vertexBufferFileName.SetName(wxString(mesh->mName.C_Str()) + "_Vertices");
	vertexBufferFileName.SetExt("buffer");

	wxFileName indexBufferFileName;
	indexBufferFileName.SetPath(this->assetFolder);
	indexBufferFileName.SetName(wxString(mesh->mName.C_Str()) + "_Indices");
	indexBufferFileName.SetExt("buffer");

	rapidjson::Document meshDoc;
	meshDoc.SetObject();

	meshDoc.AddMember("primitive_type", rapidjson::Value().SetString("TRIANGLE_LIST"), meshDoc.GetAllocator());
	meshDoc.AddMember("shader", rapidjson::Value().SetString("Shaders/Standard.shader", meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("shadow_shader", rapidjson::Value().SetString("Shaders/StandardShadow.shader", meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("texture", rapidjson::Value().SetString(this->MakeAssetFileReference(textureFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("index_buffer", rapidjson::Value().SetString(this->MakeAssetFileReference(indexBufferFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("vertex_buffer", rapidjson::Value().SetString(this->MakeAssetFileReference(vertexBufferFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

	if (scene->mNumMaterials <= mesh->mMaterialIndex)
	{
		LOG("Error: Bad material index: %d", mesh->mMaterialIndex);
		return false;
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 1)
	{
		LOG("Error: Material does not have exactly one diffuse texture.");
		return false;
	}

	aiString texturePath;
	if (aiReturn_SUCCESS != material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
	{
		LOG("Failed to acquire texture path from material.");
		return false;
	}

	wxString textureFullPath = this->assetFolder + wxString::Format("/%s", texturePath.C_Str());
	LOG("Found texture: %s", (const char*)textureFullPath.c_str());

	rapidjson::Document textureDoc;
	textureDoc.SetObject();
	textureDoc.AddMember("flip_vertical", rapidjson::Value().SetBool(true), textureDoc.GetAllocator());
	textureDoc.AddMember("image_file", rapidjson::Value().SetString(this->MakeAssetFileReference(textureFullPath), textureDoc.GetAllocator()), textureDoc.GetAllocator());

	if (mesh->mNumVertices == 0)
	{
		LOG("Error: No vertices found.");
		return false;
	}

	if (mesh->mNumUVComponents[0] != 2)
	{
		LOG("Error: Expected exactly 2 UV components in first channel.");
		return false;
	}

	rapidjson::Document verticesDoc;
	verticesDoc.SetObject();

	rapidjson::Value vertexBufferValue;
	vertexBufferValue.SetArray();

	Imzadi::AxisAlignedBoundingBox* boundingBox = nullptr;

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		Imzadi::Vector3 position, normal;
		Imzadi::Vector2 texCoords;

		if (!this->MakeVector(position, mesh->mVertices[i]))
			return false;

		if (!this->MakeTexCoords(texCoords, mesh->mTextureCoords[0][i]))
			return false;

		if (!this->MakeVector(normal, mesh->mNormals[i]))
			return false;

		position = nodeToWorld.TransformPoint(position);
		normal = nodeToWorld.TransformNormal(normal);

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

		if (boundingBox)
			boundingBox->Expand(position);
		else
			boundingBox = new Imzadi::AxisAlignedBoundingBox(position);
	}

	verticesDoc.AddMember("bind", rapidjson::Value().SetString("vertex", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
	verticesDoc.AddMember("stride", rapidjson::Value().SetInt(8), verticesDoc.GetAllocator());
	verticesDoc.AddMember("type", rapidjson::Value().SetString("float", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
	verticesDoc.AddMember("buffer", vertexBufferValue, verticesDoc.GetAllocator());

	if (mesh->HasBones())
	{
		verticesDoc.AddMember("usage", rapidjson::Value().SetString("dynamic", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
		verticesDoc.AddMember("bare_buffer", rapidjson::Value().SetBool(true), verticesDoc.GetAllocator());
	}

	rapidjson::Value boundingBoxValue;
	Imzadi::Asset::SaveBoundingBox(boundingBoxValue, *boundingBox, &meshDoc);
	meshDoc.AddMember("bounding_box", boundingBoxValue, meshDoc.GetAllocator());
	delete boundingBox;

	if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
	{
		LOG("Error: Only triangle primitive currently supported.");
		return false;
	}

	if (mesh->mNumFaces == 0)
	{
		LOG("Error: No faces found.");
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
			LOG("Error: Expected exactly 3 indices in face.");
			return false;
		}

		for (int j = 0; j < face->mNumIndices; j++)
		{
			unsigned int index = face->mIndices[j];
			indexBufferValue.PushBack(rapidjson::Value().SetUint(index), indicesDoc.GetAllocator());

			if (index != static_cast<unsigned int>(static_cast<unsigned short>(index)))
			{
				LOG("Error: Index (%d) doesn't fit in an unsigned short.", index);
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
		skeletonFileName.SetPath(this->assetFolder);
		skeletonFileName.SetName(mesh->mName.C_Str());
		skeletonFileName.SetExt("skeleton");

		wxFileName skinWeightsFileName;
		skinWeightsFileName.SetPath(this->assetFolder);
		skinWeightsFileName.SetName(mesh->mName.C_Str());
		skinWeightsFileName.SetExt("skin_weights");

		meshDoc.AddMember("position_offset", rapidjson::Value().SetInt(0), meshDoc.GetAllocator());
		meshDoc.AddMember("normal_offset", rapidjson::Value().SetInt(20), meshDoc.GetAllocator());
		meshDoc.AddMember("skeleton", rapidjson::Value().SetString(this->MakeAssetFileReference(skeletonFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
		meshDoc.AddMember("skin_weights", rapidjson::Value().SetString(this->MakeAssetFileReference(skinWeightsFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

		rapidjson::Document skeletonDoc;
		skeletonDoc.SetObject();

		Imzadi::Skeleton skeleton;

		// TODO: Build the skeleton here.  Identify which nodes are bones using the prescribed method in the docs.

		std::string error;
		/*if (!skeleton.Save(skeletonDoc, error))
		{
			LOG("Error: %s", error.c_str());
			return false;
		}*/

		rapidjson::Document skinWeightsDoc;
		skinWeightsDoc.SetObject();

		Imzadi::SkinWeights skinWeights;
		skinWeights.SetNumVertices(mesh->mNumVertices);

		for (int i = 0; i < mesh->mNumBones; i++)
		{
			const aiBone* bone = mesh->mBones[i];

			for (int j = 0; j < bone->mNumWeights; j++)
			{
				const aiVertexWeight* vertexWeight = &bone->mWeights[j];

				if (vertexWeight->mVertexId >= skinWeights.GetNumVertices())
				{
					LOG("Error: Vertex weight index (%d) out of range (max: %d).", vertexWeight->mVertexId, skinWeights.GetNumVertices() - 1);
					return false;
				}

				std::vector<Imzadi::SkinWeights::BoneWeight>& boneWeightArray = skinWeights.GetBonesWeightsForVertex(vertexWeight->mVertexId);
				Imzadi::SkinWeights::BoneWeight boneWeight;
				boneWeight.boneName = bone->mName.C_Str();
				boneWeight.weight = vertexWeight->mWeight;
				boneWeightArray.push_back(boneWeight);
			}
		}

		if (!skinWeights.Save(skinWeightsDoc, error))
		{
			LOG("Error: %s", error.c_str());
			return false;
		}

		if (!this->WriteJsonFile(skeletonDoc, skeletonFileName.GetFullPath()))
			return false;

		if (!this->WriteJsonFile(skinWeightsDoc, skinWeightsFileName.GetFullPath()))
			return false;

		// TODO: What about animations for the mesh?
	}

	if (!this->WriteJsonFile(meshDoc, meshFileName.GetFullPath()))
		return false;

	if (!this->WriteJsonFile(textureDoc, textureFileName.GetFullPath()))
		return false;

	if (!this->WriteJsonFile(verticesDoc, vertexBufferFileName.GetFullPath()))
		return false;

	if (!this->WriteJsonFile(indicesDoc, indexBufferFileName.GetFullPath()))
		return false;

	return true;
}

bool Converter::MakeTransform(Imzadi::Transform& transformOut, const aiMatrix4x4& matrixIn)
{
	if (matrixIn.d1 != 0.0 || matrixIn.d2 != 0.0 || matrixIn.d3 != 0.0 || matrixIn.d4 != 1.0)
		return false;

	transformOut.matrix.ele[0][0] = matrixIn.a1;
	transformOut.matrix.ele[0][1] = matrixIn.a2;
	transformOut.matrix.ele[0][2] = matrixIn.a3;

	transformOut.matrix.ele[1][0] = matrixIn.b1;
	transformOut.matrix.ele[1][1] = matrixIn.b2;
	transformOut.matrix.ele[1][2] = matrixIn.b3;

	transformOut.matrix.ele[2][0] = matrixIn.c1;
	transformOut.matrix.ele[2][1] = matrixIn.c2;
	transformOut.matrix.ele[2][2] = matrixIn.c3;

	transformOut.translation.x = matrixIn.a4;
	transformOut.translation.y = matrixIn.b4;
	transformOut.translation.z = matrixIn.c4;

	return true;
}

bool Converter::MakeVector(Imzadi::Vector3& vectorOut, const aiVector3D& vectorIn)
{
	vectorOut.SetComponents(vectorIn.x, vectorIn.y, vectorIn.z);
	return true;
}

bool Converter::MakeTexCoords(Imzadi::Vector2& texCoordsOut, const aiVector3D& texCoordsIn)
{
	texCoordsOut.SetComponents(texCoordsIn.x, texCoordsIn.y);
	return true;
}

bool Converter::WriteJsonFile(const rapidjson::Document& jsonDoc, const wxString& assetFile)
{
	std::ofstream fileStream;
	fileStream.open((const char*)assetFile.c_str(), std::ios::out);
	if (!fileStream.is_open())
	{
		LOG("Failed to open (for writing) the file: %s", (const char*)assetFile.c_str());
		return false;
	}

	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(stringBuffer);
	if (!jsonDoc.Accept(prettyWriter))
	{
		LOG("Failed to generate JSON text from JSON data foe file: %s", (const char*)assetFile.c_str());
		return false;
	}

	fileStream << stringBuffer.GetString();
	fileStream.close();
	LOG("Wrote file: %s", (const char*)assetFile.c_str());
	return true;
}

wxString Converter::MakeAssetFileReference(const wxString& assetFile)
{
	wxFileName fileName(assetFile);
	fileName.MakeRelativeTo(this->assetRootFolder);
	wxString relativeAssetPath = fileName.GetFullPath();
	return relativeAssetPath;
}