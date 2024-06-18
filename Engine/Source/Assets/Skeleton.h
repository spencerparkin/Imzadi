#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Transform.h"
#include <unordered_map>

namespace Imzadi
{
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
	 * bone spaces.
	 */
	class IMZADI_API Skeleton : public Asset
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
		bool GatherBones(const Vector3& position, BoneTransformType boneTransformType, std::vector<Bone*>& boneArray);

		/**
		 * Update all internally-cached transforms that are a function of our single-source-of-truth transforms.
		 * This must be called after posing a skeleton and before using it to deform a mesh.
		 */
		void UpdateCachedTransforms(BoneTransformType transformType) const;

		/**
		 * Reset the current pose to the bind pose.
		 */
		void ResetCurrentPose();

		void DebugDraw(BoneTransformType transformType, const Transform& objectToWorld) const;

	private:
		Bone* rootBone;

		mutable BoneMap boneMap;
		mutable bool boneMapValid;
	};

	/**
	 * These are the nodes of the skeleton tree.  A bone is realized
	 * as a line-segment connecting the origin of a parent node to the
	 * origin of a child node.  A bone is also a space, and the skeleton
	 * tree is a hierarchy of spaces.  A child's transform is relative
	 * to its parent, so to get the space of a node you must concatinate
	 * transforms from the root of the tree to the node in question.
	 */
	class IMZADI_API Bone
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

		void SetWeightable(bool weightable) { this->canBeWeightedAgainst = weightable; }
		bool GetWeightable() const { return this->canBeWeightedAgainst; }

		void SetBindPoseChildToParent(const Transform& childToParent) { this->bindPose.childToParent = childToParent; }
		const Transform& GetBindPoseChildToParent() const { return this->bindPose.childToParent; }

		void SetCurrentPoseChildToParent(const Transform& childToParent) { this->currentPose.childToParent = childToParent; }
		const Transform& GetCurrentPoseChildToParent() { return this->currentPose.childToParent; }

		void UpdateCachedTransforms(BoneTransformType transformType);

		void PopulateBoneMap(BoneMap& boneMap) const;

		void DebugDraw(BoneTransformType transformType, const Transform& objectToWorld) const;

		bool Load(const rapidjson::Value& boneValue);
		bool Save(rapidjson::Value& boneValue, rapidjson::Document* doc) const;

		Imzadi::Vector3 CalcObjectSpaceCenter(BoneTransformType transformType) const;

		struct Transforms
		{
			Transform childToParent;
			Transform boneToObject;
			Transform objectToBone;
		};

		Transforms* GetTransforms(BoneTransformType transformType);
		const Transforms* GetTransforms(BoneTransformType transformType) const;

	private:

		std::string name;
		Bone* parentBone;
		std::vector<Bone*> childBoneArray;
		Transforms bindPose;
		Transforms currentPose;
		bool canBeWeightedAgainst;
	};
}