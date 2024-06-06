#include "SkinWeights.h"
#include "Buffer.h"
#include "Skeleton.h"
#include "Math/Vector3.h"

using namespace Collision;

SkinWeights::SkinWeights()
{
}

/*virtual*/ SkinWeights::~SkinWeights()
{
}

/*virtual*/ bool SkinWeights::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	return false;
}

/*virtual*/ bool SkinWeights::Unload()
{
	return true;
}

/*virtual*/ bool SkinWeights::Save(rapidjson::Document& jsonDoc) const
{
	return false;
}

void SkinWeights::Clear()
{
	this->weightedVertexArray.clear();
}

bool SkinWeights::AutoSkin(const Skeleton* skeleton, const BareBuffer* bindPoseVertexBuffer, uint32_t elementStride, uint32_t vertexOffset, double radius)
{
	if (bindPoseVertexBuffer->GetSize() % elementStride != 0)
		return false;

	if (vertexOffset + 3 * sizeof(float) > elementStride)
		return false;

	if (!skeleton->UpdateCachedTransforms())
		return false;

	this->Clear();

	const BYTE* vertexBuffer = bindPoseVertexBuffer->GetBuffer();
	uint32_t numVertices = bindPoseVertexBuffer->GetSize() / elementStride;
	for (uint32_t i = 0; i < numVertices; i++)
	{
		float* positionBuffer = (float*)&vertexBuffer[i * elementStride + vertexOffset];
		Vector3 position;
		position.x = *positionBuffer++;
		position.y = *positionBuffer++;
		position.z = *positionBuffer++;

		std::vector<const Bone*> boneArray;
		if (!skeleton->GatherBones(position, boneArray))
			return false;

		if (boneArray.size() == 0)
			return false;

		std::vector<BoneWeight> boneWeightArray;

		for (const Bone* bone : boneArray)
		{
			double distance = (position - bone->GetBindPoseChildToObject().translation).Length();
			BoneWeight boneWeight;
			boneWeight.weight = radius - distance;
			boneWeight.boneName = bone->GetName();
			if (boneWeight.weight <= 0.0)
				break;

			boneWeightArray.push_back(boneWeight);
		}

		if (boneWeightArray.size() > 0)
			this->NormalizeWeights(boneWeightArray);
		else
		{
			BoneWeight boneWeight;
			boneWeight.weight = 1.0;
			boneWeight.boneName = boneArray[0]->GetName();
			boneWeightArray.push_back(boneWeight);
		}

		this->weightedVertexArray.push_back(boneWeightArray);
	}

	return true;
}

void SkinWeights::NormalizeWeights(std::vector<BoneWeight>& boneWeightArray)
{
	double weightSum = 0.0;
	for (BoneWeight& boneWeight : boneWeightArray)
		weightSum += boneWeight.weight;

	for (BoneWeight& boneWeight : boneWeightArray)
		boneWeight.weight /= weightSum;
}