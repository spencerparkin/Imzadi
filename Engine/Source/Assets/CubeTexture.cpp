#include "CubeTexture.h"
#include "Texture.h"
#include "Game.h"
#include "Log.h"

using namespace Imzadi;

CubeTexture::CubeTexture()
{
	this->cubeTexture = nullptr;
	this->cubeTextureView = nullptr;
	this->samplerState = nullptr;
}

/*virtual*/ CubeTexture::~CubeTexture()
{
}

/*virtual*/ bool CubeTexture::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected JSON root to be an object.");
		return false;
	}

	if (!jsonDoc.HasMember("texture_array") || !jsonDoc["texture_array"].IsArray())
	{
		IMZADI_LOG_ERROR("Didn't find \"texture_array\" member.");
		return false;
	}

	const rapidjson::Value& textureArrayValue = jsonDoc["texture_array"];
	if (textureArrayValue.Size() != 6)
	{
		IMZADI_LOG_ERROR("Expected exactly 6 textures in the texture array for a cube-map, found %d.", textureArrayValue.Size());
		return false;
	}
	
	std::vector<Reference<Texture>> textureArray;
	for (int i = 0; i < textureArrayValue.Size(); i++)
	{
		const rapidjson::Value& textureFileValue = textureArrayValue[i];
		if (!textureFileValue.IsString())
		{
			IMZADI_LOG_ERROR("Expected texture array entry to be a string.");
			return false;
		}

		Reference<Asset> asset;
		std::string textureFile(textureFileValue.GetString());
		if (!assetCache->LoadAsset(textureFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to load texture %d (%s) from the texture array.", i, textureFile.c_str());
			return false;
		}

		Reference<Texture> texture;
		texture.SafeSet(asset);
		if (!texture)
		{
			IMZADI_LOG_ERROR("Whatever loaded from entry %d was not a texture.", i);
			return false;
		}

		textureArray.push_back(texture);
	}

	bool success = this->Load(textureArray);

	for (auto texture : textureArray)
		texture->Unload();

	return success;
}

bool CubeTexture::Load(std::vector<Reference<Texture>>& textureArray)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureArray[0]->GetTexture()->GetDesc(&textureDesc);

	for (int i = 1; i < textureArray.size(); i++)
	{
		D3D11_TEXTURE2D_DESC otherTextureDesc{};
		textureArray[i]->GetTexture()->GetDesc(&otherTextureDesc);

		if (textureDesc.Width != otherTextureDesc.Width ||
			textureDesc.Height != otherTextureDesc.Height)
		{
			IMZADI_LOG_ERROR("Cube-map textures don't have consistent dimensions.");
			return false;
		}

		if (textureDesc.Format != otherTextureDesc.Format)
		{
			IMZADI_LOG_ERROR("Cube-map textures don't have consistent format.");
			return false;
		}
	}

	ID3D11Device* device = Game::Get()->GetDevice();
	ID3D11DeviceContext* deviceContext = Game::Get()->GetDeviceContext();

	textureDesc.ArraySize = textureArray.size();
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT result = device->CreateTexture2D(&textureDesc, NULL, &this->cubeTexture);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create cube texture with error code: %d", result);
		return false;
	}

	for (int i = 0; i < textureArray.size(); i++)
	{
		for (int j = 0; j < textureDesc.MipLevels; j++)
		{
			D3D11_MAPPED_SUBRESOURCE subResource{};
			result = deviceContext->Map(textureArray[i]->GetTexture(), j, D3D11_MAP_READ, 0, &subResource);
			if (FAILED(result))
			{
				IMZADI_LOG_ERROR("Failed to make cube texture %d, mip %d.", i, j);
				return false;
			}

			UINT k = D3D11CalcSubresource(j, i, textureDesc.MipLevels);
			deviceContext->UpdateSubresource(this->cubeTexture, k, nullptr, subResource.pData, subResource.RowPitch, subResource.DepthPitch);
			deviceContext->Unmap(textureArray[i]->GetTexture(), j);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc{};
	resourceViewDesc.Format = textureDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	resourceViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;

	result = device->CreateShaderResourceView(this->cubeTexture, &resourceViewDesc, &this->cubeTextureView);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to create shader resource view for cube texture.  Error code: %d", result);
		return false;
	}

	// TODO: Maybe we only need one sampler state for the entire engine for now?

	return true;
}

/*virtual*/ bool CubeTexture::Unload()
{
	SafeRelease(this->cubeTexture);
	SafeRelease(this->cubeTextureView);
	SafeRelease(this->samplerState);

	return true;
}