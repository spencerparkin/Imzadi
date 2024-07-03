#include "Buffer.h"
#include "Game.h"
#include "Log.h"
#include <stdint.h>
#include <compressapi.h>

using namespace Imzadi;

//-------------------------------------- Buffer --------------------------------------

Buffer::Buffer()
{
	this->bufferType = Type::UNKNOWN;
	this->bufferSize = 0;
	this->mappedBufferData = nullptr;
}

/*virtual*/ Buffer::~Buffer()
{
}

/*virtual*/ bool Buffer::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("JSON data for buffer is not an object.");
		return false;
	}

	std::unique_ptr<unsigned char> dataBuffer;
	uint32_t dataSizeBytes = 0;

	if (jsonDoc.HasMember("data") && jsonDoc["data"].IsString())
	{
		std::string bufferDataFile = jsonDoc["data"].GetString();
		if (!assetCache->ResolveAssetPath(bufferDataFile))
		{
			IMZADI_LOG_ERROR("Failed to resolve buffer data file: " + bufferDataFile);
			return false;
		}

		std::filesystem::path bufferDataPath(bufferDataFile);
		dataSizeBytes = std::filesystem::file_size(bufferDataPath);
		if (dataSizeBytes == 0)
		{
			IMZADI_LOG_ERROR("The size of the buffer data file is zero.");
			return false;
		}

		std::fstream fileStream;
		fileStream.open(bufferDataFile.c_str(), std::ios::in | std::ios::binary);
		if (!fileStream.is_open())
		{
			IMZADI_LOG_ERROR("Failed to open buffer data file: " + bufferDataFile);
			return false;
		}

		dataBuffer.reset(new uint8_t[dataSizeBytes]);
		fileStream.read((char*)dataBuffer.get(), dataSizeBytes);
		fileStream.close();
	}
	else
	{
		if (!jsonDoc.HasMember("size") || !jsonDoc["size"].IsUint())
		{
			IMZADI_LOG_ERROR("If no buffer data is given, a buffer size must be given.");
			return false;
		}

		dataSizeBytes = jsonDoc["size"].GetUint();
		if (dataSizeBytes == 0)
		{
			IMZADI_LOG_ERROR("The size of the buffer is zero.");
			return false;
		}

		dataBuffer.reset(new uint8_t[dataSizeBytes]);
		::memset(dataBuffer.get(), 0, dataSizeBytes);
	}

	if (jsonDoc.HasMember("compressed") && jsonDoc["compressed"].GetBool())
	{
		if (!jsonDoc.HasMember("decompressed_size") || !jsonDoc["decompressed_size"].IsUint())
		{
			IMZADI_LOG_ERROR("Can't decompress buffer if not given the decmpression size.");
			return false;
		}

		ULONG_PTR uncompressedSizeBytes = jsonDoc["decompressed_size"].GetUint();

		DECOMPRESSOR_HANDLE decompressor = NULL;

		if (!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &decompressor))
		{
			IMZADI_LOG_ERROR("Failed to create decompressor.  Error code: %d", GetLastError());
			return false;
		}

		std::unique_ptr<unsigned char> decompressedDataBuffer;
		decompressedDataBuffer.reset(new unsigned char[uncompressedSizeBytes]);

		if (!Decompress(decompressor, dataBuffer.get(), dataSizeBytes, decompressedDataBuffer.get(), uncompressedSizeBytes, &uncompressedSizeBytes))
		{
			IMZADI_LOG_ERROR("Failed to decompress texture data.  Error code: %d", GetLastError());
			return false;
		}

		CloseDecompressor(decompressor);
		dataBuffer.swap(decompressedDataBuffer);
	}

	if (!jsonDoc.HasMember("type") || !jsonDoc["type"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"type\" member or it's not a string.");
		return false;
	}

	std::string bufferTypeStr = jsonDoc["type"].GetString();
	Type desiredType = Type::UNKNOWN;
	if (bufferTypeStr == "static")
		desiredType = Type::STATIC;
	else if (bufferTypeStr == "dynamic_small")
		desiredType = Type::DYNAMIC_SMALL;
	else if (bufferTypeStr == "dynamic_large")
		desiredType = Type::DYNAMIC_LARGE;
	else
	{
		IMZADI_LOG_ERROR("Buffer type \"%s\" unknonwn or not yet supported.", bufferTypeStr.c_str());
		return false;
	}

	if (!this->Create(desiredType, dataBuffer.get(), dataSizeBytes))
	{
		IMZADI_LOG_ERROR("Failed to create buffer!");
		return false;
	}

	return true;
}

/*virtual*/ bool Buffer::Unload()
{
	if (this->bufferType == Type::DYNAMIC_SMALL && this->uploadBufferResource.Get())
		this->uploadBufferResource->Unmap(0, nullptr);

	this->uploadBufferResource.Reset();
	this->bufferResource.Reset();
	this->bufferSize = 0;
	this->mappedBufferData = nullptr;
	return true;
}

bool Buffer::Create(Type desiredBufferType, const uint8_t* initialBufferData, uint32_t initialBufferSize)
{
	if (this->bufferType != Type::UNKNOWN)
	{
		IMZADI_LOG_ERROR("Buffer type already established.");
		return false;
	}

	this->bufferType = desiredBufferType;
	this->bufferSize = initialBufferSize;

	if (this->bufferSize == 0)
	{
		IMZADI_LOG_ERROR("Tried to create zero-size buffer.");
		return false;
	}

	if (this->bufferType == Type::STATIC && !initialBufferData)
	{
		IMZADI_LOG_ERROR("An initial buffer must be given for static buffers.");
		return false;
	}

	HRESULT result = 0;

	Game::Get()->WaitForGPUIdle();

	ID3D12Device* device = Game::Get()->GetDevice();

	D3D12_RESOURCE_DESC resourceDesc;
	::memset(&resourceDesc, 0, sizeof(resourceDesc));
	if (!this->GetResourceDesc(resourceDesc))
		return false;

	if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && resourceDesc.Width != initialBufferSize)
	{
		IMZADI_LOG_ERROR("Resource descibed as buffer but it is of the wrong size.");
		return false;
	}

	D3D12_HEAP_PROPERTIES heapProperties;
	::memset(&heapProperties, 0, sizeof(heapProperties));
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	result = device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&this->uploadBufferResource));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create upload buffer resource with error: %d", result);
		return false;
	}

	D3D12_RANGE readRange{};
	result = this->uploadBufferResource->Map(0, &readRange, reinterpret_cast<void**>(&mappedBufferData));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to map buffer resource memory with error: %d", result);
		return false;
	}

	if (this->bufferType == Type::STATIC || this->bufferType == Type::DYNAMIC_LARGE)
	{
		::memset(&heapProperties, 0, sizeof(heapProperties));
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		result = device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&this->bufferResource));
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR("Failed to create buffer resource with error: %d", result);
			return false;
		}

		if (this->bufferType == Type::STATIC)
		{
			Game::CommandData* commandData = Game::Get()->GetCommandData();
			commandData->allocator.Reset();
			commandData->list->Reset(commandData->allocator.Get(), nullptr);
			commandData->list->CopyResource(this->bufferResource.Get(), this->uploadBufferResource.Get());
			commandData->list->Close();

			ID3D12CommandList* commandListArray[] = { commandData->list.Get() };
			commandData->queue->ExecuteCommandLists(_countof(commandListArray), commandListArray);

			ResetEvent(commandData->fenceEvent);
			UINT64 i = commandData->fence->GetCompletedValue() + 1;
			commandData->fence->SetEventOnCompletion(i, commandData->fenceEvent);
			commandData->queue->Signal(commandData->fence.Get(), i);
			WaitForSingleObjectEx(commandData->fenceEvent, INFINITE, FALSE);

			this->uploadBufferResource->Unmap(0, nullptr);
			this->uploadBufferResource.Reset();
		}
	}

	if (this->bufferType == Type::DYNAMIC_LARGE || this->bufferType == Type::DYNAMIC_SMALL)
	{
		if (initialBufferData != nullptr)
		{
			if (!this->Write(initialBufferData, initialBufferSize, 0))
				return false;
		}
	}

	return true;
}

bool Buffer::Write(const uint8_t* inputBufferData, uint32_t inputBufferSize, uint32_t offset)
{
	if (this->bufferType != Type::DYNAMIC_SMALL && this->bufferType != Type::DYNAMIC_LARGE)
	{
		IMZADI_LOG_ERROR("Buffer type does not support writes.");
		return false;
	}

	if (offset + inputBufferSize > this->bufferSize)
	{
		IMZADI_LOG_ERROR("Given buffer won't fit into GPU buffer.");
		return false;
	}

	::memcpy(&this->mappedBufferData[offset], inputBufferData, inputBufferSize);

	if (this->bufferType == Type::DYNAMIC_LARGE)
	{
		// Note that it is very unlikely that this buffer class instance will
		// go out of scope before the queue is processed, but it is not impossible!
		// One obvious fix is to create a custom queue item class and then have it
		// own a reference to this buffer.  I'm going to just be lazy for now.
		Game::Get()->EnqueuePreRenderCallback([=](ID3D12GraphicsCommandList* commandList) {
			commandList->CopyResource(this->bufferResource.Get(), this->uploadBufferResource.Get());
		});

		return true;
	}
	
	return false;
}

bool Buffer::Read(uint8_t* outputBufferData, uint32_t& outputBufferSize)
{
	IMZADI_LOG_ERROR("Read operation on buffers not yet supported.");
	return false;
}

/*virtual*/ bool Buffer::GetResourceDesc(D3D12_RESOURCE_DESC& resourceDesc)
{
	IMZADI_LOG_ERROR("A buffer class derivative must override the GetResourceDesc() virtual method.");
	return false;
}

ID3D12Resource* Buffer::GetBufferResource()
{
	switch (this->bufferType)
	{
	case Type::DYNAMIC_SMALL:
		return this->uploadBufferResource.Get();
	case Type::DYNAMIC_LARGE:
	case Type::STATIC:
		return this->bufferResource.Get();
	}

	return nullptr;
}

//-------------------------------------- ElementBuffer --------------------------------------

ElementBuffer::ElementBuffer()
{
	this->strideBytes = 0;
	this->numElements = 0;
	this->componentFormat = DXGI_FORMAT_UNKNOWN;
}

/*virtual*/ ElementBuffer::~ElementBuffer()
{
	
}

/*virtual*/ bool ElementBuffer::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected JSON doc to be an object.");
		return false;
	}

	if (!jsonDoc.HasMember("component_format") || !jsonDoc["component_format"].IsString())
	{
		IMZADI_LOG_ERROR("No \"type\" field in JSON data for buffer.");
		return false;
	}

	UINT32 componentTypeSize = 0;
	std::string componentType = jsonDoc["component_format"].GetString();
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

	if (!jsonDoc.HasMember("components_per_elements") || !jsonDoc["components_per_elements"].IsInt())
	{
		IMZADI_LOG_ERROR("No \"components_per_elements\" member in JSON data.");
		return false;
	}

	UINT componentsPerElement = jsonDoc["components_per_elements"].GetInt();
	this->strideBytes = componentsPerElement * componentTypeSize;

	if (this->bufferSize % this->strideBytes != 0)
	{
		IMZADI_LOG_ERROR("The buffer size %d is not a multiple of the stride %d.", this->bufferSize, this->strideBytes);
		return false;
	}

	if (!Buffer::Load(jsonDoc, assetCache))
		return false;

	this->numElements = this->bufferSize / this->strideBytes;

	return true;
}

/*virtual*/ bool ElementBuffer::GetResourceDesc(D3D12_RESOURCE_DESC& resourceDesc)
{
	::memset(&resourceDesc, 0, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = this->bufferSize;
	return true;
}