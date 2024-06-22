#include "JsonUtils.h"
#include "Log.h"

/*static*/ bool JsonUtils::WriteJsonFile(const rapidjson::Document& jsonDoc, const wxString& assetFile)
{
	std::filesystem::path assetPath((const char*)assetFile.c_str());
	if (std::filesystem::exists(assetPath))
	{
		std::filesystem::remove(assetPath);
		IMZADI_LOG_INFO("Deleted file: %s", (const char*)assetFile.c_str());
	}

	std::ofstream fileStream;
	fileStream.open((const char*)assetFile.c_str(), std::ios::out);
	if (!fileStream.is_open())
	{
		IMZADI_LOG_ERROR("Failed to open (for writing) the file: %s", (const char*)assetFile.c_str());
		return false;
	}

	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(stringBuffer);
	if (!jsonDoc.Accept(prettyWriter))
	{
		IMZADI_LOG_ERROR("Failed to generate JSON text from JSON data for file: %s", (const char*)assetFile.c_str());
		return false;
	}

	fileStream << stringBuffer.GetString();
	fileStream.close();
	IMZADI_LOG_INFO("Wrote file: %s", (const char*)assetFile.c_str());
	return true;
}

/*static*/ bool JsonUtils::ReadJsonFile(rapidjson::Document& jsonDoc, const wxString& assetFile)
{
	std::ifstream fileStream;
	fileStream.open((const char*)assetFile.c_str(), std::ios::in);
	if (!fileStream.is_open())
	{
		IMZADI_LOG_ERROR("Failed to open (for reading) the file: %s", (const char*)assetFile.c_str());
		return false;
	}

	rapidjson::IStreamWrapper streamWrapper(fileStream);
	jsonDoc.ParseStream(streamWrapper);
	if (jsonDoc.HasParseError())
	{
		IMZADI_LOG_ERROR("Failed to parse file: %s", (const char*)assetFile.c_str());
		rapidjson::ParseErrorCode errorCode = jsonDoc.GetParseError();
		IMZADI_LOG_ERROR("Parser error: %s", rapidjson::GetParseError_En(errorCode));
		return false;
	}

	fileStream.close();
	return true;
}