#include "SkinWeights.h"
#include "Buffer.h"
#include "Skeleton.h"
#include "Math/Vector3.h"

using namespace Imzadi;

SkinWeights::SkinWeights()
{
}

/*virtual*/ SkinWeights::~SkinWeights()
{
}

/*virtual*/ bool SkinWeights::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("weighted_vertices"))
		return false;

	const rapidjson::Value& weightedVerticesArrayValue = jsonDoc["weighted_vertices"];
	if (!weightedVerticesArrayValue.IsArray())
		return false;

	this->weightedVertexArray.clear();

	for (int i = 0; i < weightedVerticesArrayValue.Size(); i++)
	{
		const rapidjson::Value& weightedVertexValue = weightedVerticesArrayValue[i];
		if (!weightedVertexValue.IsArray())
			return false;

		std::vector<BoneWeight> boneWeightArray;

		for (int j = 0; j < weightedVertexValue.Size(); j++)
		{
			const rapidjson::Value& boneWeightValue = weightedVertexValue[j];
			if (!boneWeightValue.IsObject())
				return false;

			if (!boneWeightValue.HasMember("bone_name") || !boneWeightValue["bone_name"].IsString())
				return false;

			if (!boneWeightValue.HasMember("weight") || !boneWeightValue["weight"].IsFloat())
				return false;

			BoneWeight boneWeight;
			boneWeight.boneName = boneWeightValue["bone_name"].GetString();
			boneWeight.weight = boneWeightValue["weight"].GetFloat();
			boneWeightArray.push_back(boneWeight);
		}

		this->weightedVertexArray.push_back(boneWeightArray);
	}

	return true;
}

/*virtual*/ bool SkinWeights::Unload()
{
	return true;
}

/*virtual*/ bool SkinWeights::Save(rapidjson::Document& jsonDoc) const
{
	jsonDoc.SetObject();

	rapidjson::Value weightedVerticesArrayValue;
	weightedVerticesArrayValue.SetArray();

	for (int i = 0; i < (signed)this->weightedVertexArray.size(); i++)
	{
		const std::vector<BoneWeight>& boneWeightArray = this->weightedVertexArray[i];

		rapidjson::Value weightedVertexValue;
		weightedVertexValue.SetArray();

		for (int j = 0; j < (signed)boneWeightArray.size(); j++)
		{
			const BoneWeight& boneWeight = boneWeightArray[j];

			rapidjson::Value boneWeightValue;
			boneWeightValue.SetObject();
			boneWeightValue.AddMember("bone_name", rapidjson::Value().SetString(boneWeight.boneName.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
			boneWeightValue.AddMember("weight", rapidjson::Value().SetFloat(boneWeight.weight), jsonDoc.GetAllocator());

			weightedVertexValue.PushBack(boneWeightValue, jsonDoc.GetAllocator());
		}

		weightedVerticesArrayValue.PushBack(weightedVertexValue, jsonDoc.GetAllocator());
	}

	jsonDoc.AddMember("weighted_vertices", weightedVerticesArrayValue, jsonDoc.GetAllocator());

	return true;
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

	skeleton->UpdateCachedTransforms(BoneTransformType::BIND_POSE);

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

		std::vector<Bone*> boneArray;
		if (!const_cast<Skeleton*>(skeleton)->GatherBones(position, BoneTransformType::BIND_POSE, boneArray))
			return false;

		if (boneArray.size() == 0)
			return false;

		std::vector<BoneWeight> boneWeightArray;

		for (const Bone* bone : boneArray)
		{
			if (!bone->GetWeightable())
				continue;

			double distance = (position - bone->CalcObjectSpaceCenter(BoneTransformType::BIND_POSE)).Length();
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
			const Bone* foundBone = nullptr;
			for (const Bone* bone : boneArray)
			{
				if (bone->GetWeightable())
				{
					foundBone = bone;
					break;
				}
			}
			
			if (!foundBone)
				return false;

			BoneWeight boneWeight;
			boneWeight.weight = 1.0;
			boneWeight.boneName = foundBone->GetName();
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