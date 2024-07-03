#pragma once

#include "AssetCache.h"
#include "Assets/ConstantsBuffer.h"
#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map>

using Microsoft::WRL::ComPtr;

namespace Imzadi
{
	class ConstantsBuffer;

	/**
	 * As if the term "shader" wasn't abused enough, we're not just a pixel shader here,
	 * or a vertex shader; we're an entire pipeline state.
	 */
	class IMZADI_API Shader : public Asset
	{
	public:
		Shader();
		virtual ~Shader();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;

		ID3D12PipelineState* GetPipelineState() { return this->pipelineState.Get(); }

	private:

		bool CompileShader(const std::string& shaderFile, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob*& blob);
		bool PopulateInputLayout(D3D12_INPUT_ELEMENT_DESC* inputLayoutArray, const rapidjson::Value& inputLayoutArrayValue, std::vector<std::string>& semanticArray);
		bool ConfigureRasterizationState(const rapidjson::Document& jsonDoc, D3D12_RASTERIZER_DESC& rasterizerDesc);
		bool ConfigureDepthStencilState(const rapidjson::Document& jsonDoc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
		bool ConfigureBlendState(const rapidjson::Document& jsonDoc, D3D12_BLEND_DESC& blendDesc);

		bool TranslateBlend(const std::string& blendStr, D3D12_BLEND& blend);
		bool TranslateBlendOp(const std::string& blendOpStr, D3D12_BLEND_OP& blendOp);

		ComPtr<ID3D12PipelineState> pipelineState;
		Reference<ConstantsBuffer> constantsBuffer;
	};
}