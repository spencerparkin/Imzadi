#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Transform.h"
#include <unordered_map>

class Bone;
class KeyFrame;

typedef std::unordered_map<std::string, Bone*> BoneMap;

enum BoneTransformType
{
	BIND_POSE,
	CURRENT_POSE
};

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

	Bone* FindBone(const std::string& name);
	const Bone* FindBone(const std::string& name) const;
	void InvalidateBoneMap() const;

	bool GatherBones(std::vector<Bone*>& boneArray);

	/**
	 * Gather all bones of the skeleton into the given array, but order
	 * them closest to furthest from the given position.
	 * 
	 * @param[in] position Bones are sorted based on distance to this point from the bone centers.
	 * @param[in] boneTransformType Are we looking at the bind pose or the current pose?
	 * @param[out] boneArray The sorted list of bones is returned in this array.
	 * @return False is returned if there are no bones to return; true, otherwise.
	 */
	bool GatherBones(const Collision::Vector3& position, BoneTransformType boneTransformType, std::vector<Bone*>& boneArray);

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
	 * Create a key-frame from the current pose.
	 */
	KeyFrame* CreateKeyFrame() const;

	/**
	 * Update all internally-cached transforms that are a function of our single-source-of-truth transforms.
	 * This must be called after posing a skeleton and before using it to deform a mesh.
	 */
	void UpdateCachedTransforms(BoneTransformType transformType) const;

	/**
	 * Reset the current pose to the bind pose.
	 */
	void ResetCurrentPose();

	void DebugDraw(BoneTransformType transformType, const Collision::Transform& objectToWorld) const;

	void MakeBasicBiped();

private:
	Bone* rootBone;

	mutable BoneMap boneMap;
	mutable bool boneMapValid;
};

/**
 * These are the nodes of the skeleton tree.  A bone is realized
 * in its parent space as a vector (of some fixed length) seated
 * at origin and pointing down the +X-axis prior to being oriented.
 * The space of the bone (a child space of the parent) mirrors the
 * orientation of the bone, but with origin at the head of the vector.
 * The parent space of the root bone is always object space.
 * 
 * Things live in spaces and sub-spaces can be attached to things.
 * A bone is a fixed-length vector at origin with a sub-space attached
 * to the tip of the vector.  A bone lives in its parent space.  The
 * space of the bone is the space at its tip.  Child bones of a bone
 * live in the space of the bone.  Note that since the bone vector
 * has a sub-space on its tip, this gives the vector orientation beyond
 * that of a normal vector.  With a normal vector, rotating the vector
 * about an axis parallel to it does not change it.  This is not the
 * case, however, with our bone vectors, since the sub-space at its
 * tip is a property of the vector.
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

	void SetLength(double length) { this->length = length; }
	double GetLength() const { return this->length; }

	void SetBindPoseOrientation(const Collision::Matrix3x3& orientation) { this->bindPose.orientation = orientation; }
	const Collision::Matrix3x3& GetBindPoseOrientation() const { return this->bindPose.orientation; }

	void SetCurrentPoseOrientation(const Collision::Matrix3x3& orientation) { this->currentPose.orientation = orientation; }
	const Collision::Matrix3x3& GetCurrentPoseOrientation() const { return this->currentPose.orientation; }

	void UpdateCachedTransforms(BoneTransformType transformType);

	void PopulateBoneMap(BoneMap& boneMap) const;

	void DebugDraw(BoneTransformType transformType, const Collision::Transform& objectToWorld) const;

	bool Load(const rapidjson::Value& boneValue);
	bool Save(rapidjson::Value& boneValue, rapidjson::Document* doc) const;

	Collision::Vector3 CalcObjectSpaceCenter(BoneTransformType transformType) const;

private:

	struct Transforms
	{
		Collision::Matrix3x3 orientation;
		Collision::Transform boneToObject;
		Collision::Transform objectToBone;
	};

	Transforms* GetTransforms(BoneTransformType transformType);
	const Transforms* GetTransforms(BoneTransformType transformType) const;

	std::string name;
	Bone* parentBone;
	std::vector<Bone*> childBoneArray;
	double length;
	Transforms bindPose;
	Transforms currentPose;
};