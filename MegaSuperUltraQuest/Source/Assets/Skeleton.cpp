#include "Skeleton.h"
#include "Game.h"

//-------------------------------- Skeleton --------------------------------

Skeleton::Skeleton()
{
	this->rootBone = nullptr;
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
}

void Skeleton::DebugDraw()
{
	DebugLines* debugLines = Game::Get()->GetDebugLines();
	if (!debugLines)
		return;


}

//-------------------------------- Bone --------------------------------

Bone::Bone()
{
	this->parentBone = nullptr;
	this->objectToChildValid = false;
}

/*virtual*/ Bone::~Bone()
{
	this->DeleteAllChildBones();
}

void Bone::DeleteAllChildBones()
{
	for (Bone* bone : this->childBoneArray)
		delete bone;

	this->childBoneArray.clear();
}