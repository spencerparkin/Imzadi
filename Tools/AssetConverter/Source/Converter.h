#pragma once

#include <wx/string.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/vector3.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Transform.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <unordered_set>

class Converter
{
public:
	Converter(const wxString& assetRootFolder);
	virtual ~Converter();

	bool Convert(const wxString& assetFile);

private:

	bool ProcessSceneGraph(const aiScene* scene, const aiNode* node, const Imzadi::Transform& parentNodeToWorld);
	bool ProcessMesh(const aiScene* scene, const aiNode* node, const aiMesh* mesh, const Imzadi::Transform& nodeToWorld);
	bool MakeTransform(Imzadi::Transform& transformOut, const aiMatrix4x4& matrixIn);
	bool MakeVector(Imzadi::Vector3& vectorOut, const aiVector3D& vectorIn);
	bool MakeTexCoords(Imzadi::Vector2& texCoordsOut, const aiVector3D& texCoordsIn);
	wxString MakeAssetFileReference(const wxString& assetFile);
	bool WriteJsonFile(const rapidjson::Document& jsonDoc, const wxString& assetFile);

	Assimp::Importer importer;
	wxString assetFolder;
	wxString assetRootFolder;
};