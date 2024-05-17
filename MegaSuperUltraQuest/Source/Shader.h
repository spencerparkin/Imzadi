#pragma once

#include "AssetCache.h"
#include <d3d11.h>

class Shader : public Asset
{
public:
	Shader();
	virtual ~Shader();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;

	ID3D11InputLayout* GetInputLayout() { return this->inputLayout; }
	ID3D11VertexShader* GetVertexShader() { return this->vertexShader; }
	ID3D11PixelShader* GetPixelShader() { return this->pixelShader; }

private:

	bool CompileShader(const std::string& shaderFile, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob*& blob);

	bool PopulateInputLayout(D3D11_INPUT_ELEMENT_DESC* inputLayoutArray, const rapidjson::Value& inputLayoutArrayValue, std::vector<std::string>& semanticArray);

	ID3D11InputLayout* inputLayout;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;
};