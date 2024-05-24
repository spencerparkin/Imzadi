#include "Buffer.h"
#include "Game.h"
#include <stdint.h>

Buffer::Buffer()
{
	this->buffer = nullptr;
	this->strideBytes = 0;
	this->numElements = 0;
	this->componentFormat = DXGI_FORMAT_UNKNOWN;
}

/*virtual*/ Buffer::~Buffer()
{
}

/*virtual*/ bool Buffer::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("type") || !jsonDoc["type"].IsString())
		return false;

	UINT32 componentTypeSize = 0;
	std::string componentType = jsonDoc["type"].GetString();
	if (componentType == "float")
	{
		componentTypeSize = sizeof(float);
		this->componentFormat = DXGI_FORMAT_R32_FLOAT;
	}
	else if (componentType == "int")
	{
		componentTypeSize = sizeof(int);
		this->componentFormat = DXGI_FORMAT_R32_SINT;
	}
	else if (componentType == "short")
	{
		componentTypeSize = sizeof(short);
		this->componentFormat = DXGI_FORMAT_R16_SINT;
	}
	else if (componentType == "uint")
	{
		componentTypeSize = sizeof(unsigned int);
		this->componentFormat = DXGI_FORMAT_R32_UINT;
	}
	else if (componentType == "ushort")
	{
		componentTypeSize = sizeof(unsigned short);
		this->componentFormat = DXGI_FORMAT_R16_UINT;
	}
	else
		return false;

	if (!jsonDoc.HasMember("stride") || !jsonDoc["stride"].IsInt())
		return false;

	UINT strideComponents = jsonDoc["stride"].GetInt();
	this->strideBytes = strideComponents * componentTypeSize;

	if (!jsonDoc.HasMember("bind") || !jsonDoc["bind"].IsString())
		return false;

	std::string bind = jsonDoc["bind"].GetString();

	if (!jsonDoc.HasMember("buffer"))
		return false;

	const rapidjson::Value& bufferValue = jsonDoc["buffer"];
	if (!bufferValue.IsArray())
		return false;

	if (bufferValue.Size() == 0 || bufferValue.Size() % strideComponents != 0)
		return false;

	UINT numComponents = bufferValue.Size();
	this->numElements = numComponents / strideComponents;
	
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = numComponents * componentTypeSize;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = 0;

	if (bind == "vertex")
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	else if (bind == "index")
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	else
		return false;

	std::unique_ptr<uint8_t[]> componentBuffer(new uint8_t[bufferDesc.ByteWidth]);
	int j = 0;

	for (int i = 0; i < bufferValue.Size(); i++)
	{
		const rapidjson::Value& bufferComponentValue = bufferValue[i];
		
		// I smell a template function here.
		if (componentType == "float")
		{
			if (!bufferComponentValue.IsFloat())
				return false;

			float component = bufferComponentValue.GetFloat();
			::memcpy(&componentBuffer[j], &component, componentTypeSize);
		}
		else if (componentType == "int")
		{
			if (!bufferComponentValue.IsInt())
				return false;

			int component = bufferComponentValue.GetInt();
			::memcpy(&componentBuffer[j], &component, componentTypeSize);
		}
		else if (componentType == "short")
		{
			if (!bufferComponentValue.IsInt())
				return false;

			short component = (short)bufferComponentValue.GetInt();
			::memcpy(&componentBuffer[j], &component, componentTypeSize);
		}
		else if (componentType == "uint")
		{
			if (!bufferComponentValue.IsInt())
				return false;

			unsigned int component = (unsigned int)bufferComponentValue.GetInt();
			::memcpy(&componentBuffer[j], &component, componentTypeSize);
		}
		else if (componentType == "ushort")
		{
			if (!bufferComponentValue.IsInt())
				return false;

			unsigned short component = (unsigned short)bufferComponentValue.GetInt();
			::memcpy(&componentBuffer[j], &component, componentTypeSize);
		}

		j += componentTypeSize;
	}

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = componentBuffer.get();
	HRESULT result = Game::Get()->GetDevice()->CreateBuffer(&bufferDesc, &subResourceData, &this->buffer);
	if (FAILED(result))
		return false;

	return true;
}

/*virtual*/ bool Buffer::Unload()
{
	SafeRelease(this->buffer);

	return true;
}