#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Transform.h"
#include <unordered_map>

class Bone;
class KeyFrame;

typedef std::unordered_map<std::string, Bone*> BoneMap;

/**
 * A skeleton is a hierarchy of bones (or spaces).  By itself, that's all
 * it is, but when combined with a skinned render mesh, manipulating the bones
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

	const Bone* FindBone(const std::string& name) const;
	void InvalidateBoneMap() const;

	/**
	 * Gather all bones of the skeleton into the given array, but order
	 * them closest to furthest from the given position.
	 */
	bool GatherBones(const Collision::Vector3& position, std::vector<const Bone*>& boneArray) const;

	/**
	 * Orient the bones of this skeleton using the two given key-frames.  Note that
	 * these need not be taken from the same animation.  They can be from different
	 * animations as a means of blending between animations.
	 * 
	 * You'll notice that this is a constant function.  This is because posing a skeleton
	 * does not change the defining characteristics of what a skeleton is.
	 * 
	 * @param[in] keyFrameA This can be thought of as the pose of the skeleton at interpolation value zero.
	 * @param[in] keyFrameB this can be thought of as the pose of the skeleton at interpolation value one.
	 * @param[in] alpha This is the interpolation value, typically ranging from zero to one.
	 * @return False is returned if not all bones could be oriented by the given key-frame pair; true, otherwise.
	 */
	bool Pose(const KeyFrame* keyFrameA, const KeyFrame* keyFrameB, double alpha) const;

	/**
	 * Update all internally-cached transforms that are a function of our single-source-of-truth transforms.
	 * This must be called after posing a skeleton and before using it to deform a mesh.
	 */
	bool UpdateCachedTransforms() const;

	void DebugDraw(const Collision::Transform& objectToWorld) const;

	void MakeBasicBiped(const Collision::Vector3& scale);

private:
	Bone* rootBone;

	mutable BoneMap boneMap;
	mutable bool boneMapValid;
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

	void SetParentBone(Bone* bone) { this->parentBone = bone; }
	Bone* GetParentBone() { return this->parentBone; }

	void AddChildBone(Bone* bone);
	void DeleteAllChildBones();
	Bone* GetChildBone(int i) { return this->childBoneArray[i]; }
	const Bone* GetChildBone(int i) const { return this->childBoneArray[i]; }
	size_t GetNumChildBones() const { return this->childBoneArray.size(); }

	void SetBindPoseTransform(const Collision::Transform& parentToChild);
	const Collision::Transform& GetBindPoseTransform() const;

	const Collision::Transform& GetBindPoseObjectToChild() const { return this->bindPoseObjectToChild; }
	const Collision::Transform& GetCurrentPoseObjectToChild() const { return this->currentPoseObjectToChild; }

	double GetBoneLength() const;

	void SetBoneOrientation(const Collision::Matrix3x3& boneOrientation) const;
	void SetBoneOrientation(const Collision::Quaternion& boneOrientation) const;

	bool UpdateCachedTransforms() const;

	void PopulateBoneMap(BoneMap& boneMap) const;

	void DebugDraw(const Collision::Transform& objectToWorld) const;

	bool Load(const rapidjson::Value& boneValue);
	bool Save(rapidjson::Value& boneValue, rapidjson::Document* doc) const;

private:
	std::string name;
	Bone* parentBone;
	std::vector<Bone*> childBoneArray;
	Collision::Transform bindPoseParentToChild;
	mutable Collision::Matrix3x3 currentPoseOrientation;
	mutable Collision::Transform currentPoseParentToChild;
	mutable Collision::Transform bindPoseObjectToChild;
	mutable Collision::Transform currentPoseObjectToChild;
};