#include "Texture.h"
#include "Game.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::Texture()
{
	this->texture = nullptr;
	this->textureView = nullptr;
	this->samplerState = nullptr;
}

/*virtual*/ Texture::~Texture()
{
}

/*virtual*/ bool Texture::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (jsonDoc.HasMember("imag_file") || !jsonDoc["image_file"].IsString())
		return false;

	std::string imageFile = jsonDoc["image_file"].GetString();
	if (!assetCache->ResolveAssetPath(imageFile))
		return false;

	stbi_set_flip_vertically_on_load(1);

	int textureWidth = 0, textureHeight = 0;
	int textureChannels = 0;
	int desiredChannels = 4;
	unsigned char* textureData = stbi_load(imageFile.c_str(), &textureWidth, &textureHeight, &textureChannels, desiredChannels);
	if (!textureData)
		return false;

	if (textureChannels != desiredChannels)
		return false;

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureSubresourceData{};
	textureSubresourceData.pSysMem = textureData;
	textureSubresourceData.SysMemPitch = textureWidth * desiredChannels;

	HRESULT result = Game::Get()->GetDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->texture);
	if (FAILED(result))
		return false;

	result = Game::Get()->GetDevice()->CreateShaderResourceView(texture, NULL, &this->textureView);
	if (FAILED(result))
		return false;

	::free(textureData);

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	result = Game::Get()->GetDevice()->CreateSamplerState(&samplerDesc, &this->samplerState);
	if (FAILED(result))
		return false;

	return true;
}

/*virtual*/ bool Texture::Unload()
{
	if (this->textureView)
	{
		this->textureView->Release();
		this->textureView = nullptr;
	}

	if (this->texture)
	{
		this->texture->Release();
		this->texture = nullptr;
	}

	if (this->samplerState)
	{
		this->samplerState->Release();
		this->samplerState = nullptr;
	}

	return false;
}