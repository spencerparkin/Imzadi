#include "Buffer.h"
#include "Game.h"
#include "Log.h"
#include <stdint.h>

using namespace Imzadi;

//-------------------------------------- Buffer --------------------------------------

Buffer::Buffer()
{
	this->buffer = nullptr;
	this->strideBytes = 0;
	this->numElements = 0;
	this->componentFormat = DXGI_FORMAT_UNKNOWN;
	this->canBeCached = true;
}

/*virtual*/ Buffer::~Buffer()
{
	this->Unload();
}

/*virtual*/ bool Buffer::CanBeCached() const
{
	return this->canBeCached;
}

/*virtual*/ bool Buffer::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("JSON data for buffer is not an object.");
		return false;
	}

	if (!jsonDoc.HasMember("type") || !jsonDoc["type"].IsString())
	{
		IMZADI_LOG_ERROR("No \"type\" field in JSON data for buffer.");
		return false;
	}

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
	{
		IMZADI_LOG_ERROR("Could not decypher component type: " + componentType);
		return false;
	}

	if (!jsonDoc.HasMember("stride") || !jsonDoc["stride"].IsInt())
	{
		IMZADI_LOG_ERROR("No \"stride\" member in JSON data.");
		return false;
	}

	UINT strideComponents = jsonDoc["stride"].GetInt();
	this->strideBytes = strideComponents * componentTypeSize;

	if (!jsonDoc.HasMember("bind") || !jsonDoc["bind"].IsString())
	{
		IMZADI_LOG_ERROR("No \"bind\" member in JSON data.");
		return false;
	}

	std::string bind = jsonDoc["bind"].GetString();

	if (!jsonDoc.HasMember("buffer"))
	{
		IMZADI_LOG_ERROR("No \"buffer\" member in JSON data.");
		return false;
	}

	const rapidjson::Value& bufferValue = jsonDoc["buffer"];
	if (!bufferValue.IsArray())
	{
		IMZADI_LOG_ERROR("The \"buffer\" member is not an array.");
		return false;
	}

	if (bufferValue.Size() == 0 || bufferValue.Size() % strideComponents != 0)
	{
		IMZADI_LOG_ERROR(std::format("The buffer size ({}) is zero or not divisible by the stride ({}).", bufferValue.Size(), strideComponents));
		return false;
	}

	UINT numComponents = bufferValue.Size();
	this->numElements = numComponents / strideComponents;
	
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = numComponents * componentTypeSize;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = 0;

	if (jsonDoc.HasMember("usage") && jsonDoc["usage"].IsString())
	{
		std::string usage = jsonDoc["usage"].GetString();
		if (usage == "dynamic")
		{
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			this->canBeCached = false;
		}
	}

	if (bind == "vertex")
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	else if (bind == "index")
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	else
	{
		IMZADI_LOG_ERROR(std::format("The bind ({}) is not recognized.", bind.c_str()));
		return false;
	}

	BYTE* bareComponentBuffer = nullptr;
	if (jsonDoc.HasMember("bare_buffer") && jsonDoc["bare_buffer"].IsBool() && jsonDoc["bare_buffer"].GetBool())
	{
		this->bareBuffer.Set(new BareBuffer());
		this->bareBuffer->SetSize(bufferDesc.ByteWidth);
		bareComponentBuffer = this->bareBuffer->GetBuffer();
	}

	std::unique_ptr<uint8_t[]> componentBuffer(new uint8_t[bufferDesc.ByteWidth]);
	int j = 0;

	union
	{
		float floatValue;
		int intValue;
		unsigned uintValue;
		short shortValue;
		unsigned short ushortValue;
	} component;

	for (int i = 0; i < bufferValue.Size(); i++)
	{
		const rapidjson::Value& bufferComponentValue = bufferValue[i];
		
		if (componentType == "float")
		{
			if (!bufferComponentValue.IsFloat())
			{
				IMZADI_LOG_ERROR("Expected buffer component to be a float.");
				return false;
			}

			component.floatValue = bufferComponentValue.GetFloat();
		}
		else if (componentType == "int")
		{
			if (!bufferComponentValue.IsInt())
			{
				IMZADI_LOG_ERROR("Expected buffer component to be a int.");
				return false;
			}

			component.intValue = bufferComponentValue.GetInt();
		}
		else if (componentType == "short")
		{
			if (!bufferComponentValue.IsInt())
			{
				IMZADI_LOG_ERROR("Expected buffer component to be a short.");
				return false;
			}

			component.shortValue = (short)bufferComponentValue.GetInt();
		}
		else if (componentType == "uint")
		{
			if (!bufferComponentValue.IsInt())
			{
				IMZADI_LOG_ERROR("Expected buffer component to be an unsigned int.");
				return false;
			}

			component.uintValue = (unsigned int)bufferComponentValue.GetInt();
		}
		else if (componentType == "ushort")
		{
			if (!bufferComponentValue.IsInt())
			{
				IMZADI_LOG_ERROR("Expected buffer component to be an unsigned short.");
				return false;
			}

			component.ushortValue = (unsigned short)bufferComponentValue.GetInt();
		}
		else
		{
			IMZADI_LOG_ERROR(std::format("Component type \"{}\" unrecognized or not yet supported.", componentType.c_str()));
			IMZADI_ASSERT(false);
		}

		::memcpy(&componentBuffer[j], &component, componentTypeSize);
		if (bareComponentBuffer)
			::memcpy(&bareComponentBuffer[j], &component, componentTypeSize);

		j += componentTypeSize;
	}

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = componentBuffer.get();
	HRESULT result = Game::Get()->GetDevice()->CreateBuffer(&bufferDesc, &subResourceData, &this->buffer);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("CreateBuffer() failed with error code: {}", result));
		return false;
	}

	return true;
}

/*virtual*/ bool Buffer::Unload()
{
	SafeRelease(this->buffer);

	return true;
}

bool Buffer::GetBareBuffer(Reference<BareBuffer>& givenBareBuffer)
{
	if (!this->bareBuffer)
	{
		this->bareBuffer.Set(new BareBuffer());
		this->bareBuffer->SetSize(this->numElements * this->strideBytes);
		
		ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();
		if (!deviceContext)
		{
			IMZADI_LOG_ERROR("Failed to get DX device context.");
			return false;
		}

		D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
		HRESULT result = deviceContext->Map(this->buffer, 0, D3D11_MAP_READ, 0, &mappedSubresource);
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to map DX buffer.");
			return false;
		}

		::memcpy(bareBuffer->GetBuffer(), mappedSubresource.pData, this->bareBuffer->GetSize());
		deviceContext->Unmap(this->buffer, 0);
	}

	givenBareBuffer = this->bareBuffer;
	return true;
}

//-------------------------------------- BareBuffer --------------------------------------

BareBuffer::BareBuffer()
{
	this->buffer = nullptr;
	this->bufferSize = 0;
}

/*virtual*/ BareBuffer::~BareBuffer()
{
	this->SetSize(0);
}

void BareBuffer::SetSize(UINT size)
{
	delete[] this->buffer;
	this->buffer = nullptr;
	this->bufferSize = size;
	if (size > 0)
		this->buffer = new BYTE[size];
}

UINT BareBuffer::GetSize() const
{
	return this->bufferSize;
}

BareBuffer* BareBuffer::Clone() const
{
	auto copy = new BareBuffer();
	copy->SetSize(this->bufferSize);
	::memcpy(copy->GetBuffer(), this->buffer, this->bufferSize);
	return copy;
}