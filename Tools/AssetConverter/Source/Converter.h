#pragma once

#include <wx/string.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/vector3.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Transform.h"
#include "Math/AnimTransform.h"
#include "Assets/Skeleton.h"
#include "Assets/SkinWeights.h"
#include "Assets/Animation.h"
#include "JsonUtils.h"
#include "TextureMaker.h"
#include <unordered_set>
#include <unordered_map>

class Converter
{
public:
	Converter();
	virtual ~Converter();
	
	enum Flag
	{
		CONVERT_MESHES		= 0x00000001,
		CONVERT_ANIMATIONS	= 0x00000002,
		MAKE_COLLISION		= 0x00000004
	};

	void SetFlags(uint32_t flags) { this->flags = flags; }
	uint32_t GetFlags() const { return this->flags; }

	bool Convert(const wxString& assetFile);

private:

	bool GenerateNodeToWorldMap(const aiNode* node);
	bool ProcessSceneGraph(const aiScene* scene, const aiNode* node);
	bool ProcessMesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh);
	bool GenerateSkeleton(Imzadi::Skeleton& skeleton, const aiMesh* mesh);
	bool GenerateSkeleton(Imzadi::Bone* bone, const aiNode* boneNode, const std::unordered_set<const aiNode*>& boneSet);
	bool GenerateSkinWeights(Imzadi::SkinWeights& skinWeights, const aiMesh* mesh);
	bool MakeTransform(Imzadi::Transform& transformOut, const aiMatrix4x4& matrixIn);
	bool MakeVector(Imzadi::Vector3& vectorOut, const aiVector3D& vectorIn);
	bool MakeTexCoords(Imzadi::Vector2& texCoordsOut, const aiVector3D& texCoordsIn);
	bool MakeQuat(Imzadi::Quaternion& quaternionOut, const aiQuaternion& quaternionIn);
	bool FindParentBones(const aiNode* boneNode, std::unordered_set<const aiNode*>& boneSet);
	bool GetNodeToWorldTransform(const aiNode* node, Imzadi::Transform& nodeToWorld);
	bool ProcessAnimation(const aiScene* scene, const aiAnimation* animation);
	bool FindNextKeyFrame(const aiAnimation* animation, double& currentTick, Imzadi::KeyFrame*& keyFrame);
	void GatherApplicableAnimations(rapidjson::Value& animationsArrayValue, const Imzadi::Skeleton* skeleton, rapidjson::Document* doc);
	bool IsAnimationApplicable(const wxString& animationFile, const Imzadi::Skeleton* skeleton);

	Assimp::Importer importer;
	wxString assetFolder;
	std::unordered_map<const aiNode*, Imzadi::Transform> nodeToWorldMap;
	TextureMaker textureMaker;
	uint32_t flags;
};