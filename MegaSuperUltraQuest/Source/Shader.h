#pragma once

#include "AssetCache.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include <d3d11.h>
#include <unordered_map>

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
	ID3D11Buffer* GetConstantsBuffer() { return this->constantsBuffer; }
	UINT GetConstantsBufferSize() { return this->constantsBufferSize; }

	struct Constant
	{
		UINT offset;
		UINT size;
		DXGI_FORMAT format;
	};

	bool GetConstantInfo(const std::string& name, const Constant*& constant);

private:

	bool CompileShader(const std::string& shaderFile, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob*& blob);

	bool PopulateInputLayout(D3D11_INPUT_ELEMENT_DESC* inputLayoutArray, const rapidjson::Value& inputLayoutArrayValue, std::vector<std::string>& semanticArray);

	ID3D11InputLayout* inputLayout;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11Buffer* constantsBuffer;
	UINT constantsBufferSize;
	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;

	typedef std::unordered_map<std::string, Constant> ConstantsMap;
	ConstantsMap constantsMap;
};

template<typename T>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const T* value)
{
}

template<>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const double* scalar)
{
	if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == sizeof(float))
	{
		*(float*)&((uint8_t*)subResource->pData)[constant->offset] = float(*scalar);
	}
}

template<>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Collision::Vector2* vector)
{
	if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 2 * sizeof(float))
	{
		float* floatArray = (float*)&((uint8_t*)subResource->pData)[constant->offset];
		floatArray[0] = vector->x;
		floatArray[1] = vector->y;
	}
}

template<>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Collision::Vector3* vector)
{
	if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 3 * sizeof(float))
	{
		float* floatArray = (float*)&((uint8_t*)subResource->pData)[constant->offset];
		floatArray[0] = vector->x;
		floatArray[1] = vector->y;
		floatArray[2] = vector->z;
	}
}

template<>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Collision::Vector4* vector)
{
	if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 4 * sizeof(float))
	{
		float* floatArray = (float*)&((uint8_t*)subResource->pData)[constant->offset];
		floatArray[0] = vector->x;
		floatArray[1] = vector->y;
		floatArray[2] = vector->z;
		floatArray[3] = vector->w;
	}
}

template<>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Collision::Matrix3x3* matrix)
{
	if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 16 * sizeof(float))
	{
		float* ele = (float*)&((uint8_t*)subResource->pData)[constant->offset];
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				*ele++ = float(matrix->ele[j][i]);	// Note the swap of i and j here!
	}
}

template<>
inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Collision::Matrix4x4* matrix)
{
	if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 16 * sizeof(float))
	{
		float* ele = (float*)&((uint8_t*)subResource->pData)[constant->offset];
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				*ele++ = float(matrix->ele[j][i]);	// Note the swap of i and j here!
	}
}