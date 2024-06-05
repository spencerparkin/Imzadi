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
	return false;
}

/*virtual*/ bool Skeleton::Unload()
{
	this->SetRootBone(nullptr);
	return true;
}

/*virtual*/ bool Skeleton::Save(rapidjson::Document& jsonDoc) const
{
	return false;
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
	pelvis->SetBindPoseTransform(transform);
	pelvis->SetName("Pelvis");

	// Chest.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 1.7, 0.0) * scale;
	Bone* chest = new Bone();
	chest->SetBindPoseTransform(transform);
	chest->SetName("Chest");
	pelvis->AddChildBone(chest);

	// Neck.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 0.5, 0.0) * scale;
	Bone* neck = new Bone();
	neck->SetBindPoseTransform(transform);
	neck->SetName("Neck");
	chest->AddChildBone(neck);

	// Head.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, 1.0, 0.0) * scale;
	Bone* head = new Bone();
	head->SetBindPoseTransform(transform);
	head->SetName("Head");
	neck->AddChildBone(head);

	// Left shoulder.
	transform.SetIdentity();
	transform.translation = Vector3(-0.5, 0.0, 0.0) * scale;
	Bone* leftShoulder = new Bone();
	leftShoulder->SetBindPoseTransform(transform);
	leftShoulder->SetName("LeftShoulder");
	chest->AddChildBone(leftShoulder);

	// Right shoulder.
	transform.SetIdentity();
	transform.translation = Vector3(0.5, 0.0, 0.0) * scale;
	Bone* rightShoulder = new Bone();
	rightShoulder->SetBindPoseTransform(transform);
	rightShoulder->SetName("RightShoulder");
	chest->AddChildBone(rightShoulder);

	// Left upper arm.
	transform.SetIdentity();
	transform.translation = Vector3(-1.2, 0.0, 0.0) * scale;
	Bone* leftUpperArm = new Bone();
	leftUpperArm->SetBindPoseTransform(transform);
	leftUpperArm->SetName("LeftUpperArm");
	leftShoulder->AddChildBone(leftUpperArm);

	// Right upper arm.
	transform.SetIdentity();
	transform.translation = Vector3(1.2, 0.0, 0.0) * scale;
	Bone* rightUpperArm = new Bone();
	rightUpperArm->SetBindPoseTransform(transform);
	rightUpperArm->SetName("RightUpperArm");
	rightShoulder->AddChildBone(rightUpperArm);

	// Left lower arm.
	transform.SetIdentity();
	transform.translation = Vector3(-1.2, 0.0, 0.0) * scale;
	Bone* leftLowerArm = new Bone();
	leftLowerArm->SetBindPoseTransform(transform);
	leftLowerArm->SetName("LeftLowerArm");
	leftUpperArm->AddChildBone(leftLowerArm);

	// Right lower arm.
	transform.SetIdentity();
	transform.translation = Vector3(1.2, 0.0, 0.0) * scale;
	Bone* rightLowerArm = new Bone();
	rightLowerArm->SetBindPoseTransform(transform);
	rightLowerArm->SetName("RightLowerArm");
	rightUpperArm->AddChildBone(rightLowerArm);

	// Left hip socket.
	transform.SetIdentity();
	transform.translation = Vector3(-0.5, 0.0, 0.0) * scale;
	Bone* leftHip = new Bone();
	leftHip->SetBindPoseTransform(transform);
	leftHip->SetName("LeftHip");
	pelvis->AddChildBone(leftHip);

	// Right hip socket.
	transform.SetIdentity();
	transform.translation = Vector3(0.5, 0.0, 0.0) * scale;
	Bone* rightHip = new Bone();
	rightHip->SetBindPoseTransform(transform);
	rightHip->SetName("RightHip");
	pelvis->AddChildBone(rightHip);

	// Left upper leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* leftUpperLeg = new Bone();
	leftUpperLeg->SetBindPoseTransform(transform);
	leftUpperLeg->SetName("LeftUpperLeg");
	leftHip->AddChildBone(leftUpperLeg);

	// Right upper leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* rightUpperLeg = new Bone();
	rightUpperLeg->SetBindPoseTransform(transform);
	rightUpperLeg->SetName("RightUpperLeg");
	rightHip->AddChildBone(rightUpperLeg);

	// Left lower leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* leftLowerLeg = new Bone();
	leftLowerLeg->SetBindPoseTransform(transform);
	leftLowerLeg->SetName("LeftLowerLeg");
	leftUpperLeg->AddChildBone(leftLowerLeg);

	// Right lower leg.
	transform.SetIdentity();
	transform.translation = Vector3(0.0, -1.25, 0.0) * scale;
	Bone* rightLowerLeg = new Bone();
	rightLowerLeg->SetBindPoseTransform(transform);
	rightLowerLeg->SetName("RightLowerLeg");
	rightUpperLeg->AddChildBone(rightLowerLeg);
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

void Bone::SetBindPoseTransform(const Collision::Transform& parentToChild)
{
	this->bindPoseParentToChild = parentToChild;
}

const Collision::Transform& Bone::GetBindPoseTransform() const
{
	return this->bindPoseParentToChild;
}

double Bone::GetBoneLength() const
{
	return this->bindPoseParentToChild.translation.Length();
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
	this->currentPoseParentToChild.translation = this->bindPoseParentToChild.translation;
	this->currentPoseParentToChild.matrix = this->currentPoseOrientation * this->bindPoseParentToChild.matrix;

	Transform bindPoseObjectToParent;
	if (this->parentBone)
		bindPoseObjectToParent = this->parentBone->bindPoseObjectToChild;
	else
		bindPoseObjectToParent.SetIdentity();

	this->bindPoseObjectToChild = this->bindPoseParentToChild * bindPoseObjectToParent;

	Transform currentPoseObjectToParent;
	if (this->parentBone)
		currentPoseObjectToParent = this->parentBone->currentPoseObjectToChild;
	else
		currentPoseObjectToChild.SetIdentity();

	this->currentPoseObjectToChild = this->currentPoseParentToChild * currentPoseObjectToParent;

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

	origin = this->currentPoseObjectToChild.TransformPoint(origin);
	xAxis = this->currentPoseObjectToChild.TransformNormal(xAxis);
	yAxis = this->currentPoseObjectToChild.TransformNormal(yAxis);
	zAxis = this->currentPoseObjectToChild.TransformNormal(zAxis);

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
	line.segment.point[1] = this->parentBone ? this->parentBone->currentPoseObjectToChild.translation : Vector3(0.0, 0.0, 0.0);
	line.segment = objectToWorld.TransformLineSegment(line.segment);
	debugLines->AddLine(line);

	for (const Bone* childBone : this->childBoneArray)
		childBone->DebugDraw(objectToWorld);
}