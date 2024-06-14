#pragma once

#include <wx/string.h>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/config.h"

class Converter
{
public:
	Converter();
	virtual ~Converter();

	bool Convert(const wxString& assetFile, wxString& error);

private:
	Assimp::Importer importer;
};