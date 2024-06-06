#include "Skeleton.h"
#include "Game.h"

using namespace Collision;

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
	if (!jsonDoc.HasMember("root_bone") || !jsonDoc["root_bone"].IsObject())
		return false;

	const rapidjson::Value& rootBoneValue = jsonDoc["root_bone"];
	this->SetRootBone(new Bone());
	if (!this->rootBone->Load(rootBoneValue))
		return false;
	
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
		return false;

	jsonDoc.SetObject();

	rapidjson::Value rootBoneValue;
	if (!this->rootBone->Save(rootBoneValue, &jsonDoc))
		return false;

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

const Bone* Skeleton::FindBone(const std::string& name) const
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

void Skeleton::DebugDraw(const Collision::Transform& objectToWorld) const
{
	if (!this->rootBone)
		return;

	if (!this->UpdateCachedTransforms())
		return;

	this->rootBone->DebugDraw(objectToWorld);
}

bool Skeleton::Pose(const KeyFrame* keyFrameA, const KeyFrame* keyFrameB, double alpha) const
{
	return false;
}

bool Skeleton::UpdateCachedTransforms() const
{
	if (!this->rootBone)
		return false;

	return this->rootBone->UpdateCachedTransforms();
}

void Skeleton::MakeBasicBiped(const Collision::Vector3& scale)
{
	this->SetRootBone(new Bone());

	Transform transform;

	// Pelvis.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 2.7, 0.0) * scale;
	Bone* pelvis = this->GetRootBone();
	pelvis->SetBindPoseChildToParent(transform);
	pelvis->SetName("Pelvis");

	// Chest.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 1.7, 0.0) * scale;
	Bone* chest = new Bone();
	chest->SetBindPoseChildToParent(transform);
	chest->SetName("Chest");
	pelvis->AddChildBone(chest);

	// Neck.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 0.5, 0.0) * scale;
	Bone* neck = new Bone();
	neck->SetBindPoseChildToParent(transform);
	neck->SetName("Neck");
	chest->AddChildBone(neck);

	// Head.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 1.0, 0.0) * scale;
	Bone* head = new Bone();
	head->SetBindPoseChildToParent(transform);
	head->SetName("Head");
	neck->AddChildBone(head);

	// Left shoulder.
	transform.SetIdentity();
	transform.translation = Vector3(-0.8, 0.0, 0.0) * scale;
	Bone* leftShoulder = new Bone();
	leftShoulder->SetBindPoseChildToParent(transform);
	leftShoulder->SetName("LeftShoulder");
	chest->AddChildBone(leftShoulder);

	// Right shoulder.
	transform.SetIdentity();
	transform.translation = Vector3(0.8, 0.0, 0.0) * scale;
	Bone* rightShoulder = new Bone();
	rightShoulder->SetBindPoseChildToParent(transform);
	rightShoulder->SetName("RightShoulder");
	chest->AddChildBone(rightShoulder);

	// Left upper arm.
	transform.SetIdentity();
	transform.translation = Vector3(-1.2, 0.0, 0.0) * scale;
	Bone* leftUpperArm = new Bone();
	leftUpperArm->SetBindPoseChildToParent(transform);
	leftUpperArm->SetName("LeftUpperArm");
	leftShoulder->AddChildBone(leftUpperArm);

	// Right upper arm.
	transform.SetIdentity();
	transform.translation = Vector3(1.2, 0.0, 0.0) * scale;
	Bone* rightUpperArm = new Bone();
	rightUpperArm->SetBindPoseChildToParent(transform);
	rightUpperArm->SetName("RightUpperArm");
	rightShoulder->AddChildBone(rightUpperArm);

	// Left lower arm.
	transform.SetIdentity();
	transform.translation = Vector3(-1.2, 0.0, 0.0) * scale;
	Bone* leftLowerArm = new Bone();
	leftLowerArm->SetBindPoseChildToParent(transform);
	leftLowerArm->SetName("LeftLowerArm");
	leftUpperArm->AddChildBone(leftLowerArm);

	// Right lower arm.
	transform.SetIdentity();
	transform.translation = Vector3(1.2, 0.0, 0.0) * scale;
	Bone* rightLowerArm = new Bone();
	rightLowerArm->SetBindPoseChildToParent(transform);
	rightLowerArm->SetName("RightLowerArm");
	rightUpperArm->AddChildBone(rightLowerArm);

	// Left hip socket.
	transform.SetIdentity();
	transform.translation = Vector3(-0.5, 0.0, 0.0) * scale;
	Bone* leftHip = new Bone();
	leftHip->SetBindPoseChildToParent(transform);
	leftHip->SetName("LeftHip");
	pelvis->AddChildBone(leftHip);

	// Right hip socket.
	transform.SetIdentity();
	transform.translation = Vector3(0.5, 0.0, 0.0) * scale;
	Bone* rightHip = new Bone();
	rightHip->SetBindPoseChildToParent(transform);
	rightHip->SetName("RightHip");
	pelvis->AddChildBone(rightHip);

	// Left upper leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* leftUpperLeg = new Bone();
	leftUpperLeg->SetBindPoseChildToParent(transform);
	leftUpperLeg->SetName("LeftUpperLeg");
	leftHip->AddChildBone(leftUpperLeg);

	// Right upper leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* rightUpperLeg = new Bone();
	rightUpperLeg->SetBindPoseChildToParent(transform);
	rightUpperLeg->SetName("RightUpperLeg");
	rightHip->AddChildBone(rightUpperLeg);

	// Left lower leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* leftLowerLeg = new Bone();
	leftLowerLeg->SetBindPoseChildToParent(transform);
	leftLowerLeg->SetName("LeftLowerLeg");
	leftUpperLeg->AddChildBone(leftLowerLeg);

	// Right lower leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* rightLowerLeg = new Bone();
	rightLowerLeg->SetBindPoseChildToParent(transform);
	rightLowerLeg->SetName("RightLowerLeg");
	rightUpperLeg->AddChildBone(rightLowerLeg);
}

bool Skeleton::GatherBones(const Collision::Vector3& position, std::vector<const Bone*>& boneArray) const
{
	if (!this->rootBone)
		return false;

	boneArray.clear();
	std::list<const Bone*> boneQueue;
	boneQueue.push_back(this->rootBone);
	while (boneQueue.size() > 0)
	{
		const Bone* bone = boneQueue.front();
		boneQueue.pop_front();

		for (int i = 0; i < bone->GetNumChildBones(); i++)
			boneQueue.push_back(bone->GetChildBone(i));

		boneArray.push_back(bone);
	}

	std::sort(boneArray.begin(), boneArray.end(), [&position](const Bone* boneA, const Bone* boneB) -> bool {
		const Vector3& boneOriginA = boneA->GetBindPoseChildToObject().translation;
		const Vector3& boneOriginB = boneB->GetBindPoseChildToObject().translation;
		double distanceA = (position - boneOriginA).Length();
		double distanceB = (position - boneOriginB).Length();
		return distanceA < distanceB;
	});

	return true;
}

//-------------------------------- Bone --------------------------------

Bone::Bone()
{
	this->parentBone = nullptr;
	this->currentPoseOrientation.SetIdentity();
}

/*virtual*/ Bone::~Bone()
{
	this->DeleteAllChildBones();
}

bool Bone::Load(const rapidjson::Value& boneValue)
{
	if (!boneValue.HasMember("name") || !boneValue["name"].IsString())
		return false;

	this->name = boneValue["name"].GetString();

	if (!boneValue.HasMember("bind_pose_child_to_parent"))
		return false;

	if (!Asset::LoadTransform(boneValue["bind_pose_child_to_parent"], this->bindPoseChildToParent))
		return false;

	if (!boneValue.HasMember("child_bone_array") || !boneValue["child_bone_array"].IsArray())
		return false;

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
	Asset::SaveTransform(bindPoseChildToParentValue, this->bindPoseChildToParent, doc);

	boneValue.SetObject();
	boneValue.AddMember("name", rapidjson::Value().SetString(this->name.c_str(), doc->GetAllocator()), doc->GetAllocator());
	boneValue.AddMember("bind_pose_child_to_parent", bindPoseChildToParentValue, doc->GetAllocator());

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

double Bone::GetBoneLength() const
{
	return this->bindPoseChildToParent.translation.Length();
}

void Bone::SetBoneOrientation(const Collision::Matrix3x3& boneOrientation) const
{
	this->currentPoseOrientation = boneOrientation;
}

void Bone::SetBoneOrientation(const Collision::Quaternion& boneOrientation) const
{
	this->currentPoseOrientation.SetFromQuat(boneOrientation);
}

bool Bone::UpdateCachedTransforms() const
{
	this->currentPoseChildToParent.translation = this->bindPoseChildToParent.translation;
	this->currentPoseChildToParent.matrix = this->currentPoseOrientation * this->bindPoseChildToParent.matrix;

	Transform bindPoseParentToObject;
	if (this->parentBone)
		bindPoseParentToObject = this->parentBone->bindPoseChildToObject;
	else
		bindPoseParentToObject.SetIdentity();

	this->bindPoseChildToObject = bindPoseParentToObject * this->bindPoseChildToParent;

	Transform currentPoseParentToObject;
	if (this->parentBone)
		currentPoseParentToObject = this->parentBone->currentPoseChildToObject;
	else
		currentPoseParentToObject.SetIdentity();

	this->currentPoseChildToObject = currentPoseParentToObject * this->currentPoseChildToParent;

	for (const Bone* childBone : this->childBoneArray)
		if (!childBone->UpdateCachedTransforms())
			return false;

	return true;
}

void Bone::PopulateBoneMap(BoneMap& boneMap) const
{
	boneMap.insert(std::pair<std::string, Bone*>(this->name, const_cast<Bone*>(this)));

	for (Bone* childBone : this->childBoneArray)
		childBone->PopulateBoneMap(boneMap);
}

void Bone::DebugDraw(const Collision::Transform& objectToWorld) const
{
	DebugLines* debugLines = Game::Get()->GetDebugLines();
	if (!debugLines)
		return;

	Vector3 origin(0.0, 0.0, 0.0);
	Vector3 xAxis(0.1, 0.0, 0.0);
	Vector3 yAxis(0.0, 0.1, 0.0);
	Vector3 zAxis(0.0, 0.0, 0.1);

	origin = this->currentPoseChildToObject.TransformPoint(origin);
	xAxis = this->currentPoseChildToObject.TransformNormal(xAxis);
	yAxis = this->currentPoseChildToObject.TransformNormal(yAxis);
	zAxis = this->currentPoseChildToObject.TransformNormal(zAxis);

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

	line.color.SetComponents(0.5, 0.5, 0.5);
	line.segment.point[0] = origin;
	line.segment.point[1] = this->parentBone ? this->parentBone->currentPoseChildToObject.translation : Vector3(0.0, 0.0, 0.0);
	line.segment = objectToWorld.TransformLineSegment(line.segment);
	debugLines->AddLine(line);

	for (const Bone* childBone : this->childBoneArray)
		childBone->DebugDraw(objectToWorld);
}