#include "Animation.h"
#include "Math/Interval.h"
#include <algorithm>

using namespace Collision;

//------------------------------- Animation -------------------------------

Animation::Animation()
{
}

/*virtual*/ Animation::~Animation()
{
	this->Clear();
}

/*virtual*/ bool Animation::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	this->Clear();

	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("key_frame_array") || !jsonDoc["key_frame_array"].IsArray())
		return false;

	const rapidjson::Value& keyFrameArrayValue = jsonDoc["key_frame_array"];

	for (int i = 0; i < keyFrameArrayValue.Size(); i++)
	{
		const rapidjson::Value& keyFrameValue = keyFrameArrayValue[i];
		auto keyFrame = new KeyFrame();
		this->keyFrameArray.push_back(keyFrame);
		if (!keyFrame->Load(keyFrameValue))
			return false;
	}

	this->TimeSort();

	return true;
}

/*virtual*/ bool Animation::Unload()
{
	return false;
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
			return false;

		keyFrameArrayValue.PushBack(keyFrameValue, jsonDoc.GetAllocator());
	}

	jsonDoc.AddMember("key_frame_array", keyFrameArrayValue, jsonDoc.GetAllocator());

	return true;
}

void Animation::Clear()
{
	for (KeyFrame* keyFrame : this->keyFrameArray)
		delete keyFrame;
	
	this->keyFrameArray.clear();
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

bool Animation::FindKeyFramesFromTime(const KeyFrame*& keyFrameA, const KeyFrame*& keyFrameB, double timeSeconds) const
{
	if (this->keyFrameArray.size() == 0)
		return false;

	int i = 0;
	int j = this->keyFrameArray.size() - 1;

	while (true)
	{
		keyFrameA = this->keyFrameArray[i];
		keyFrameB = this->keyFrameArray[j];

		if (i == j - 1)
			break;

		Interval interval(keyFrameA->GetTime(), keyFrameB->GetTime());
		if (!interval.IsValid())
			return false;

		if (!interval.ContainsValue(timeSeconds))
			return false;

		int k = (i + j) / 2;
		const KeyFrame* midKeyFrame = this->keyFrameArray[k];
		if (!interval.ContainsValue(midKeyFrame->GetTime()))
			return false;

		if (timeSeconds <= midKeyFrame->GetTime())
			j = k;
		else
			i = k;
	}

	return true;
}

bool Animation::GetKeyFramesFromCursor(const Cursor& cursor, const KeyFrame*& keyFrameA, const KeyFrame*& keyFrameB) const
{
	int i = cursor.i;
	if (!(0 <= i && i < this->keyFrameArray.size()))
		return false;

	int j = i + 1;
	if (cursor.loop)
		j = j % this->keyFrameArray.size();

	if (!(0 <= j && j < this->keyFrameArray.size()))
		return false;

	keyFrameA = this->keyFrameArray[i];
	keyFrameB = this->keyFrameArray[j];
	return true;
}

bool Animation::AdvanceCursor(Cursor& cursor, double deltaTimeSeconds) const
{
	cursor.timeSeconds += deltaTimeSeconds;

	// Typically we'd do one or two interations at most here.
	for (int i = 0; i < this->keyFrameArray.size(); i++)
	{
		const KeyFrame* keyFrameA = nullptr;
		const KeyFrame* keyFrameB = nullptr;
		if (!this->GetKeyFramesFromCursor(cursor, keyFrameA, keyFrameB))
			return false;

		Interval interval(keyFrameA->GetTime(), keyFrameB->GetTime());
		if (!interval.IsValid())
			return false;

		if (interval.ContainsValue(cursor.timeSeconds))
			return true;

		if (cursor.timeSeconds > interval)
		{
			cursor.i++;
			if (cursor.i >= this->keyFrameArray.size())
			{
				if (cursor.loop)
					cursor.i = 0;
				else
					return false;
			}
		}
		else if (cursor.timeSeconds < interval)
		{
			cursor.i--;
			if (cursor.i < 0)
			{
				if (cursor.loop)
					cursor.i = this->keyFrameArray.size() - 1;
				else
					return false;
			}
		}
	}
	
	return false;
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

bool KeyFrame::Load(const rapidjson::Value& keyFrameValue)
{
	this->Clear();

	if (!keyFrameValue.IsObject())
		return false;

	if (!keyFrameValue.HasMember("time") || !keyFrameValue["time"].IsFloat())
		return false;

	this->timeSeconds = keyFrameValue["time"].GetFloat();

	if (!keyFrameValue.HasMember("pose_info_array") || !keyFrameValue["pose_info_array"].IsArray())
		return false;

	const rapidjson::Value& poseInfoArrayValue = keyFrameValue["pose_info_array"];

	for (int i = 0; i < poseInfoArrayValue.Size(); i++)
	{
		const rapidjson::Value& poseInfoValue = poseInfoArrayValue[i];
		if (!poseInfoValue.IsObject())
			return false;

		if (!poseInfoValue.HasMember("bone_name") || !poseInfoValue["bone_name"].IsString())
			return false;

		if (!poseInfoValue.HasMember("bone_orientation"))
			return false;

		if (!poseInfoValue.HasMember("bone_length") || !poseInfoValue["bone_length"].IsFloat())
			return false;

		PoseInfo poseInfo;
		poseInfo.boneName = poseInfoValue["bone_name"].GetString();
		poseInfo.boneState.length = poseInfoValue["bone_length"].GetFloat();
		if (!Asset::LoadMatrix(poseInfoValue["bone_orientation"], poseInfo.boneState.orientation))
			return false;

		this->poseInfoArray.push_back(poseInfo);
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

		rapidjson::Value orientationValue;
		Asset::SaveMatrix(orientationValue, poseInfo.boneState.orientation, &jsonDoc);

		poseInfoValue.AddMember("bone_name", rapidjson::Value().SetString(poseInfo.boneName.c_str(), jsonDoc.GetAllocator()), jsonDoc.GetAllocator());
		poseInfoValue.AddMember("bone_orientation", orientationValue, jsonDoc.GetAllocator());
		poseInfoValue.AddMember("bone_length", rapidjson::Value().SetFloat(poseInfo.boneState.length), jsonDoc.GetAllocator());

		poseInfoArrayValue.PushBack(poseInfoValue, jsonDoc.GetAllocator());
	}

	keyFrameValue.AddMember("pose_info_array", poseInfoArrayValue, jsonDoc.GetAllocator());

	return true;
}

bool KeyFrame::Interpolate(const KeyFrame* keyFrameA, const KeyFrame* keyFrameB, double timeSeconds)
{
	if (keyFrameA->poseInfoArray.size() != keyFrameB->poseInfoArray.size())
		return false;

	Interval interval(keyFrameA->GetTime(), keyFrameB->GetTime());
	if (!interval.IsValid())
		return false;

	double alpha = interval.Alpha(timeSeconds);
	if (alpha < 0.0 || alpha > 1.0)
		return false;

	this->Clear();

	for (int i = 0; i < keyFrameA->poseInfoArray.size(); i++)
	{
		const PoseInfo& poseInfoA = keyFrameA->poseInfoArray[i];
		const PoseInfo& poseInfoB = keyFrameB->poseInfoArray[i];

		if (poseInfoA.boneName != poseInfoB.boneName)
			return false;

		PoseInfo poseInfo;
		poseInfo.boneName = poseInfoA.boneName;
		poseInfo.boneState.orientation.InterpolateOrientations(poseInfoA.boneState.orientation, poseInfoB.boneState.orientation, alpha);
		poseInfo.boneState.length = poseInfoA.boneState.length + alpha * (poseInfoB.boneState.length - poseInfoA.boneState.length);
		this->poseInfoArray.push_back(poseInfo);
	}

	return true;
}

int KeyFrame::Pose(Skeleton* skeleton) const
{
	int poseCount = 0;

	for (const PoseInfo& poseInfo : this->poseInfoArray)
	{
		Bone* bone = skeleton->FindBone(poseInfo.boneName);
		if (bone)
		{
			bone->SetCurrentPoseState(poseInfo.boneState);
			poseCount++;
		}
	}

	return poseCount;
}

void KeyFrame::MakePose(const Skeleton* skeleton)
{
	this->Clear();

	std::vector<Bone*> boneArray;
	const_cast<Skeleton*>(skeleton)->GatherBones(boneArray);

	for (const Bone* bone : boneArray)
	{
		const Bone::Transforms* transform = bone->GetTransforms(BoneTransformType::CURRENT_POSE);

		PoseInfo poseInfo;
		poseInfo.boneName = bone->GetName();
		poseInfo.boneState = transform->boneState;
		this->poseInfoArray.push_back(poseInfo);
	}
}