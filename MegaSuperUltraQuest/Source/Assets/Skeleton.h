#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Transform.h"

class Bone;

/**
 * A skeleton is a hierarchy of bones (or space).  By itself, that's all
 * it is, but when bound to a skinned render mesh, manipulating the bones
 * of the skeleton will in turn manipulate (or deform) the vertices of
 * the mesh.  Each vertex in a skinned mesh can be weighted to one or
 * more bones.  Loosely speaking, the location of a vertex is calculated
 * as a weighted linear combination of that vertex in each of the associated
 * bone spaces.  At least, that's what I think it's supposed to be.
 */
class Skeleton : public Asset
{
public:
	Skeleton();
	virtual ~Skeleton();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
	virtual bool Save(rapidjson::Document& jsonDoc) const override;

	void SetRootBone(Bone* bone);
	Bone* GetRootBone() { return this->rootBone; }
	const Bone* GetRootBone() const { return this->rootBone; }

	void DebugDraw();

private:
	Bone* rootBone;
};

/**
 * These are the nodes of the skeleton tree.
 */
class Bone
{
public:
	Bone();
	virtual ~Bone();

	void SetName(const std::string& name) { this->name = name; }
	const std::string& GetName() const { return this->name; }

	void SetParentBone(Bone* bone) { this->parentBone = parentBone; }
	Bone* GetParentBone() { return this->parentBone; }

	void AddChildBone(Bone* bone) { this->childBoneArray.push_back(bone); }
	void DeleteAllChildBones();

private:
	std::string name;
	Bone* parentBone;
	std::vector<Bone*> childBoneArray;
	Collision::Transform parentToChild;
	mutable Collision::Transform objectToChild;
	mutable bool objectToChildValid;
};