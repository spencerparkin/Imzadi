#include "Animation.h"
#include "Skeleton.h"
#include "Error.h"
#include "Math/Interval.h"
#include <algorithm>
#include <unordered_set>

using namespace Imzadi;

//------------------------------- Animation -------------------------------

Animation::Animation()
{
	this->name = "Unknown";
}

/*virtual*/ Animation::~Animation()
{
	this->Clear();
}

/*virtual*/ bool Animation::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	this->Clear();

	if (!jsonDoc.IsObject())
	{
		IMZADI_ERROR("Given JSON doc was not an object.");
		return false;
	}

	if (!jsonDoc.HasMember("name") || !jsonDoc["name"].IsString())
	{
		IMZADI_ERROR("Animation has no name.");
		return false;
	}

	this->name = jsonDoc["name"].GetString();

	if (!jsonDoc.HasMember("key_frame_array") || !jsonDoc["key_frame_array"].IsArray())
	{
		IMZADI_ERROR("No \"key_frame_array\" found in JSON.");
		return false;
	}

	const rapidjson::Value& keyFrameArrayValue = jsonDoc["key_frame_array"];

	for (int i = 0; i < keyFrameArrayValue.Size(); i++)
	{
		const rapidjson::Value& keyFrameValue = keyFrameArrayValue[i];
		auto keyFrame = new KeyFrame();
		this->keyFrameArray.push_back(keyFrame);
		if (!keyFrame->Load(keyFrameValue))
		{
			IMZADI_ERROR(std::format("Failed to load key-frame {}.", i));
			return false;
		}
	}

	this->TimeSort();

	return true;
}

/*virtual*/ bool Animation::Unload()
{
	this->Clear();
	return true;
}

/*virtual*/ bool Animation::Save(rapidjson::Document& jsonDoc) const
{
	jsonDoc.SetObject();

	rapidjson::Value keyFrameArrayValue;
	keyFrameArrayValue.SetArray();

	for (const KeyFrame* keyFrame : this->keyFrameArray)
	{
		rapidjson::Value keyFrameValue;
		if (!keyFrame->Save(keyFrameValue, jsonDoc))
		{
			IMZADI_ERROR("Failed to save key-frame.");
			return false;
		}

		keyFrameArrayValue.PushBack(keyFrameValue, jsonDoc.GetAllocator());
	}

	jsonDoc.AddMember("key_frame_array", keyFrameArrayValue, jsonDoc.GetAllocator());
	jsonDoc.AddMember("name", rapidjson::Value().SetString(this->name.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());

	return true;
}

void Animation::Clear()
{
	for (KeyFrame* keyFrame : this->keyFrameArray)
		delete keyFrame;
	
	this->keyFrameArray.clear();
}

KeyFrame* Animation::GetKeyFrame(int i)
{
	if (i < 0 || i >= this->keyFrameArray.size())
		return nullptr;

	return this->keyFrameArray[i];
}

const KeyFrame* Animation::GetKeyFrame(int i) const
{
	return const_cast<Animation*>(this)->GetKeyFrame(i);
}

void Animation::AddKeyFrame(KeyFrame* keyFrame)
{
	this->keyFrameArray.push_back(keyFrame);
}

void Animation::TimeSort()
{
	std::sort(this->keyFrameArray.begin(), this->keyFrameArray.end(), [](const KeyFrame* frameA, const KeyFrame* frameB) -> bool {
		return frameA->GetTime() < frameB->GetTime();
	});
}

bool Animation::MakeCursorFromTime(Cursor& cursor, double timeSeconds) const
{
	if (this->keyFrameArray.size() < 2)
		return false;

	int i = 0;
	int j = this->keyFrameArray.size() - 1;

	while (i != j - 1)
	{
		const KeyFrame* keyFrameA = this->keyFrameArray[i];
		const KeyFrame* keyFrameB = this->keyFrameArray[j];

		Interval interval(keyFrameA->GetTime(), keyFrameB->GetTime());
		if (!interval.IsValid())
			return false;

		if (!interval.ContainsValue(timeSeconds))
			return false;

		int k = (i + j) / 2;
		if (k == i)
			k++;
		else if (k == j)
			k--;

		const KeyFrame* midKeyFrame = this->keyFrameArray[k];
		if (!interval.ContainsValue(midKeyFrame->GetTime()))
			return false;

		if (timeSeconds <= midKeyFrame->GetTime())
			j = k;
		else
			i = k;
	}

	cursor.timeSeconds = timeSeconds;
	cursor.i = i;
	return true;
}

bool Animation::FindKeyFramesFromTime(double timeSeconds, KeyFramePair& keyFramePair) const
{
	Cursor cursor;
	if (!this->MakeCursorFromTime(cursor, timeSeconds))
		return false;

	return this->GetKeyFramesFromCursor(cursor, keyFramePair);
}

bool Animation::GetKeyFramesFromCursor(const Cursor& cursor, KeyFramePair& keyFramePair) const
{
	int i = cursor.i;
	int j = i + 1;

	keyFramePair.lowerBound = this->GetKeyFrame(i);
	keyFramePair.upperBound = this->GetKeyFrame(j);

	return keyFramePair.lowerBound && keyFramePair.upperBound;
}

double Animation::GetStartTime() const
{
	if (this->keyFrameArray.size() == 0)
		return 0.0;

	return this->keyFrameArray[0]->GetTime();
}

double Animation::GetEndTime() const
{
	if (this->keyFrameArray.size() == 0)
		return 0.0;

	return this->keyFrameArray[this->keyFrameArray.size() - 1]->GetTime();
}

double Animation::GetDuration() const
{
	return this->GetEndTime() - this->GetStartTime();
}

bool Animation::AdvanceCursor(Cursor& cursor, double deltaTimeSeconds, bool loop) const
{
	cursor.timeSeconds += deltaTimeSeconds;

	// Typically we'd do one or two interations at most here.
	for (int i = 0; i < this->keyFrameArray.size(); i++)
	{
		KeyFramePair keyFramePair{};
		if (!this->GetKeyFramesFromCursor(cursor, keyFramePair))
			return false;

		Interval interval(keyFramePair.lowerBound->GetTime(), keyFramePair.upperBound->GetTime());
		if (!interval.IsValid())
			return false;

		if (interval.ContainsValue(cursor.timeSeconds))
			return true;

		if (cursor.timeSeconds > interval)
		{
			cursor.i++;
			if (cursor.i >= this->keyFrameArray.size() - 1)
			{
				if (!loop)
				{
					cursor.i--;
					cursor.timeSeconds = this->keyFrameArray[this->keyFrameArray.size() - 1]->GetTime();
					return false;
				}
				
				cursor.i = 0;
				cursor.timeSeconds -= this->GetDuration();
			}
		}
		else if (cursor.timeSeconds < interval)
		{
			cursor.i--;
			if (cursor.i < 0)
			{
				if (!loop)
				{
					cursor.i++;
					cursor.timeSeconds = this->keyFrameArray[0]->GetTime();
					return false;
				}

				cursor.i = this->keyFrameArray.size() - 2;
				cursor.timeSeconds += this->GetDuration();
			}
		}
	}
	
	return false;
}

bool Animation::CanAnimateSkeleton(const Skeleton* skeleton, double threshold) const
{
	std::unordered_set<std::string> boneSet;
	int matchCount = 0;

	for (const KeyFrame* keyFrame : this->keyFrameArray)
	{
		for (int i = 0; i < keyFrame->GetPoseCount(); i++)
		{
			const KeyFrame::PoseInfo& poseInfo = keyFrame->GetPoseInfo(i);
			if (boneSet.find(poseInfo.boneName) == boneSet.end())
			{
				boneSet.insert(poseInfo.boneName);
				if (skeleton->FindBone(poseInfo.boneName))
					matchCount++;
			}
		}
	}

	double matchRatio = double(matchCount) / double(boneSet.size());
	return matchRatio > threshold;
}

//------------------------------- KeyFrame -------------------------------

KeyFrame::KeyFrame()
{
	this->timeSeconds = 0.0;
}

/*virtual*/ KeyFrame::~KeyFrame()
{
}

void KeyFrame::Clear()
{
	this->poseInfoArray.clear();
}

void KeyFrame::Sort()
{
	std::sort(this->poseInfoArray.begin(), this->poseInfoArray.end(), [](const PoseInfo& infoA, const PoseInfo& infoB) -> bool {
		return ::strcmp(infoA.boneName.c_str(), infoB.boneName.c_str()) < 0;
	});
}

void KeyFrame::Copy(const KeyFrame& keyFrame)
{
	this->Clear();

	for (const PoseInfo& info : keyFrame.poseInfoArray)
		this->poseInfoArray.push_back(info);

	this->timeSeconds = keyFrame.timeSeconds;
}

bool KeyFrame::AddPoseInfo(const PoseInfo& poseInfo)
{
	for (int i = 0; i < this->poseInfoArray.size(); i++)
		if (this->poseInfoArray[i].boneName == poseInfo.boneName)
			return false;

	this->poseInfoArray.push_back(poseInfo);
	return true;
}

bool KeyFrame::Load(const rapidjson::Value& keyFrameValue)
{
	this->Clear();

	if (!keyFrameValue.IsObject())
	{
		IMZADI_ERROR("Key-frame JSON is not an object.");
		return false;
	}

	if (!keyFrameValue.HasMember("time") || !keyFrameValue["time"].IsFloat())
	{
		IMZADI_ERROR("Key-frame has no \"time\" member.");
		return false;
	}

	this->timeSeconds = keyFrameValue["time"].GetFloat();

	if (!keyFrameValue.HasMember("pose_info_array") || !keyFrameValue["pose_info_array"].IsArray())
	{
		IMZADI_ERROR("Key-frame has no \"pose_info_array\" member.");
		return false;
	}

	const rapidjson::Value& poseInfoArrayValue = keyFrameValue["pose_info_array"];

	for (int i = 0; i < poseInfoArrayValue.Size(); i++)
	{
		const rapidjson::Value& poseInfoValue = poseInfoArrayValue[i];
		if (!poseInfoValue.IsObject())
			return false;

		if (!poseInfoValue.HasMember("bone_name") || !poseInfoValue["bone_name"].IsString())
			return false;

		if (!poseInfoValue.HasMember("bone_child_to_parent"))
			return false;

		PoseInfo poseInfo;
		poseInfo.boneName = poseInfoValue["bone_name"].GetString();
		if (!Asset::LoadAnimTransform(poseInfoValue["bone_child_to_parent"], poseInfo.childToParent))
			return false;

		if (!this->AddPoseInfo(poseInfo))
		{
			IMZADI_ERROR(std::format("Failed to add pose info for bone \"{}\".", poseInfo.boneName.c_str()));
			return false;
		}
	}

	return true;
}

bool KeyFrame::Save(rapidjson::Value& keyFrameValue, rapidjson::Document& jsonDoc) const
{
	keyFrameValue.SetObject();
	keyFrameValue.AddMember("time", rapidjson::Value().SetFloat(this->timeSeconds), jsonDoc.GetAllocator());

	rapidjson::Value poseInfoArrayValue;
	poseInfoArrayValue.SetArray();

	for (const PoseInfo& poseInfo : this->poseInfoArray)
	{
		rapidjson::Value poseInfoValue;
		poseInfoValue.SetObject();

		rapidjson::Value childToParentValue;
		Asset::SaveAnimTransform(childToParentValue, poseInfo.childToParent, &jsonDoc);

		poseInfoValue.AddMember("bone_name", rapidjson::Value().SetString(poseInfo.boneName.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
		poseInfoValue.AddMember("bone_child_to_parent", childToParentValue, jsonDoc.GetAllocator());

		poseInfoArrayValue.PushBack(poseInfoValue, jsonDoc.GetAllocator());
	}

	keyFrameValue.AddMember("pose_info_array", poseInfoArrayValue, jsonDoc.GetAllocator());

	return true;
}

bool KeyFrame::Interpolate(const KeyFramePair& keyFramePair, double timeSeconds)
{
	if (keyFramePair.lowerBound->poseInfoArray.size() != keyFramePair.upperBound->poseInfoArray.size())
		return false;

	Interval interval(keyFramePair.lowerBound->GetTime(), keyFramePair.upperBound->GetTime());
	if (!interval.IsValid())
		return false;

	double alpha = interval.Alpha(timeSeconds);
	if (alpha < 0.0 || alpha > 1.0)
		return false;

	this->Clear();

	for (int i = 0; i < keyFramePair.lowerBound->poseInfoArray.size(); i++)
	{
		const PoseInfo& poseInfoA = keyFramePair.lowerBound->poseInfoArray[i];
		const PoseInfo& poseInfoB = keyFramePair.upperBound->poseInfoArray[i];

		if (poseInfoA.boneName != poseInfoB.boneName)
			return false;

		PoseInfo poseInfo;
		poseInfo.boneName = poseInfoA.boneName;
		poseInfo.childToParent.Interpolate(poseInfoA.childToParent, poseInfoB.childToParent, alpha);
		this->poseInfoArray.push_back(poseInfo);
	}

	return true;
}

int KeyFrame::PoseSkeleton(Skeleton* skeleton) const
{
	int poseCount = 0;

	for (const PoseInfo& poseInfo : this->poseInfoArray)
	{
		Bone* bone = skeleton->FindBone(poseInfo.boneName);
		if (bone)
		{
			Transform childToParent;
			poseInfo.childToParent.GetToTransform(childToParent);
			bone->SetCurrentPoseChildToParent(childToParent);
			poseCount++;
		}
	}

	return poseCount;
}

bool KeyFrame::MakePoseFromSkeleton(const Skeleton* skeleton)
{
	this->Clear();

	std::vector<Bone*> boneArray;
	const_cast<Skeleton*>(skeleton)->GatherBones(boneArray);

	for (const Bone* bone : boneArray)
	{
		const Bone::Transforms* transform = bone->GetTransforms(BoneTransformType::CURRENT_POSE);

		PoseInfo poseInfo;
		poseInfo.boneName = bone->GetName();

		if (!poseInfo.childToParent.SetFromTransform(transform->childToParent))
			return false;

		if (!this->AddPoseInfo(poseInfo))
			return false;
	}

	return true;
}