#include "Texture.h"
#include "Game.h"
#include "Log.h"
#include <compressapi.h>

using namespace Imzadi;

Texture::Texture()
{
	this->texture = nullptr;
	this->textureView = nullptr;
}

/*virtual*/ Texture::~Texture()
{
}

/*virtual*/ bool Texture::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected JSON doc to be an object.");
		return false;
	}

	if (!jsonDoc.HasMember("width") || !jsonDoc.HasMember("height"))
	{
		IMZADI_LOG_ERROR("No \"width\" and \"height\" members found.");
		return false;
	}

	if (!jsonDoc["width"].IsUint() || !jsonDoc["height"].IsUint())
	{
		IMZADI_LOG_ERROR("Expected \"width\" and \"height\" members to be unsigned integers.");
		return false;
	}

	uint32_t textureWidth = jsonDoc["width"].GetUint();
	uint32_t textureHeight = jsonDoc["height"].GetUint();

	if (!jsonDoc.HasMember("format") || !jsonDoc["format"].IsString())
	{
		IMZADI_LOG_ERROR("Expected \"format\" member to exist and be a string.");
		return false;
	}

	std::string format = jsonDoc["format"].GetString();
	uint32_t texelSizeBytes = 0;
	if (format == "RGBA")
		texelSizeBytes = 4;
	else if (format == "RGB")
		texelSizeBytes = 3;
	else if (format == "A")
		texelSizeBytes = 1;
	else
	{
		IMZADI_LOG_ERROR("Format \"%s\" not recognized or not yet supported.", format.c_str());
		return false;
	}

	if (!jsonDoc.HasMember("data") || !jsonDoc["data"].IsString())
	{
		IMZADI_LOG_ERROR("No \"data\" member in JSON data or it's not a string.");
		return false;
	}

	std::string textureDataFile = jsonDoc["data"].GetString();
	if (!assetCache->ResolveAssetPath(textureDataFile))
	{
		IMZADI_LOG_ERROR("Failed to resolve texture data file: " + textureDataFile);
		return false;
	}

	std::filesystem::path textureDataPath(textureDataFile);
	uint32_t dataSizeBytes = std::filesystem::file_size(textureDataPath);
	if (dataSizeBytes == 0)
	{
		IMZADI_LOG_ERROR("The size of the texture data file is zero.");
		return false;
	}

	std::fstream fileStream;
	fileStream.open(textureDataFile.c_str(), std::ios::in | std::ios::binary);
	if (!fileStream.is_open())
	{
		IMZADI_LOG_ERROR("Failed to open texture data file: " + textureDataFile);
		return false;
	}

	std::unique_ptr<unsigned char> dataBuffer(new unsigned char[dataSizeBytes]);
	fileStream.read((char*)dataBuffer.get(), dataSizeBytes);
	fileStream.close();

	const unsigned char* textureData = dataBuffer.get();

	std::unique_ptr<unsigned char> decompressedDataBuffer;
	if (jsonDoc.HasMember("compressed") && jsonDoc["compressed"].GetBool())
	{
		DECOMPRESSOR_HANDLE decompressor = NULL;

		if (!CreateDecompressor(COMPRESS_ALGORITHM_XPRESS, NULL, &decompressor))
		{
			IMZADI_LOG_ERROR("Failed to create decompressor.  Error code: %d", GetLastError());
			return false;
		}

		ULONG_PTR uncompressedSizeBytes = textureWidth * textureHeight * texelSizeBytes;
		decompressedDataBuffer.reset(new unsigned char[uncompressedSizeBytes]);

		if (!Decompress(decompressor, dataBuffer.get(), dataSizeBytes, decompressedDataBuffer.get(), uncompressedSizeBytes, &uncompressedSizeBytes))
		{
			IMZADI_LOG_ERROR("Failed to decompress texture data.  Error code: %d", GetLastError());
			return false;
		}

		CloseDecompressor(decompressor);
		textureData = decompressedDataBuffer.get();
	}

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;		// TODO: Need to make MIPs at some point.
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (jsonDoc.HasMember("for_staging") && jsonDoc["for_staging"].GetBool())
	{
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		textureDesc.BindFlags = 0;
	}

	if(format == "RGBA")
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	else if (format == "A")
		textureDesc.Format = DXGI_FORMAT_A8_UNORM;
	else
	{
		IMZADI_LOG_ERROR("Could not use format \"%s\".", format.c_str());
		return false;
	}

	D3D11_SUBRESOURCE_DATA textureSubresourceData{};
	textureSubresourceData.pSysMem = textureData;
	textureSubresourceData.SysMemPitch = textureWidth * texelSizeBytes;

	HRESULT result = Game::Get()->GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->texture);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("CreateTexture2D() failed with error code: {}", result));
		return false;
	}

	if (jsonDoc.HasMember("for_staging") && jsonDoc["for_staging"].GetBool())
		return true;

	result = Game::Get()->GetDevice()->CreateShaderResourceView(texture, NULL, &this->textureView);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("CreateShaderResourceView() failed with error code: {}", result));
		return false;
	}

	return true;
}

/*virtual*/ bool Texture::Unload()
{
	SafeRelease(this->textureView);
	SafeRelease(this->texture);

	return true;
}