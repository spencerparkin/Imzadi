#include "Skeleton.h"
#include "Game.h"
#include "Error.h"

using namespace Imzadi;

//-------------------------------- Skeleton --------------------------------

Skeleton::Skeleton()
{
	this->rootBone = nullptr;
	this->boneMapValid = false;
}

/*virtual*/ Skeleton::~Skeleton()
{
	this->SetRootBone(nullptr);
}

/*virtual*/ bool Skeleton::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_ERROR("Expected JSON doc to be an object.");
		return false;
	}

	if (!jsonDoc.HasMember("root_bone") || !jsonDoc["root_bone"].IsObject())
	{
		IMZADI_ERROR("No \"root_bone\" member found or it's not an object.");
		return false;
	}

	const rapidjson::Value& rootBoneValue = jsonDoc["root_bone"];
	this->SetRootBone(new Bone());
	if (!this->rootBone->Load(rootBoneValue))
	{
		IMZADI_ERROR("Failed to load bone tree.");
		return false;
	}
	
	this->UpdateCachedTransforms(BoneTransformType::BIND_POSE);
	this->ResetCurrentPose();

	return true;
}

/*virtual*/ bool Skeleton::Unload()
{
	this->SetRootBone(nullptr);
	return true;
}

/*virtual*/ bool Skeleton::Save(rapidjson::Document& jsonDoc) const
{
	if (!this->rootBone)
	{
		IMZADI_ERROR("No root bone!");
		return false;
	}

	jsonDoc.SetObject();

	rapidjson::Value rootBoneValue;
	if (!this->rootBone->Save(rootBoneValue, &jsonDoc))
	{
		IMZADI_ERROR("Failed to save bone tree.");
		return false;
	}

	jsonDoc.AddMember("root_bone", rootBoneValue, jsonDoc.GetAllocator());

	return true;
}

void Skeleton::SetRootBone(Bone* bone)
{
	delete this->rootBone;
	this->rootBone = bone;
	this->boneMapValid = false;
}

void Skeleton::InvalidateBoneMap() const
{
	this->boneMapValid = false;
}

Bone* Skeleton::FindBone(const std::string& name)
{
	if (!this->rootBone)
		return nullptr;

	if (!this->boneMapValid)
	{
		this->boneMap.clear();
		this->rootBone->PopulateBoneMap(this->boneMap);
		this->boneMapValid = true;
	}

	BoneMap::iterator iter = this->boneMap.find(name);
	if (iter == this->boneMap.end())
		return nullptr;

	return iter->second;
}

const Bone* Skeleton::FindBone(const std::string& name) const
{
	return const_cast<Skeleton*>(this)->FindBone(name);
}

void Skeleton::DebugDraw(BoneTransformType transformType, const Transform& objectToWorld) const
{
	if (this->rootBone)
	{
		this->UpdateCachedTransforms(transformType);
		this->rootBone->DebugDraw(transformType, objectToWorld);
	}
}

void Skeleton::UpdateCachedTransforms(BoneTransformType transformType) const
{
	if (this->rootBone)
		const_cast<Bone*>(this->rootBone)->UpdateCachedTransforms(transformType);
}

void Skeleton::ResetCurrentPose()
{
	std::vector<Bone*> boneArray;
	if (this->GatherBones(boneArray))
		for (Bone* bone : boneArray)
			bone->SetCurrentPoseChildToParent(bone->GetBindPoseChildToParent());
}

bool Skeleton::GatherBones(std::vector<Bone*>& boneArray)
{
	if (!this->rootBone)
		return false;

	boneArray.clear();
	std::list<Bone*> boneQueue;
	boneQueue.push_back(this->rootBone);
	while (boneQueue.size() > 0)
	{
		Bone* bone = boneQueue.front();
		boneQueue.pop_front();

		for (int i = 0; i < bone->GetNumChildBones(); i++)
			boneQueue.push_back(bone->GetChildBone(i));

		boneArray.push_back(bone);
	}

	return true;
}

bool Skeleton::GatherBones(const Vector3& position, BoneTransformType boneTransformType, std::vector<Bone*>& boneArray)
{
	if (!this->GatherBones(boneArray))
		return false;

	std::sort(boneArray.begin(), boneArray.end(), [&position, boneTransformType](const Bone* boneA, const Bone* boneB) -> bool {
		const Vector3& centerA = boneA->CalcObjectSpaceCenter(boneTransformType);
		const Vector3& centerB = boneB->CalcObjectSpaceCenter(boneTransformType);
		double distanceA = (position - centerA).Length();
		double distanceB = (position - centerB).Length();
		return distanceA < distanceB;
	});

	return true;
}

//-------------------------------- Bone --------------------------------

Bone::Bone()
{
	this->canBeWeightedAgainst = true;
	this->parentBone = nullptr;
	this->bindPose.childToParent.SetIdentity();
	this->currentPose.childToParent.SetIdentity();
}

/*virtual*/ Bone::~Bone()
{
	this->DeleteAllChildBones();
}

bool Bone::Load(const rapidjson::Value& boneValue)
{
	if (!boneValue.HasMember("name") || !boneValue["name"].IsString())
	{
		IMZADI_ERROR("No \"name\" member found or it's not a string.");
		return false;
	}

	this->name = boneValue["name"].GetString();

	if (!boneValue.HasMember("bind_pose_child_to_parent"))
	{
		IMZADI_ERROR("No \"bind_pose_child_to_parent\" member found.");
		return false;
	}

	if (!Asset::LoadTransform(boneValue["bind_pose_child_to_parent"], this->bindPose.childToParent))
	{
		IMZADI_ERROR("Failed to load transform from member \"bind_pose_child_to_parent\".");
		return false;
	}

	if (!boneValue.HasMember("weightable") || !boneValue["weightable"].IsBool())
	{
		IMZADI_ERROR("No \"weightable\" member found or it's not a bool.");
		return false;
	}

	this->canBeWeightedAgainst = boneValue["weightable"].GetBool();

	if (!boneValue.HasMember("child_bone_array") || !boneValue["child_bone_array"].IsArray())
	{
		IMZADI_ERROR("No \"child_bone_array\" member found or it's not an array.");
		return false;
	}

	const rapidjson::Value& childBoneArrayValue = boneValue["child_bone_array"];
	this->DeleteAllChildBones();
	for (int i = 0; i < childBoneArrayValue.Size(); i++)
	{
		const rapidjson::Value& childBoneValue = childBoneArrayValue[i];
		Bone* childBone = new Bone();
		childBone->SetParentBone(this);
		this->childBoneArray.push_back(childBone);
		if (!childBone->Load(childBoneValue))
			return false;
	}

	return true;
}

bool Bone::Save(rapidjson::Value& boneValue, rapidjson::Document* doc) const
{
	rapidjson::Value bindPoseChildToParentValue;
	Asset::SaveTransform(bindPoseChildToParentValue, this->bindPose.childToParent, doc);

	boneValue.SetObject();
	boneValue.AddMember("name", rapidjson::Value().SetString(this->name.c_str(), doc->GetAllocator()), doc->GetAllocator());
	boneValue.AddMember("bind_pose_child_to_parent", bindPoseChildToParentValue, doc->GetAllocator());
	boneValue.AddMember("weightable", rapidjson::Value().SetBool(this->canBeWeightedAgainst), doc->GetAllocator());

	rapidjson::Value childBoneArrayValue;
	childBoneArrayValue.SetArray();

	for (const Bone* bone : this->childBoneArray)
	{
		rapidjson::Value childBoneValue;
		if (!bone->Save(childBoneValue, doc))
			return false;

		childBoneArrayValue.PushBack(childBoneValue, doc->GetAllocator());
	}

	boneValue.AddMember("child_bone_array", childBoneArrayValue, doc->GetAllocator());

	return true;
}

void Bone::AddChildBone(Bone* bone)
{
	this->childBoneArray.push_back(bone);
	bone->SetParentBone(this);
}

void Bone::DeleteAllChildBones()
{
	for (Bone* bone : this->childBoneArray)
		delete bone;

	this->childBoneArray.clear();
}

const Bone::Transforms* Bone::GetTransforms(BoneTransformType transformType) const
{
	return const_cast<Bone*>(this)->GetTransforms(transformType);
}

Bone::Transforms* Bone::GetTransforms(BoneTransformType transformType)
{
	switch (transformType)
	{
	case BoneTransformType::BIND_POSE:
		return &this->bindPose;
	case BoneTransformType::CURRENT_POSE:
		return &this->currentPose;
	}

	return nullptr;
}

void Bone::UpdateCachedTransforms(BoneTransformType transformType)
{
	const Transforms* parentTransforms = this->parentBone ? this->parentBone->GetTransforms(transformType) : nullptr;
	Transforms* transforms = this->GetTransforms(transformType);

	if (!parentTransforms)
		transforms->boneToObject = transforms->childToParent;
	else
		transforms->boneToObject = parentTransforms->boneToObject * transforms->childToParent;
	
	bool inverted = transforms->objectToBone.Invert(transforms->boneToObject);
	assert(inverted);

	for (Bone* childBone : this->childBoneArray)
		childBone->UpdateCachedTransforms(transformType);
}

Vector3 Bone::CalcObjectSpaceCenter(BoneTransformType transformType) const
{
	const Transforms* parentTransforms = this->parentBone ? this->parentBone->GetTransforms(transformType) : nullptr;
	const Transforms* transforms = this->GetTransforms(transformType);

	if(parentTransforms)
		return parentTransforms->boneToObject.TransformPoint(transforms->childToParent.translation / 2.0);

	return transforms->childToParent.translation / 2.0;
}

void Bone::PopulateBoneMap(BoneMap& boneMap) const
{
	boneMap.insert(std::pair<std::string, Bone*>(this->name, const_cast<Bone*>(this)));

	for (Bone* childBone : this->childBoneArray)
		childBone->PopulateBoneMap(boneMap);
}

void Bone::DebugDraw(BoneTransformType transformType, const Transform& objectToWorld) const
{
	DebugLines* debugLines = Game::Get()->GetDebugLines();
	if (!debugLines)
		return;

	const Transforms* transforms = this->GetTransforms(transformType);

	Vector3 origin = transforms->boneToObject.TransformPoint(Vector3(0.0, 0.0, 0.0));
	Vector3 xAxis = transforms->boneToObject.TransformNormal(Vector3(0.1, 0.0, 0.0));
	Vector3 yAxis = transforms->boneToObject.TransformNormal(Vector3(0.0, 0.1, 0.0));
	Vector3 zAxis = transforms->boneToObject.TransformNormal(Vector3(0.0, 0.0, 0.1));

	DebugLines::Line line;

	line.color.SetComponents(1.0, 0.0, 0.0);
	line.segment.point[0] = origin;
	line.segment.point[1] = origin + xAxis;
	line.segment = objectToWorld.TransformLineSegment(line.segment);
	debugLines->AddLine(line);

	line.color.SetComponents(0.0, 1.0, 0.0);
	line.segment.point[0] = origin;
	line.segment.point[1] = origin + yAxis;
	line.segment = objectToWorld.TransformLineSegment(line.segment);
	debugLines->AddLine(line);

	line.color.SetComponents(0.0, 0.0, 1.0);
	line.segment.point[0] = origin;
	line.segment.point[1] = origin + zAxis;
	line.segment = objectToWorld.TransformLineSegment(line.segment);
	debugLines->AddLine(line);

	const Transforms* parentTransforms = this->parentBone ? this->parentBone->GetTransforms(transformType) : nullptr;

	line.color.SetComponents(0.5, 0.5, 0.5);
	line.segment.point[0] = origin;
	line.segment.point[1] = parentTransforms ? parentTransforms->boneToObject.translation : Vector3(0.0, 0.0, 0.0);
	line.segment = objectToWorld.TransformLineSegment(line.segment);
	debugLines->AddLine(line);

	for (const Bone* childBone : this->childBoneArray)
		childBone->DebugDraw(transformType, objectToWorld);
}