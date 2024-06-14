#include "Converter.h"
#include <wx/filename.h>

Converter::Converter()
{
}

/*virtual*/ Converter::~Converter()
{
}

bool Converter::Convert(const wxString& assetFile, wxString& error)
{
	wxFileName fileName(assetFile);
	wxString assetFolder = fileName.GetPath();

	this->importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0);

	// TODO: Can we alter the flags here to retain the animation information and then use it in the conversion process?
	//       The rig information is also not retained.  Even when I retain it, however, I can't find any bone weights.
	const aiScene* scene = importer.ReadFile(assetFile.c_str(), aiProcess_PreTransformVertices | aiProcess_Triangulate | aiProcess_GlobalScale);
	if (!scene)
	{
		error = wxString::Format("%s:\nImport error: %s\n", assetFile.c_str(), importer.GetErrorString());
		return false;
	}


	
	return true;
}