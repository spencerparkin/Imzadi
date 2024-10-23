#include "Converter.h"
#include "Log.h"
#include <wx/filename.h>
#include <fstream>
#include "assimp/postprocess.h"
#include "assimp/config.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Polygon.h"
#include "AssetCache.h"
#include "App.h"
#include "Frame.h"
#include "TextureMaker.h"
#include <wx/textdlg.h>

Converter::Converter()
{
	this->flags = 0;
	this->textureMakerFlags = 0;
}

/*virtual*/ Converter::~Converter()
{
}

bool Converter::Convert(const wxString& assetFile)
{
	IMZADI_LOG_INFO("Converting file: %s", (const char*)assetFile.c_str());

	wxFileName fileName(assetFile);
	this->assetFolder = fileName.GetPath();
	IMZADI_LOG_INFO("Assets will be dumped in folder: %s", (const char*)this->assetFolder.c_str());

	this->importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0);

	IMZADI_LOG_INFO("Calling Ass-Imp to load file: %s", (const char*)assetFile.c_str());
	const aiScene* scene = importer.ReadFile(assetFile.c_str(), aiProcess_GlobalScale | aiProcess_PopulateArmatureData);
	if (!scene)
	{
		IMZADI_LOG_ERROR("Import error: %s", importer.GetErrorString());
		return false;
	}

	IMZADI_LOG_INFO("Generating node-to-world transformation map...");
	this->nodeToWorldMap.clear();
	if (!this->GenerateNodeToWorldMap(scene->mRootNode))
	{
		IMZADI_LOG_ERROR("Failed to generate transformation map.");
		return false;
	}

	this->navGraph.Clear();

	if ((this->flags & Flag::CONVERT_MESHES) != 0)
	{
		IMZADI_LOG_INFO("Processing scene graph...");
		if (!this->ProcessSceneGraph(scene, scene->mRootNode))
		{
			IMZADI_LOG_ERROR("Failed to process scene graph.");
			return false;
		}
	}

	if ((this->flags & Flag::CONVERT_ANIMATIONS) != 0)
	{
		IMZADI_LOG_INFO("Found %d animations.", scene->mNumAnimations);
		for (int i = 0; i < scene->mNumAnimations; i++)
		{
			IMZADI_LOG_INFO("Processing animation %d of %d.", i + 1, scene->mNumAnimations);
			const aiAnimation* animation = scene->mAnimations[i];
			if (!this->ProcessAnimation(scene, animation))
			{
				IMZADI_LOG_ERROR("Failed to process animation.");
				return false;
			}
		}
	}

	if ((this->flags & Flag::CONVERT_SKYDOME) != 0)
	{
		const aiNode* skyDomeNode = this->FindNodeByName(scene, "SkyDome");
		if (!skyDomeNode)
		{
			IMZADI_LOG_ERROR("Failed to find sky-dome node.");
			return false;
		}

		if (!this->GenerateSkyDome(assetFile, scene, skyDomeNode))
		{
			IMZADI_LOG_ERROR("Failed to generate sky-dome.");
			return false;
		}
	}

	if ((this->flags & Flag::MAKE_NAV_GRAPH) != 0)
	{
		wxFileName navGraphFile;
		navGraphFile.SetPath(fileName.GetPath());
		navGraphFile.SetName(fileName.GetName());
		navGraphFile.SetExt("nav_graph");

		rapidjson::Document navGraphDoc;
		if (!this->navGraph.Save(navGraphDoc))
		{
			IMZADI_LOG_ERROR("Failed to save nav-graph to JSON.");
			return false;
		}

		if (!JsonUtils::WriteJsonFile(navGraphDoc, navGraphFile.GetFullPath()))
		{
			IMZADI_LOG_ERROR("Failed to write nav-graph file: %s", (const char*)navGraphFile.GetFullPath().c_str());
			return false;
		}
	}

	return true;
}

const aiNode* Converter::FindNodeByName(const aiScene* scene, const char* name)
{
	std::list<const aiNode*> nodeQueue;
	nodeQueue.push_back(scene->mRootNode);
	while (nodeQueue.size() > 0)
	{
		std::list<const aiNode*>::iterator iter = nodeQueue.begin();
		const aiNode* node = *iter;
		nodeQueue.erase(iter);

		if (strcmp(node->mName.C_Str(), name) == 0)
			return node;

		for (int i = 0; i < node->mNumChildren; i++)
			nodeQueue.push_back(node->mChildren[i]);
	}

	return nullptr;
}

bool Converter::GenerateSkyDome(const wxString& assetFile, const aiScene* scene, const aiNode* node)
{
	wxFileName fileName(assetFile);

	wxFileName skyDomeFile;
	skyDomeFile.SetPath(fileName.GetPath());
	skyDomeFile.SetName(fileName.GetName());
	skyDomeFile.SetExt("sky_dome");

	wxFileName cubeTextureFile;
	cubeTextureFile.SetPath(fileName.GetPath());
	cubeTextureFile.SetName(fileName.GetName());
	cubeTextureFile.SetExt("cube_texture");

	const aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];

	wxFileName vertexBufferFileName;
	vertexBufferFileName.SetPath(this->assetFolder);
	vertexBufferFileName.SetName(wxString(mesh->mName.C_Str()) + "_Vertices");
	vertexBufferFileName.SetExt("buffer");

	wxFileName indexBufferFileName;
	indexBufferFileName.SetPath(this->assetFolder);
	indexBufferFileName.SetName(wxString(mesh->mName.C_Str()) + "_Indices");
	indexBufferFileName.SetExt("buffer");

	if (node->mNumMeshes != 1)
	{
		IMZADI_LOG_ERROR("Didn't find a sky-dome mesh.");
		return false;
	}

	rapidjson::Document skyDomeDoc;
	skyDomeDoc.SetObject();
	skyDomeDoc.AddMember("primitive_type", rapidjson::Value().SetString("TRIANGLE_LIST"), skyDomeDoc.GetAllocator());
	skyDomeDoc.AddMember("shader", rapidjson::Value().SetString("Shaders/SkyDome.shader", skyDomeDoc.GetAllocator()), skyDomeDoc.GetAllocator());
	skyDomeDoc.AddMember("cube_texture", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(cubeTextureFile.GetFullPath()), skyDomeDoc.GetAllocator()), skyDomeDoc.GetAllocator());
	skyDomeDoc.AddMember("index_buffer", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(indexBufferFileName.GetFullPath()), skyDomeDoc.GetAllocator()), skyDomeDoc.GetAllocator());
	skyDomeDoc.AddMember("vertex_buffer", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(vertexBufferFileName.GetFullPath()), skyDomeDoc.GetAllocator()), skyDomeDoc.GetAllocator());

	Imzadi::Transform nodeToWorld;
	if (!this->GetNodeToWorldTransform(node, nodeToWorld))
	{
		IMZADI_LOG_ERROR("Failed to get node-to-world transform for the sky-dome node.");
		return false;
	}

	rapidjson::Document indicesDoc;
	if (!this->GenerateIndexBuffer(indicesDoc, mesh))
		return false;

	rapidjson::Document verticesDoc;
	Imzadi::AxisAlignedBoundingBox boundingBox;
	if (!this->GenerateVertexBuffer(verticesDoc, mesh, nodeToWorld, VBufFlag::POSITION, boundingBox))
		return false;

	rapidjson::Document cubeTextureDoc;		// For now, this is just meant to be hand-edited after we kick it out.
	cubeTextureDoc.SetObject();

	rapidjson::Value textureArrayValue;
	textureArrayValue.SetArray();
	textureArrayValue.PushBack(rapidjson::Value().SetString("Textures/SkyDome_PX.texture", cubeTextureDoc.GetAllocator()), cubeTextureDoc.GetAllocator());
	textureArrayValue.PushBack(rapidjson::Value().SetString("Textures/SkyDome_NX.texture", cubeTextureDoc.GetAllocator()), cubeTextureDoc.GetAllocator());
	textureArrayValue.PushBack(rapidjson::Value().SetString("Textures/SkyDome_PY.texture", cubeTextureDoc.GetAllocator()), cubeTextureDoc.GetAllocator());
	textureArrayValue.PushBack(rapidjson::Value().SetString("Textures/SkyDome_NY.texture", cubeTextureDoc.GetAllocator()), cubeTextureDoc.GetAllocator());
	textureArrayValue.PushBack(rapidjson::Value().SetString("Textures/SkyDome_PZ.texture", cubeTextureDoc.GetAllocator()), cubeTextureDoc.GetAllocator());
	textureArrayValue.PushBack(rapidjson::Value().SetString("Textures/SkyDome_NZ.texture", cubeTextureDoc.GetAllocator()), cubeTextureDoc.GetAllocator());

	cubeTextureDoc.AddMember("texture_array", textureArrayValue, cubeTextureDoc.GetAllocator());

	if (!JsonUtils::WriteJsonFile(skyDomeDoc, skyDomeFile.GetFullPath()))
		return false;

	if (!JsonUtils::WriteJsonFile(cubeTextureDoc, cubeTextureFile.GetFullPath()))
		return false;

	if (!JsonUtils::WriteJsonFile(indicesDoc, indexBufferFileName.GetFullPath()))
		return false;

	if (!JsonUtils::WriteJsonFile(verticesDoc, vertexBufferFileName.GetFullPath()))
		return false;

	return true;
}

bool Converter::ProcessAnimation(const aiScene* scene, const aiAnimation* animation)
{
	wxString animName(animation->mName.C_Str());
	animName.Replace(" ", "_");

	wxTextEntryDialog nameDialog(wxGetApp().GetFrame(), wxString::Format("The current animation will be saved as \"%s\".  Rename it?", animation->mName.C_Str()), "Rename?", animName);
	if (nameDialog.ShowModal() == wxID_OK)
		animName = nameDialog.GetValue();

	wxFileName animFile;
	animFile.SetPath(wxGetApp().GetAssetsRootFolder() + "/Animations");
	animFile.SetName(animName);
	animFile.SetExt("animation");

	Imzadi::Animation generatedAnimation;
	generatedAnimation.SetName((const char*)animName.c_str());
	double currentTick = -std::numeric_limits<double>::max();
	Imzadi::KeyFrame* keyFrame = nullptr;
	while (this->FindNextKeyFrame(animation, currentTick, keyFrame))
		generatedAnimation.AddKeyFrame(keyFrame);

	rapidjson::Document animDoc;
	generatedAnimation.Save(animDoc);
	if (!JsonUtils::WriteJsonFile(animDoc, animFile.GetFullPath()))
		return false;

	return true;
}

bool Converter::FindNextKeyFrame(const aiAnimation* animation, double& currentTick, Imzadi::KeyFrame*& keyFrame)
{
	double soonestTick = std::numeric_limits<double>::max();

	for (int i = 0; i < animation->mNumChannels; i++)
	{
		const aiNodeAnim* nodeAnim = animation->mChannels[i];
		
		for (int j = 0; j < nodeAnim->mNumPositionKeys; j++)
		{
			const aiVectorKey* positionKey = &nodeAnim->mPositionKeys[j];
			if (positionKey->mTime > currentTick && positionKey->mTime < soonestTick)
				soonestTick = positionKey->mTime;
		}

		for (int j = 0; j < nodeAnim->mNumRotationKeys; j++)
		{
			const aiQuatKey* rotationKey = &nodeAnim->mRotationKeys[j];
			if (rotationKey->mTime > currentTick && rotationKey->mTime < soonestTick)
				soonestTick = rotationKey->mTime;
		}

		for (int j = 0; j < nodeAnim->mNumScalingKeys; j++)
		{
			const aiVectorKey* scalingKey = &nodeAnim->mScalingKeys[j];
			if (scalingKey->mTime > currentTick && scalingKey->mTime < soonestTick)
				soonestTick = scalingKey->mTime;
		}
	}

	if (soonestTick == std::numeric_limits<double>::max())
		return false;

	currentTick = soonestTick;

	keyFrame = new Imzadi::KeyFrame();
	keyFrame->SetTime(soonestTick / animation->mTicksPerSecond);

	// We pose all the nodes in a single key-frame.
	for (int i = 0; i < animation->mNumChannels; i++)
	{
		const aiNodeAnim* nodeAnim = animation->mChannels[i];

		Imzadi::KeyFrame::PoseInfo poseInfo;
		poseInfo.boneName = nodeAnim->mNodeName.C_Str();

		int findCount = 0;

		for (int j = 0; j < nodeAnim->mNumPositionKeys - 1; j++)
		{
			const aiVectorKey* positionKeyA = &nodeAnim->mPositionKeys[j];
			const aiVectorKey* positionKeyB = &nodeAnim->mPositionKeys[j + 1];

			if (positionKeyA->mTime <= currentTick && currentTick <= positionKeyB->mTime)
			{
				Imzadi::Vector3 translationA, translationB;
				this->MakeVector(translationA, positionKeyA->mValue);
				this->MakeVector(translationB, positionKeyB->mValue);
				double alpha = (currentTick - positionKeyA->mTime) / (positionKeyB->mTime - positionKeyA->mTime);
				poseInfo.childToParent.translation.Lerp(translationA, translationB, alpha);
				findCount++;
				break;
			}
		}

		for (int j = 0; j < nodeAnim->mNumRotationKeys - 1; j++)
		{
			const aiQuatKey* rotationKeyA = &nodeAnim->mRotationKeys[j];
			const aiQuatKey* rotationKeyB = &nodeAnim->mRotationKeys[j + 1];

			if (rotationKeyA->mTime <= currentTick && currentTick <= rotationKeyB->mTime)
			{
				Imzadi::Quaternion rotationA, rotationB;
				this->MakeQuat(rotationA, rotationKeyA->mValue);
				this->MakeQuat(rotationB, rotationKeyB->mValue);
				double alpha = (currentTick - rotationKeyA->mTime) / (rotationKeyB->mTime - rotationKeyA->mTime);
				poseInfo.childToParent.rotation.Interpolate(rotationA, rotationB, alpha);
				findCount++;
				break;
			}
		}

		for (int j = 0; j < nodeAnim->mNumScalingKeys - 1; j++)
		{
			const aiVectorKey* scalingKeyA = &nodeAnim->mScalingKeys[j];
			const aiVectorKey* scalingKeyB = &nodeAnim->mScalingKeys[j + 1];

			if (scalingKeyA->mTime <= currentTick && currentTick <= scalingKeyB->mTime)
			{
				Imzadi::Vector3 scaleA, scaleB;
				this->MakeVector(scaleA, scalingKeyA->mValue);
				this->MakeVector(scaleB, scalingKeyB->mValue);
				double alpha = (currentTick - scalingKeyA->mTime) / (scalingKeyB->mTime - scalingKeyA->mTime);
				poseInfo.childToParent.scale.Lerp(scaleA, scaleB, alpha);
				findCount++;
				break;
			}
		}

		if (!poseInfo.childToParent.IsValid())
			IMZADI_LOG_WARNING("Warning: Encountered invalid child-to-parent transform!");

		if (findCount == 3)
			keyFrame->AddPoseInfo(poseInfo);
		else
			IMZADI_LOG_ERROR("Failed to find scale, position and translation at tick %f!", currentTick);
	}
	
	return true;
}

bool Converter::GenerateNodeToWorldMap(const aiNode* node)
{
	Imzadi::Transform nodeToWorld;

	if (!node->mParent)
		nodeToWorld.SetIdentity();
	else
	{
		Imzadi::Transform parentNodeToWorld;
		if (!this->GetNodeToWorldTransform(node->mParent, parentNodeToWorld))
		{
			IMZADI_LOG_ERROR("Failed to find parent node's transform in the map.");
			return false;
		}

		Imzadi::Transform childToParent;
		if (!this->MakeTransform(childToParent, node->mTransformation))
		{
			IMZADI_LOG_ERROR("Failed to make child-to-parent transform from node matrix.");
			return false;
		}

		nodeToWorld = parentNodeToWorld * childToParent;
	}

	this->nodeToWorldMap.insert(std::pair<const aiNode*, Imzadi::Transform>(node, nodeToWorld));

	for (int i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* childNode = node->mChildren[i];
		this->GenerateNodeToWorldMap(childNode);
	}

	return true;
}

bool Converter::GetNodeToWorldTransform(const aiNode* node, Imzadi::Transform& nodeToWorld)
{
	std::unordered_map<const aiNode*, Imzadi::Transform>::iterator iter = this->nodeToWorldMap.find(node);
	if (iter == this->nodeToWorldMap.end())
		return false;

	nodeToWorld = iter->second;
	return true;
}

bool Converter::ProcessSceneGraph(const aiScene* scene, const aiNode* node)
{
	IMZADI_LOG_INFO("Processing node: %s", node->mName.C_Str());

	if (node->mNumMeshes > 0)
	{
		IMZADI_LOG_INFO("Found %d meshe(s).", node->mNumMeshes);

		for (int i = 0; i < node->mNumMeshes; i++)
		{
			IMZADI_LOG_INFO("Processing mesh %d of %d.", i + 1, node->mNumMeshes);
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			std::string meshName(mesh->mName.C_Str());

			// Ignore port meshes, because these are processed when a mesh is processed.
			if (meshName.find("Port") == 0)
				continue;

			if (meshName.find("TriggerBox") == 0)
			{
				if (!this->ProcessTriggerBox(scene, node, mesh))
				{
					IMZADI_LOG_ERROR("Trigger-box processing failed!");
					return false;
				}
			}
			else
			{
				if (!this->ProcessMesh(scene, node, mesh))
				{
					IMZADI_LOG_ERROR("Mesh processing failed!");
					return false;
				}
			}
		}
	}
	
	for (int i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* childNode = node->mChildren[i];
		if (!this->ProcessSceneGraph(scene, childNode))
			return false;
	}

	return true;
}

bool Converter::ProcessTriggerBox(const aiScene* scene, const aiNode* node, const aiMesh* mesh)
{
	Imzadi::Transform nodeToWorld;
	if (!this->GetNodeToWorldTransform(node, nodeToWorld))
	{
		IMZADI_LOG_ERROR("Error: Failed to get node-to-world transform for mesh node.");
		return false;
	}

	Imzadi::AxisAlignedBoundingBox boundingBox;
	
	boundingBox.MakeReadyForExpansion();

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		Imzadi::Vector3 position;
		this->MakeVector(position, mesh->mVertices[i]);
		position = nodeToWorld.TransformPoint(position);
		boundingBox.Expand(position);
	}

	std::string meshName(mesh->mName.C_Str());
	size_t i = meshName.find('_');
	if (i == std::string::npos)
	{
		IMZADI_LOG_ERROR("Mesh name \"%s\" has no underscore in it.  Everything right of the underscore is used to name the trigger-box.  Left of the underscore should be \"TriggerBox\".", meshName.c_str());
		return false;
	}

	std::string triggerBoxName = meshName.substr(i + 1, std::string::npos);

	wxFileName triggerBoxFileName;
	triggerBoxFileName.SetPath(this->assetFolder);
	triggerBoxFileName.SetName(triggerBoxName);
	triggerBoxFileName.SetExt("trigger_box");

	rapidjson::Document triggerBoxDoc;
	triggerBoxDoc.SetObject();

	rapidjson::Value triggerBoxValue;
	Imzadi::Asset::SaveBoundingBox(triggerBoxValue, boundingBox, &triggerBoxDoc);

	triggerBoxDoc.AddMember("box", triggerBoxValue, triggerBoxDoc.GetAllocator());
	triggerBoxDoc.AddMember("name", rapidjson::Value().SetString(triggerBoxName.c_str(), triggerBoxDoc.GetAllocator()), triggerBoxDoc.GetAllocator());

	if (!JsonUtils::WriteJsonFile(triggerBoxDoc, triggerBoxFileName.GetFullPath()))
		return false;

	return true;
}

bool Converter::GenerateIndexBuffer(rapidjson::Document& indicesDoc, const aiMesh* mesh)
{
	indicesDoc.SetObject();

	rapidjson::Value indexBufferValue;
	indexBufferValue.SetArray();

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace* face = &mesh->mFaces[i];
		if (face->mNumIndices != 3)
		{
			IMZADI_LOG_ERROR("Error: Expected exactly 3 indices in face.");
			return false;
		}

		for (int j = 0; j < face->mNumIndices; j++)
		{
			unsigned int index = face->mIndices[j];
			indexBufferValue.PushBack(rapidjson::Value().SetUint(index), indicesDoc.GetAllocator());

			if (index != static_cast<unsigned int>(static_cast<unsigned short>(index)))
			{
				IMZADI_LOG_ERROR("Error: Index (%d) doesn't fit in an unsigned short.", index);
				return false;
			}
		}
	}

	indicesDoc.AddMember("bind", rapidjson::Value().SetString("index", indicesDoc.GetAllocator()), indicesDoc.GetAllocator());
	indicesDoc.AddMember("stride", rapidjson::Value().SetInt(1), indicesDoc.GetAllocator());
	indicesDoc.AddMember("type", rapidjson::Value().SetString("ushort", indicesDoc.GetAllocator()), indicesDoc.GetAllocator());
	indicesDoc.AddMember("buffer", indexBufferValue, indicesDoc.GetAllocator());

	return true;
}

bool Converter::GenerateVertexBuffer(rapidjson::Document& verticesDoc, const aiMesh* mesh, const Imzadi::Transform& nodeToObject, uint32_t flags, Imzadi::AxisAlignedBoundingBox& boundingBox)
{
	if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
	{
		IMZADI_LOG_ERROR("Only triangle primitive currently supported.");
		return false;
	}

	if (mesh->mNumFaces == 0)
	{
		IMZADI_LOG_ERROR("No faces found.");
		return false;
	}

	if (mesh->mNumVertices == 0)
	{
		IMZADI_LOG_ERROR("No vertices in mesh!");
		return false;
	}

	if ((flags & VBufFlag::TEXCOORD) != 0 && mesh->mNumUVComponents[0] != 2)
	{
		IMZADI_LOG_ERROR("Expected exactly 2 UV components in first channel.");
		return false;
	}

	verticesDoc.SetObject();

	rapidjson::Value vertexBufferValue;
	vertexBufferValue.SetArray();

	boundingBox.MakeReadyForExpansion();

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		if ((flags & VBufFlag::POSITION) != 0)
		{
			Imzadi::Vector3 position;
			if (!this->MakeVector(position, mesh->mVertices[i]))
				return false;

			position = nodeToObject.TransformPoint(position);

			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.x), verticesDoc.GetAllocator());
			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.y), verticesDoc.GetAllocator());
			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(position.z), verticesDoc.GetAllocator());

			boundingBox.Expand(position);
		}

		if ((flags & VBufFlag::TEXCOORD) != 0)
		{
			Imzadi::Vector2 texCoords;
			if (!this->MakeTexCoords(texCoords, mesh->mTextureCoords[0][i]))
				return false;

			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(texCoords.x), verticesDoc.GetAllocator());
			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(texCoords.y), verticesDoc.GetAllocator());
		}

		if ((flags & VBufFlag::NORMAL) != 0)
		{
			Imzadi::Vector3 normal;
			if (!this->MakeVector(normal, mesh->mNormals[i]))
				return false;

			normal = nodeToObject.TransformVector(normal);
			if (!normal.Normalize())
				return false;

			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.x), verticesDoc.GetAllocator());
			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.y), verticesDoc.GetAllocator());
			vertexBufferValue.PushBack(rapidjson::Value().SetFloat(normal.z), verticesDoc.GetAllocator());
		}
	}

	UINT stride = 0;
	if ((flags & VBufFlag::POSITION))
		stride += 3;
	if ((flags & VBufFlag::TEXCOORD))
		stride += 2;
	if ((flags & VBufFlag::NORMAL))
		stride += 3;

	verticesDoc.AddMember("bind", rapidjson::Value().SetString("vertex", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
	verticesDoc.AddMember("stride", rapidjson::Value().SetInt(stride), verticesDoc.GetAllocator());
	verticesDoc.AddMember("type", rapidjson::Value().SetString("float", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
	verticesDoc.AddMember("buffer", vertexBufferValue, verticesDoc.GetAllocator());

	return true;
}

bool Converter::ProcessMesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh)
{
	// TODO: Add LOD support.  The engine already has some work done on this front.

	IMZADI_LOG_INFO("Processing mesh: %s", mesh->mName.C_Str());

	wxFileName meshFileName;
	meshFileName.SetPath(this->assetFolder);
	meshFileName.SetName(mesh->mName.C_Str());
	meshFileName.SetExt(mesh->HasBones() ? "skinned_render_mesh" : "render_mesh");

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
	meshDoc.AddMember("index_buffer", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(indexBufferFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
	meshDoc.AddMember("vertex_buffer", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(vertexBufferFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

	// Don't let shadows cast onto the characters.  It just looks too awful.  The game looks bad, but we don't have to make it look even worse.
	if (mesh->HasBones())
		meshDoc.AddMember("shadow_scale", rapidjson::Value().SetFloat(1.0f), meshDoc.GetAllocator());

	wxString meshName = meshFileName.GetName();
	meshDoc.AddMember("name", rapidjson::Value().SetString((const char*)meshName.c_str(), meshDoc.GetAllocator()), meshDoc.GetAllocator());

	if (scene->mNumMaterials <= mesh->mMaterialIndex)
	{
		IMZADI_LOG_ERROR("Error: Bad material index: %d", mesh->mMaterialIndex);
		return false;
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 1)
	{
		IMZADI_LOG_ERROR("Error: Material does not have exactly one diffuse texture.  It has %d of them.", material->GetTextureCount(aiTextureType_DIFFUSE));
		return false;
	}

	aiString texturePath;
	if (aiReturn_SUCCESS != material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
	{
		IMZADI_LOG_ERROR("Failed to acquire texture path from material.");
		return false;
	}

	wxFileName textureFileName(texturePath.C_Str());
	wxString textureFullPath;
	if (!textureFileName.IsAbsolute())
		textureFileName.Normalize(wxPATH_NORM_ALL, this->assetFolder);
	textureFullPath = textureFileName.GetFullPath();
	IMZADI_LOG_INFO("Found texture: %s", (const char*)textureFullPath.c_str());
	if (!this->textureMaker.MakeTexture(textureFullPath, this->textureMakerFlags))
	{
		IMZADI_LOG_ERROR("Failed to make texture!");
		return false;
	}

	meshDoc.AddMember("texture", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(this->textureMaker.GetTextureFilePath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

	Imzadi::Transform nodeToWorld;
	if (!this->GetNodeToWorldTransform(node, nodeToWorld))
	{
		IMZADI_LOG_ERROR("Error: Failed to get node-to-world transform for mesh node.");
		return false;
	}

	Imzadi::Transform worldToObject;
	worldToObject.SetIdentity();

	// This flag is given in most cases, but certainly not in the case of a character mesh.
	// This is because we want the character mesh's object-space-origin to be at their feet.
	if ((this->flags & Flag::CENTER_OBJ_SPACE_AT_ORIGIN) != 0)
	{
		if (mesh->mNumVertices == 0)
		{
			IMZADI_LOG_ERROR("Encountered mesh with zero vertices.");
			return false;
		}

		Imzadi::Vector3 center(0.0, 0.0, 0.0);
		for (int i = 0; i < mesh->mNumVertices; i++)
		{
			Imzadi::Vector3 position;
			this->MakeVector(position, mesh->mVertices[i]);
			center += nodeToWorld.TransformPoint(position);
		}

		center /= double(mesh->mNumVertices);
		worldToObject.translation = -center;
	}

	Imzadi::Transform objectToWorld;
	objectToWorld.Invert(worldToObject);
	rapidjson::Value meshObjectToWorldValue;
	Imzadi::Asset::SaveTransform(meshObjectToWorldValue, objectToWorld, &meshDoc);
	meshDoc.AddMember("object_to_world", meshObjectToWorldValue, meshDoc.GetAllocator());

	Imzadi::Transform nodeToObject = worldToObject * nodeToWorld;

	rapidjson::Document verticesDoc;
	Imzadi::AxisAlignedBoundingBox boundingBox;
	if (!this->GenerateVertexBuffer(verticesDoc, mesh, nodeToObject, VBufFlag::POSITION | VBufFlag::NORMAL | VBufFlag::TEXCOORD, boundingBox))
		return false;

	rapidjson::Value boundingBoxValue;
	Imzadi::Asset::SaveBoundingBox(boundingBoxValue, boundingBox, &meshDoc);
	meshDoc.AddMember("bounding_box", boundingBoxValue, meshDoc.GetAllocator());

	if (mesh->HasBones())
	{
		verticesDoc.AddMember("usage", rapidjson::Value().SetString("dynamic", verticesDoc.GetAllocator()), verticesDoc.GetAllocator());
		verticesDoc.AddMember("bare_buffer", rapidjson::Value().SetBool(true), verticesDoc.GetAllocator());
	}

	rapidjson::Document indicesDoc;
	if (!this->GenerateIndexBuffer(indicesDoc, mesh))
		return false;

	if ((this->flags & Flag::MAKE_COLLISION) != 0)
	{
		wxFileName collisionFileName;
		collisionFileName.SetPath(this->assetFolder);
		collisionFileName.SetName(mesh->mName.C_Str());
		collisionFileName.SetExt("collision");

		rapidjson::Document collisionDoc;
		collisionDoc.SetObject();

		rapidjson::Value collisionSetObjectToWorldValue;
		Imzadi::Asset::SaveTransform(collisionSetObjectToWorldValue, objectToWorld, &collisionDoc);
		collisionDoc.AddMember("object_to_world", collisionSetObjectToWorldValue, collisionDoc.GetAllocator());

		rapidjson::Value shapeSetValue;
		shapeSetValue.SetArray();

		std::vector<Imzadi::Polygon> polygonArray;
		for (int i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace* face = &mesh->mFaces[i];

			Imzadi::Polygon polygon;
			for (int j = 0; j < face->mNumIndices; j++)
			{
				Imzadi::Vector3 vertex;

				unsigned int k = face->mIndices[j];
				if (k >= mesh->mNumVertices)
				{
					IMZADI_LOG_ERROR("Index out of range!");
					return false;
				}

				this->MakeVector(vertex, mesh->mVertices[k]);
				vertex = nodeToObject.TransformPoint(vertex);
				polygon.vertexArray.push_back(vertex);
			}

			polygonArray.push_back(polygon);
		}

		if ((this->flags & Flag::COMPRESS_COLLISION) != 0)
		{
#if defined _DEBUG
			bool sanityCheck = true;
#else
			bool sanityCheck = false;
#endif
			Imzadi::Polygon::Compress(polygonArray, true, sanityCheck);
		}

		for(const Imzadi::Polygon& polygon : polygonArray)
		{
			rapidjson::Value collisionPolygonValue;
			collisionPolygonValue.SetObject();
			collisionPolygonValue.AddMember("type", rapidjson::Value().SetString("polygon", collisionDoc.GetAllocator()), collisionDoc.GetAllocator());

			rapidjson::Value vertexArrayValue;
			vertexArrayValue.SetArray();

			for (const Imzadi::Vector3& vertex : polygon.vertexArray)
			{
				rapidjson::Value vertexValue;
				Imzadi::Asset::SaveVector(vertexValue, vertex, &collisionDoc);
				vertexArrayValue.PushBack(vertexValue, collisionDoc.GetAllocator());
			}

			collisionPolygonValue.AddMember("vertex_array", vertexArrayValue, collisionDoc.GetAllocator());

			shapeSetValue.PushBack(collisionPolygonValue, collisionDoc.GetAllocator());
		}

		collisionDoc.AddMember("shape_set", shapeSetValue, collisionDoc.GetAllocator());

		if (!JsonUtils::WriteJsonFile(collisionDoc, collisionFileName.GetFullPath()))
			return false;

		if ((this->flags & Flag::MAKE_NAV_GRAPH) != 0)
		{
			for (const Imzadi::Polygon& polygon : polygonArray)
			{
				Imzadi::Polygon worldPolygon;
				objectToWorld.TransformPolygon(polygon, worldPolygon);
				this->navGraph.AddPathSegment(worldPolygon);
			}
		}
	}

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
		meshDoc.AddMember("skeleton", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(skeletonFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());
		meshDoc.AddMember("skin_weights", rapidjson::Value().SetString(wxGetApp().MakeAssetFileReference(skinWeightsFileName.GetFullPath()), meshDoc.GetAllocator()), meshDoc.GetAllocator());

		// Note that for this to work with 3Ds Max, the scene should have been exported
		// with the model in bind-pose.  Even if posed, I don't know why it doesn't already
		// do that, but whatever.  Similarly, exporting an animation won't work unless the
		// animation is applied at time of export.
		Imzadi::Skeleton skeleton;
		if (!this->GenerateSkeleton(skeleton, mesh))
		{
			IMZADI_LOG_ERROR("Failed to generate skeleton.");
			return false;
		}

		rapidjson::Document skeletonDoc;
		skeletonDoc.SetObject();
		if (!skeleton.Save(skeletonDoc))
		{
			IMZADI_LOG_ERROR("Failed to save skeleton.");
			return false;
		}

		Imzadi::SkinWeights skinWeights;
		if (!this->GenerateSkinWeights(skinWeights, mesh))
		{
			IMZADI_LOG_ERROR("Failed to generate skin weights.");
			return false;
		}

		rapidjson::Document skinWeightsDoc;
		skinWeightsDoc.SetObject();
		if (!skinWeights.Save(skinWeightsDoc))
		{
			IMZADI_LOG_ERROR("Failed to save skin-weights.");
			return false;
		}

		if (!JsonUtils::WriteJsonFile(skeletonDoc, skeletonFileName.GetFullPath()))
			return false;

		if (!JsonUtils::WriteJsonFile(skinWeightsDoc, skinWeightsFileName.GetFullPath()))
			return false;

		rapidjson::Value animationsArrayValue;
		animationsArrayValue.SetArray();
		this->GatherApplicableAnimations(animationsArrayValue, &skeleton, &meshDoc);
		meshDoc.AddMember("animations", animationsArrayValue, meshDoc.GetAllocator());
	}

	if (!this->ProcessMeshPorts(scene, node, worldToObject, meshDoc))
		return false;

	if (!JsonUtils::WriteJsonFile(meshDoc, meshFileName.GetFullPath()))
		return false;

	if (!JsonUtils::WriteJsonFile(verticesDoc, vertexBufferFileName.GetFullPath()))
		return false;

	if (!JsonUtils::WriteJsonFile(indicesDoc, indexBufferFileName.GetFullPath()))
		return false;

	return true;
}

bool Converter::ProcessMeshPorts(const aiScene* scene, const aiNode* node, const Imzadi::Transform& worldToObject, rapidjson::Document& meshDoc)
{
	std::unordered_map<std::string, Imzadi::Transform> portMap;

	for (int i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* portNode = node->mChildren[i];
		wxString portName(portNode->mName.C_Str());
		if (portName.find("Port") != 0)
			continue;

		Imzadi::Transform nodeToWorld;
		if (!this->GetNodeToWorldTransform(portNode, nodeToWorld))
		{
			IMZADI_LOG_ERROR("Error: Failed to get node-to-world transform for port node: %s", (const char*)portName.c_str());
			return false;
		}

		Imzadi::Transform nodeToObject = worldToObject * nodeToWorld;
		nodeToObject.matrix = nodeToObject.matrix.Orthonormalized(IMZADI_AXIS_FLAG_X);
		portMap.insert(std::pair(portName, nodeToObject));
	}

	if (portMap.size() > 0)
	{
		rapidjson::Value portMapObject;
		portMapObject.SetObject();

		for (auto pair : portMap)
		{
			const std::string& portName = pair.first;
			const Imzadi::Transform& nodeToWorld = pair.second;

			rapidjson::Value transformValue;
			Imzadi::Asset::SaveTransform(transformValue, nodeToWorld, &meshDoc);

			portMapObject.AddMember(rapidjson::Value().SetString(portName.c_str(), meshDoc.GetAllocator()), transformValue, meshDoc.GetAllocator());
		}

		meshDoc.AddMember("port_map", portMapObject, meshDoc.GetAllocator());
	}

	return true;
}

void Converter::GatherApplicableAnimations(rapidjson::Value& animationsArrayValue, const Imzadi::Skeleton* skeleton, rapidjson::Document* doc)
{
	wxString animationsFolder = wxGetApp().GetAssetsRootFolder() + "/Animations";
	std::filesystem::path animationsFolderPath((const char*)animationsFolder.c_str());

	try
	{
		for (const auto dirEntry : std::filesystem::recursive_directory_iterator(animationsFolderPath))
		{
			if (dirEntry.is_regular_file())
			{
				std::string ext = dirEntry.path().extension().string();
				if (ext == ".animation")
				{
					wxString animationPath(dirEntry.path().c_str());
					if (this->IsAnimationApplicable(animationPath, skeleton))
					{
						wxString animationRef = wxGetApp().MakeAssetFileReference(animationPath);
						rapidjson::Value animationRefValue;
						animationRefValue.SetString(animationRef, doc->GetAllocator());
						animationsArrayValue.PushBack(animationRefValue, doc->GetAllocator());
					}
				}
			}
		}
	}
	catch (std::filesystem::filesystem_error error)
	{
		IMZADI_LOG_ERROR("Error: %s", error.what());
	}
}

bool Converter::IsAnimationApplicable(const wxString& animationFile, const Imzadi::Skeleton* skeleton)
{
	rapidjson::Document animDoc;
	if (!JsonUtils::ReadJsonFile(animDoc, animationFile))
	{
		IMZADI_LOG_WARNING("Warning: Could not open file: %s", (const char*)animationFile.c_str());
		return false;
	}

	Imzadi::Animation animation;
	if (!animation.Load(animDoc, nullptr))
	{
		IMZADI_LOG_WARNING("Warning: Could not load animation file: %s", (const char*)animationFile.c_str());
		return false;
	}

	return animation.CanAnimateSkeleton(skeleton, 0.5);
}

bool Converter::GenerateSkeleton(Imzadi::Skeleton& skeleton, const aiMesh* mesh)
{
	if (mesh->mNumBones == 0)
	{
		IMZADI_LOG_ERROR("No bones found!");
		return false;
	}

	// It is not obvious at all what subset of the node hierarchy actually constitutes the skeleton.
	// I get the impression that all this data is just a ridiculous mess.
	std::unordered_set<const aiNode*> boneSet;
	for (int i = 0; i < mesh->mNumBones; i++)
	{
		const aiBone* bone = mesh->mBones[i];
		boneSet.insert(bone->mNode);
	}

	const aiNode* rootBoneNode = nullptr;
	for (int i = 0; i < mesh->mNumBones; i++)
	{
		const aiBone* bone = mesh->mBones[i];
		if (!this->FindParentBones(bone->mNode, boneSet))
		{
			if (!rootBoneNode)
				rootBoneNode = bone->mNode;
			else
			{
				IMZADI_LOG_ERROR("Error: There can be only one root bone node, but found another!");
				return false;
			}
		}

		for (int j = 0; j < bone->mNode->mNumChildren; j++)
			boneSet.insert(bone->mNode->mChildren[j]);
	}

	if (!rootBoneNode)
	{
		IMZADI_LOG_ERROR("Error: Did not find root bone of skeleton.");
		return false;
	}

	std::string desiredRootName(rootBoneNode->mName.C_Str());

	while (rootBoneNode->mParent)
	{
		rootBoneNode = rootBoneNode->mParent;
		boneSet.insert(rootBoneNode);
	}

	skeleton.SetRootBone(new Imzadi::Bone());
	if (!this->GenerateSkeleton(skeleton.GetRootBone(), rootBoneNode, boneSet))
		return false;

	while (true)
	{
		Imzadi::Bone* rootBone = skeleton.GetRootBone();
		if (rootBone->GetName() == desiredRootName)
			break;

		if (!skeleton.ChopRoot())
		{
			IMZADI_LOG_ERROR("Error: Failed to chop root of skeleton tree.");
			return false;
		}
	}

	return true;
}

bool Converter::FindParentBones(const aiNode* boneNode, std::unordered_set<const aiNode*>& boneSet)
{
	const aiNode* parentNode = boneNode->mParent;
	if (!parentNode)
		return false;

	if (boneSet.find(parentNode) != boneSet.end())
		return true;

	if (!this->FindParentBones(parentNode, boneSet))
		return false;

	boneSet.insert(parentNode);
	return true;
}

bool Converter::GenerateSkeleton(Imzadi::Bone* bone, const aiNode* boneNode, const std::unordered_set<const aiNode*>& boneSet)
{
	if (boneSet.find(boneNode) == boneSet.end())
		return false;

	bone->SetName(boneNode->mName.C_Str());
	
	Imzadi::Transform childToParent;
	if (!this->MakeTransform(childToParent, boneNode->mTransformation))
		return false;

	bone->SetBindPoseChildToParent(childToParent);

	for (int i = 0; i < boneNode->mNumChildren; i++)
	{
		const aiNode* childNode = boneNode->mChildren[i];
		if (boneSet.find(childNode) != boneSet.end())
		{
			auto childBone = new Imzadi::Bone();
			bone->AddChildBone(childBone);
			if (!this->GenerateSkeleton(childBone, childNode, boneSet))
				return false;
		}
	}

	return true;
}

bool Converter::GenerateSkinWeights(Imzadi::SkinWeights& skinWeights, const aiMesh* mesh)
{
	skinWeights.SetNumVertices(mesh->mNumVertices);

	for (int i = 0; i < mesh->mNumBones; i++)
	{
		const aiBone* bone = mesh->mBones[i];

		for (int j = 0; j < bone->mNumWeights; j++)
		{
			const aiVertexWeight* vertexWeight = &bone->mWeights[j];

			if (vertexWeight->mVertexId >= skinWeights.GetNumVertices())
			{
				IMZADI_LOG_ERROR("Error: Vertex weight index (%d) out of range (max: %d).", vertexWeight->mVertexId, skinWeights.GetNumVertices() - 1);
				return false;
			}

			std::vector<Imzadi::SkinWeights::BoneWeight>& boneWeightArray = skinWeights.GetBonesWeightsForVertex(vertexWeight->mVertexId);
			Imzadi::SkinWeights::BoneWeight boneWeight;
			boneWeight.boneName = bone->mName.C_Str();
			boneWeight.weight = vertexWeight->mWeight;
			boneWeightArray.push_back(boneWeight);
		}
	}

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

bool Converter::MakeQuat(Imzadi::Quaternion& quaternionOut, const aiQuaternion& quaternionIn)
{
	quaternionOut.w = quaternionIn.w;
	quaternionOut.x = quaternionIn.x;
	quaternionOut.y = quaternionIn.y;
	quaternionOut.z = quaternionIn.z;
	return true;
}