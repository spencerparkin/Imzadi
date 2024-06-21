#include "SkinnedRenderMesh.h"
#include "RenderObjects/AnimatedMeshInstance.h"
#include "Skeleton.h"
#include "SkinWeights.h"
#include "Game.h"
#include "Log.h"

using namespace Imzadi;

SkinnedRenderMesh::SkinnedRenderMesh()
{
	this->positionOffset = 0;
	this->normalOffset = 0;
}

/*virtual*/ SkinnedRenderMesh::~SkinnedRenderMesh()
{
}

/*virtual*/ bool SkinnedRenderMesh::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!RenderMeshAsset::Load(jsonDoc, assetCache))
	{
		IMZADI_LOG_ERROR("Failed to load underlying render mesh of skinned mesh.");
		return false;
	}

	if (!jsonDoc.HasMember("skeleton") || !jsonDoc["skeleton"].IsString())
	{
		IMZADI_LOG_ERROR("No \"skeleton\" member or it's not a string.");
		return false;
	}

	std::string skeletonFile = jsonDoc["skeleton"].GetString();
	Reference<Asset> asset;
	if (!assetCache->LoadAsset(skeletonFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load skeleton file: " + skeletonFile);
		return false;
	}

	this->skeleton.SafeSet(asset.Get());
	if (!this->skeleton)
	{
		IMZADI_LOG_ERROR(std::format("Whatever loaded from file {} was not a skeleton.", skeletonFile.c_str()));
		return false;
	}

	if (!this->vertexBuffer->GetBareBuffer(this->bindPoseVertices))
	{
		IMZADI_LOG_ERROR("Failed to get bare-buffer for bind pose vertices.");
		return false;
	}

	this->currentPoseVertices.Set(this->bindPoseVertices->Clone());

	if (!jsonDoc.HasMember("skin_weights") || !jsonDoc["skin_weights"].IsString())
	{
		IMZADI_LOG_ERROR("No \"skin_weights\" member of it's not a string.");
		return false;
	}

	std::string skinWeightsFile = jsonDoc["skin_weights"].GetString();
	if (!assetCache->LoadAsset(skinWeightsFile, asset))
	{
		IMZADI_LOG_ERROR("Failed to load skin-weights file: " + skinWeightsFile);
		return false;
	}

	this->skinWeights.SafeSet(asset.Get());
	if (!skinWeights)
	{
		IMZADI_LOG_ERROR(std::format("Whatever loaded from file {} was not skin-weights.", skinWeightsFile.c_str()));
		return false;
	}

	if (!jsonDoc.HasMember("position_offset") || !jsonDoc["position_offset"].IsInt())
	{
		IMZADI_LOG_ERROR("No \"position_offset\" member or it's not an int.");
		return false;
	}

	this->positionOffset = jsonDoc["position_offset"].GetInt();

	if (!jsonDoc.HasMember("normal_offset") || !jsonDoc["normal_offset"].IsInt())
	{
		IMZADI_LOG_ERROR("No \"normal_offset\" member or it's not an int.");
		return false;
	}

	this->normalOffset = jsonDoc["normal_offset"].GetInt();

	uint32_t strideBytes = this->vertexBuffer->GetStride();

	if (this->positionOffset + 3 * sizeof(float) > strideBytes)
	{
		IMZADI_LOG_ERROR(std::format("The position offset {} plus size {} overflows the stride size {}.", this->positionOffset, 3 * sizeof(float), strideBytes));
		return false;
	}

	if (this->normalOffset + 3 * sizeof(float) > strideBytes)
	{
		IMZADI_LOG_ERROR(std::format("The normal offset {} plus size {} overflows the stride size {}.", this->normalOffset, 3 * sizeof(float), strideBytes));
		return false;
	}

	if (jsonDoc.HasMember("animations") && jsonDoc["animations"].IsArray())
	{
		this->animationMap.clear();

		const rapidjson::Value& animationsArrayValue = jsonDoc["animations"];
		for (int i = 0; i < animationsArrayValue.Size(); i++)
		{
			const rapidjson::Value& animationValue = animationsArrayValue[i];
			if (!animationValue.IsString())
			{
				IMZADI_LOG_ERROR("Expected animation entry to be a string.");
				return false;
			}

			std::string animationFile = animationValue.GetString();
			if (!assetCache->LoadAsset(animationFile, asset))
			{
				IMZADI_LOG_ERROR("Failed to load animation file: " + animationFile);
				return false;
			}

			Reference<Animation> animation;
			animation.SafeSet(asset.Get());
			if (!animation)
			{
				IMZADI_LOG_ERROR(std::format("Whatever loaded from file {} was not an animation.", animationFile.c_str()));
				return false;
			}

			if (this->animationMap.find(animation->GetName()) != this->animationMap.end())
			{
				IMZADI_LOG_ERROR(std::format("The animation name \"{}\" is already used.", animation->GetName().c_str()));
				return false;
			}

			this->animationMap.insert(std::pair<std::string, Reference<Animation>>(animation->GetName(), animation));
		}
	}

	return true;
}

/*virtual*/ bool SkinnedRenderMesh::Unload()
{
	RenderMeshAsset::Unload();

	return true;
}

void SkinnedRenderMesh::GetAnimationNames(std::unordered_set<std::string>& animationNameSet)
{
	animationNameSet.clear();
	for (auto pair : this->animationMap)
		animationNameSet.insert(pair.first);
}

void SkinnedRenderMesh::DeformMesh()
{
	uint32_t numVertices = this->vertexBuffer->GetNumElements();
	uint32_t strideBytes = this->vertexBuffer->GetStride();
	BYTE* bindPoseBuffer = this->bindPoseVertices->GetBuffer();
	BYTE* currentPoseBuffer = this->currentPoseVertices->GetBuffer();

	for (uint32_t i = 0; i < numVertices; i++)
	{
		const std::vector<SkinWeights::BoneWeight>& boneWeightArray = this->skinWeights->GetBoneWeightsForVertex(i);

		const float* bindPosePositionBuffer = (float*)&bindPoseBuffer[i * strideBytes + this->positionOffset];
		float* currentPosePositionBuffer = (float*)&currentPoseBuffer[i * strideBytes + this->positionOffset];

		const float* bindPoseNormalBuffer = (float*)&bindPoseBuffer[i * strideBytes + this->normalOffset];
		float* currentPoseNormalBuffer = (float*)&currentPoseBuffer[i * strideBytes + this->normalOffset];

		Vector3 bindPosePosition;
		bindPosePosition.x = bindPosePositionBuffer[0];
		bindPosePosition.y = bindPosePositionBuffer[1];
		bindPosePosition.z = bindPosePositionBuffer[2];

		Vector3 bindPoseNormal;
		bindPoseNormal.x = bindPoseNormalBuffer[0];
		bindPoseNormal.y = bindPoseNormalBuffer[1];
		bindPoseNormal.z = bindPoseNormalBuffer[2];

		Vector3 currentPosePosition(0.0, 0.0, 0.0);
		Vector3 currentPoseNormal(0.0, 0.0, 0.0);

		for (uint32_t j = 0; j < boneWeightArray.size(); j++)
		{
			const SkinWeights::BoneWeight& boneWeight = boneWeightArray[j];
			const Bone* bone = this->skeleton->FindBone(boneWeight.boneName);
			assert(bone != nullptr);

			const Bone::Transforms* bindPoseTransforms = bone->GetTransforms(BoneTransformType::BIND_POSE);
			const Bone::Transforms* currentPoseTransforms = bone->GetTransforms(BoneTransformType::CURRENT_POSE);

			Vector3 skinPoint = bindPoseTransforms->objectToBone.TransformPoint(bindPosePosition);
			skinPoint = currentPoseTransforms->boneToObject.TransformPoint(skinPoint);

			Vector3 skinNormal = bindPoseTransforms->objectToBone.TransformVector(bindPoseNormal);
			skinNormal = currentPoseTransforms->boneToObject.TransformVector(skinNormal);
			
			currentPosePosition += skinPoint * boneWeight.weight;
			currentPoseNormal += skinNormal * boneWeight.weight;
		}
		
		if (!currentPoseNormal.Normalize())
			currentPoseNormal.SetComponents(0.0, 0.0, 1.0);

		currentPosePositionBuffer[0] = currentPosePosition.x;
		currentPosePositionBuffer[1] = currentPosePosition.y;
		currentPosePositionBuffer[2] = currentPosePosition.z;

		currentPoseNormalBuffer[0] = currentPoseNormal.x;
		currentPoseNormalBuffer[1] = currentPoseNormal.y;
		currentPoseNormalBuffer[2] = currentPoseNormal.z;
	}

	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	// Note that we read form a bare buffer and wrote into a bare buffer beforehand so that
	// here we would be keeping the GPU buffer locked for as short a time as possible.
	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	HRESULT result = deviceContext->Map(this->vertexBuffer->GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("Skin mapping call failed with error code: {}", result));
		return;
	}

	::memcpy(mappedSubresource.pData, this->currentPoseVertices->GetBuffer(), this->currentPoseVertices->GetSize());
	deviceContext->Unmap(this->vertexBuffer->GetBuffer(), 0);
}

/*virtual*/ bool SkinnedRenderMesh::MakeRenderInstance(Reference<RenderObject>& renderObject)
{
	renderObject.Set(new AnimatedMeshInstance());
	auto instance = dynamic_cast<AnimatedMeshInstance*>(renderObject.Get());
	instance->SetRenderMesh(this);
	instance->SetBoundingBox(this->objectSpaceBoundingBox);
	instance->SetSkinnedMesh(this);
	return true;
}

Animation* SkinnedRenderMesh::GetAnimation(const std::string& animationName)
{
	AnimationMap::iterator iter = this->animationMap.find(animationName);
	if (iter == this->animationMap.end())
		return nullptr;

	return iter->second;
}