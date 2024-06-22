#pragma once

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include <wx/string.h>

class JsonUtils
{
public:
	static bool WriteJsonFile(const rapidjson::Document& jsonDoc, const wxString& assetFile);
	static bool ReadJsonFile(rapidjson::Document& jsonDoc, const wxString& assetFile);
};