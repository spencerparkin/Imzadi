#pragma once

#include <wx/string.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/vector3.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

class Converter
{
public:
	Converter();
	virtual ~Converter();

	bool Convert(const wxString& assetFile, wxString& error);

private:

	bool ConvertMesh(const aiMesh* mesh, const aiScene* scene, const wxString& assetFolder, wxString& error);
	Imzadi::Vector3 ConvertVector(const aiVector3D& vector);
	Imzadi::Vector2 ConvertTexCoords(const aiVector3D& texCoords);
	bool WriteJsonFile(const rapidjson::Document& jsonDoc, const wxString& assetFile, wxString& error);
	wxString MakeAssetFileReference(const wxString& assetFile);

	Assimp::Importer importer;
};