#pragma once

#include "AssetCache.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include <d3d11.h>
#include <unordered_map>

namespace Imzadi
{
	class IMZADI_API Shader : public Asset
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

	inline uint32_t Align16(uint32_t offset)
	{
		// I'm sure there is a fancy bit-twittling way
		// to do this, but this is fine for now.
		while (offset % 16 != 0)
			offset++;
		return offset;
	}

	inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const double* scalar)
	{
		if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == sizeof(float))
		{
			*(float*)&((uint8_t*)subResource->pData)[constant->offset] = float(*scalar);
		}
	}

	inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Vector2* vector)
	{
		if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 2 * sizeof(float))
		{
			float* floatArray = (float*)&((uint8_t*)subResource->pData)[constant->offset];
			floatArray[0] = vector->x;
			floatArray[1] = vector->y;
		}
	}

	inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Imzadi::Vector3* vector)
	{
		if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 3 * sizeof(float))
		{
			float* floatArray = (float*)&((uint8_t*)subResource->pData)[constant->offset];
			floatArray[0] = vector->x;
			floatArray[1] = vector->y;
			floatArray[2] = vector->z;
		}
	}

	inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Vector4* vector)
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

	inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Matrix3x3* matrix)
	{
		if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 16 * sizeof(float))
		{
			float* ele = (float*)&((uint8_t*)subResource->pData)[constant->offset];
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					*ele++ = float(matrix->ele[j][i]);	// Note the swap of i and j here!
		}
	}

	inline void StoreShaderConstant(D3D11_MAPPED_SUBRESOURCE* subResource, const Shader::Constant* constant, const Matrix4x4* matrix)
	{
		if (constant->format == DXGI_FORMAT_R32_FLOAT && constant->size == 16 * sizeof(float))
		{
			float* ele = (float*)&((uint8_t*)subResource->pData)[constant->offset];
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					*ele++ = float(matrix->ele[j][i]);	// Note the swap of i and j here!
		}
	}
}