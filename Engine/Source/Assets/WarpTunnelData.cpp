#include "WarpTunnelData.h"
#include "Log.h"

using namespace Imzadi;

WarpTunnelData::WarpTunnelData()
{
}

/*virtual*/ WarpTunnelData::~WarpTunnelData()
{
}

/*virtual*/ bool WarpTunnelData::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.HasMember("mesh") || !jsonDoc["mesh"].IsString())
	{
		IMZADI_LOG_ERROR("No \"mesh\" field found or it's not a string.");
		return false;
	}

	this->meshFile = jsonDoc["mesh"].GetString();

	if (!jsonDoc.HasMember("collision") || !jsonDoc["collision"].IsString())
	{
		IMZADI_LOG_ERROR("No \"collision\" field found or it's not a string.");
		return false;
	}

	this->collisionFile = jsonDoc["collision"].GetString();

	if (!jsonDoc.HasMember("port_binds") || !jsonDoc["port_binds"].IsArray())
	{
		IMZADI_LOG_ERROR("No \"port_binds\" member found or it's not an array.");
		return false;
	}

	this->portBindArray.clear();
	const rapidjson::Value& portBindsArrayValue = jsonDoc["port_binds"];
	for (int i = 0; i < portBindsArrayValue.Size(); i++)
	{
		const rapidjson::Value& portBindValue = portBindsArrayValue[i];
		if (!portBindValue.IsObject())
		{
			IMZADI_LOG_ERROR("Expected each entry of the \"port_bind\" array member to be an object.");
			return false;
		}

		if (!portBindValue.HasMember("domestic_port") || !portBindValue["domestic_port"].IsString() ||
			!portBindValue.HasMember("foreign_port") || !portBindValue["foreign_port"].IsString() ||
			!portBindValue.HasMember("foreign_mesh") || !portBindValue["foreign_mesh"].IsString())
		{
			IMZADI_LOG_ERROR("Expected to find \"domestic_port\", \"foreign_port\" and \"foregn_mesh\", all as string, but did not.");
			return false;
		}

		PortBind portBind;
		portBind.domesticPort = portBindValue["domestic_port"].GetString();
		portBind.foreignPort = portBindValue["foreign_port"].GetString();
		portBind.foreignMesh = portBindValue["foreign_mesh"].GetString();
		this->portBindArray.push_back(portBind);
	}

	return true;
}

/*virtual*/ bool WarpTunnelData::Unload()
{
	this->portBindArray.clear();

	return true;
}

const WarpTunnelData::PortBind* WarpTunnelData::GetPortBind(int i) const
{
	return const_cast<WarpTunnelData*>(this)->GetPortBind(i);
}

WarpTunnelData::PortBind* WarpTunnelData::GetPortBind(int i)
{
	if (i < 0 || i >= (signed)this->portBindArray.size())
		return nullptr;

	return &this->portBindArray[i];
}