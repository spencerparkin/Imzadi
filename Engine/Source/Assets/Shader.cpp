#include "Shader.h"
#include "Game.h"
#include "Log.h"
#include <d3dcompiler.h>
#include <codecvt>
#include <locale>

using namespace Imzadi;

Shader::Shader()
{
}

/*virtual*/ Shader::~Shader()
{
}

/*virtual*/ bool Shader::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Given JSON doc was not an object.");
		return false;
	}

	// Do we really need to do this?  I'm not entirely sure, but I guess it couldn't hurt.
	Game::Get()->WaitForGPUIdle();

	HRESULT result = 0;

	ComPtr<ID3DBlob> psBlob, vsBlob;

	if (jsonDoc.HasMember("vs_shader_object") && jsonDoc.HasMember("ps_shader_object"))
	{
		if (!jsonDoc["vs_shader_object"].IsString() || !jsonDoc["ps_shader_object"].IsString())
		{
			IMZADI_LOG_ERROR("A \"vs_shader_object\" and \"ps_shader_object\" member are not both present.");
			return false;
		}

		std::string vsShaderObjFile = jsonDoc["vs_shader_object"].GetString();
		std::string psShaderObjFile = jsonDoc["ps_shader_object"].GetString();

		if (!assetCache->ResolveAssetPath(vsShaderObjFile))
		{
			IMZADI_LOG_ERROR("Failed to resolve path: " + vsShaderObjFile);
			return false;
		}

		if (!assetCache->ResolveAssetPath(psShaderObjFile))
		{
			IMZADI_LOG_ERROR("Failed to resolve path: " + psShaderObjFile);
			return false;
		}

		std::wstring vsShaderObjFileW = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(vsShaderObjFile);
		std::wstring psShaderObjFileW = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(psShaderObjFile);

		result = D3DReadFileToBlob(vsShaderObjFileW.c_str(), &vsBlob);
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR(std::format("Failed to read VS file blob ({}) with error code: {}", vsShaderObjFile.c_str(), result));
			return false;
		}

		result = D3DReadFileToBlob(psShaderObjFileW.c_str(), &psBlob);
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR(std::format("Failed to read PS file blob ({}) with error code: {}", psShaderObjFile.c_str(), result));
			return false;
		}
	}
	else if (jsonDoc.HasMember("shader_code"))
	{
		if (!jsonDoc["shader_code"].IsString())
		{
			IMZADI_LOG_ERROR("No \"shader_code\" member found.");
			return false;
		}

		std::string shaderCodeFile = jsonDoc["shader_code"].GetString();
		if (!assetCache->ResolveAssetPath(shaderCodeFile))
		{
			IMZADI_LOG_ERROR("Failed to resolve path: " + shaderCodeFile);
			return false;
		}

		std::string vsEntryPoint = "VS_Main";
		if (jsonDoc.HasMember("vs_entry_point") && jsonDoc["vs_entry_point"].IsString())
			vsEntryPoint = jsonDoc["vs_entry_point"].GetString();

		std::string psEntryPoint = "PS_Main";
		if (jsonDoc.HasMember("ps_entry_point") && jsonDoc["ps_entry_point"].IsString())
			psEntryPoint = jsonDoc["ps_entry_point"].GetString();

		std::string vsModel = "vs_5_0";
		if (jsonDoc.HasMember("vs_model") && jsonDoc["vs_model"].IsString())
			vsModel = jsonDoc["vs_model"].GetString();

		std::string psModel = "ps_5_0";
		if (jsonDoc.HasMember("ps_model") && jsonDoc["ps_model"].IsString())
			psModel = jsonDoc["ps_model"].GetString();

		if (!this->CompileShader(shaderCodeFile, vsEntryPoint, vsModel, *vsBlob.GetAddressOf()))
		{
			IMZADI_LOG_ERROR("VS compilation failed.");
			return false;
		}

		if (!this->CompileShader(shaderCodeFile, psEntryPoint, psModel, *psBlob.GetAddressOf()))
		{
			IMZADI_LOG_ERROR("PS compilation failed.");
			return false;
		}
	}

	if (!vsBlob.Get() || !psBlob.Get())
	{
		IMZADI_LOG_ERROR("Did not acquire both a VS and PS shader blob.");
		return false;
	}

	if (!jsonDoc.HasMember("vs_input_layout"))
	{
		IMZADI_LOG_ERROR("No \"vs_input_layout\" member found.");
		return false;
	}

	const rapidjson::Value& inputLayoutValue = jsonDoc["vs_input_layout"];
	if (!inputLayoutValue.IsArray() || inputLayoutValue.Size() == 0)
	{
		IMZADI_LOG_ERROR("The \"vs_input_layout\" member is not an array or it is empty.");
		return false;
	}

	std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]> inputElementDescArray(new D3D12_INPUT_ELEMENT_DESC[inputLayoutValue.Size()]);
	std::vector<std::string> semanticArray;
	semanticArray.reserve(inputLayoutValue.Size());
	if (!this->PopulateInputLayout(inputElementDescArray.get(), inputLayoutValue, semanticArray))
	{
		IMZADI_LOG_ERROR("Failed to populate VS input layout info.");
		return false;
	}

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	if (!this->ConfigureRasterizationState(jsonDoc, rasterizerDesc))
		return false;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	if (!this->ConfigureDepthStencilState(jsonDoc, depthStencilDesc))
		return false;

	D3D12_BLEND_DESC blendDesc{};
	if (!this->ConfigureBlendState(jsonDoc, blendDesc))
		return false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.InputLayout = { inputElementDescArray.get(), inputLayoutValue.Size() };
	pipelineStateDesc.pRootSignature = Game::Get()->GetRootSignature();
	pipelineStateDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pipelineStateDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pipelineStateDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pipelineStateDesc.PS.BytecodeLength = psBlob->GetBufferSize();
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.BlendState = blendDesc;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.SampleDesc.Count = 1;

	result = Game::Get()->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&this->pipelineState));
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR("Failed to created graphics pipeline state with error: %d", result);
		return false;
	}

	if (jsonDoc.HasMember("constants_buffer") && jsonDoc["constants_buffer"].IsString())
	{
		std::string constantsBufferFile = jsonDoc["constants_buffer"].GetString();
		Reference<Asset> asset;
		if (!assetCache->LoadAsset(constantsBufferFile, asset))
		{
			IMZADI_LOG_ERROR("Failed to load constants buffer asset for shader asset.");
			return false;
		}

		this->constantsBuffer.SafeSet(asset.Get());
		if (!this->constantsBuffer.Get())
		{
			IMZADI_LOG_ERROR("Whatever loaded for the constants buffer wasn't a constants buffer.");
			return false;
		}
	}

	return true;
}

bool Shader::PopulateInputLayout(D3D12_INPUT_ELEMENT_DESC* inputLayoutArray, const rapidjson::Value& inputLayoutArrayValue, std::vector<std::string>& semanticArray)
{
	for (int i = 0; i < inputLayoutArrayValue.Size(); i++)
	{
		D3D12_INPUT_ELEMENT_DESC* inputElementDesc = &inputLayoutArray[i];
		::memset(inputElementDesc, 0, sizeof(D3D12_INPUT_ELEMENT_DESC));

		const rapidjson::Value& inputLayoutElementValue = inputLayoutArrayValue[i];
		if (!inputLayoutElementValue.IsObject())
		{
			IMZADI_LOG_ERROR(std::format("Element {} was not an object.", i));
			return false;
		}

		if (!inputLayoutElementValue.HasMember("semantic") || !inputLayoutElementValue["semantic"].IsString())
		{
			IMZADI_LOG_ERROR("No \"semantic\" member found or it's not a string.");
			return false;
		}

		std::string semantic = inputLayoutElementValue["semantic"].GetString();
		semanticArray.push_back(semantic);
		inputElementDesc->SemanticName = semanticArray[semanticArray.size() - 1].c_str();

		if (inputLayoutElementValue.HasMember("semantic_index") && inputLayoutElementValue["semantic_index"].IsInt())
			inputElementDesc->SemanticIndex = inputLayoutElementValue["semantic_index"].GetInt();

		if (!inputLayoutElementValue.HasMember("element_format") || !inputLayoutElementValue["element_format"].IsString())
		{
			IMZADI_LOG_ERROR("No \"element_format\" member found or it's not a string.");
			return false;
		}

		std::string format = inputLayoutElementValue["element_format"].GetString();
		if (format == "R32G32_FLOAT")
			inputElementDesc->Format = DXGI_FORMAT_R32G32_FLOAT;
		else if (format == "R32G32B32_FLOAT")
			inputElementDesc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
		else if (format == "R32G32B32A32_FLOAT")
			inputElementDesc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		else
		{
			IMZADI_LOG_ERROR(std::format("The format \"{}\" is not recognize or not yet supported.", format.c_str()));
			return false;
		}

		inputElementDesc->AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementDesc->InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

		if (inputLayoutElementValue.HasMember("slot") && inputLayoutElementValue["slot"].IsInt())
			inputElementDesc->InputSlot = inputLayoutElementValue["slot"].GetInt();
	}

	return true;
}

bool Shader::CompileShader(const std::string& shaderFile, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob*& blob)
{
	std::wstring shaderFileW = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(shaderFile);

	UINT flags = 0;
#if _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#endif

	ComPtr<ID3DBlob> errorsBlob;
	HRESULT result = D3DCompileFromFile(shaderFileW.c_str(), nullptr, nullptr, entryPoint.c_str(), shaderModel.c_str(), flags, 0, &blob, &errorsBlob);
	if (FAILED(result))
	{
		const char* errorMsg = "Unknown error!";
		if (errorsBlob)
			errorMsg = (const char*)errorsBlob->GetBufferPointer();
		else if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorMsg = "File not found!";

		IMZADI_LOG_ERROR(std::format("Shader compilation of file {} failed with error code {}, because: {}", shaderFile.c_str(), result, errorMsg));
		return false;
	}

	return true;
}

bool Shader::ConfigureRasterizationState(const rapidjson::Document& jsonDoc, D3D12_RASTERIZER_DESC& rasterizerDesc)
{
	::memset(&rasterizerDesc, 0, sizeof(rasterizerDesc));

	if (jsonDoc.HasMember("fill_mode") && jsonDoc["fill_mode"].IsString())
	{
		std::string fillMode = jsonDoc["fill_mode"].GetString();

		if (fillMode == "FILL_SOLID")
			rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		else if (fillMode == "FILL_WIREFRAME")
			rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		else
		{
			IMZADI_LOG_ERROR("Did not recognize fill mode: %s", fillMode.c_str());
			return false;
		}
	}

	if (jsonDoc.HasMember("cull_mode") && jsonDoc["cull_mode"].IsString())
	{
		std::string cullMode = jsonDoc["cull_mode"].GetString();

		if (cullMode == "CULL_BACK")
			rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
		else if (cullMode == "CULL_FRONT")
			rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		else if (cullMode == "CULL_NONE")
			rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		else
		{
			IMZADI_LOG_ERROR("Did not recognize cull mode: %s", cullMode.c_str());
			return false;
		}
	}

	if (jsonDoc.HasMember("front_ccw") && jsonDoc["front_ccw"].IsBool())
	{
		if (jsonDoc["front_ccw"].GetBool())
			rasterizerDesc.FrontCounterClockwise = TRUE;
		else
			rasterizerDesc.FrontCounterClockwise = FALSE;
	}

	if (jsonDoc.HasMember("depth_clip_enabled") && jsonDoc["depth_clip_enabled"].IsBool())
	{
		if (jsonDoc["depth_clip_enabled"].GetBool())
			rasterizerDesc.DepthClipEnable = TRUE;
		else
			rasterizerDesc.DepthClipEnable = FALSE;
	}

	return true;
}

bool Shader::ConfigureDepthStencilState(const rapidjson::Document& jsonDoc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc)
{
	::memset(&depthStencilDesc, 0, sizeof(depthStencilDesc));

	if (jsonDoc.HasMember("depth_enabled") && jsonDoc["depth_enabled"].IsBool())
	{
		if (jsonDoc["depth_enabled"].GetBool())
			depthStencilDesc.DepthEnable = TRUE;
		else
			depthStencilDesc.DepthEnable = FALSE;
	}

	if (jsonDoc.HasMember("stencil_enabled") && jsonDoc["stencil_enabled"].IsBool())
	{
		if (jsonDoc["stencil_enabled"].GetBool())
			depthStencilDesc.StencilEnable = TRUE;
		else
			depthStencilDesc.StencilEnable = FALSE;
	}

	return true;
}

bool Shader::ConfigureBlendState(const rapidjson::Document& jsonDoc, D3D12_BLEND_DESC& blendDesc)
{
	::memset(&blendDesc, 0, sizeof(blendDesc));

	if (jsonDoc.HasMember("blend_enabled") && jsonDoc["blend_enabled"].IsBool())
	{
		if (jsonDoc["blend_enabled"].GetBool())
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
		else
			blendDesc.RenderTarget[0].BlendEnable = FALSE;
	}

	if (jsonDoc.HasMember("src_blend") && jsonDoc["src_blend"].IsString())
	{
		std::string srcBlend = jsonDoc["src_blend"].GetString();

		if (!this->TranslateBlend(srcBlend, blendDesc.RenderTarget[0].SrcBlend))
			return false;
	}

	if (!jsonDoc.HasMember("dest_blend") && jsonDoc["dest_blend"].IsString())
	{
		std::string destBlend = jsonDoc["dest_blend"].GetString();

		if (!this->TranslateBlend(destBlend, blendDesc.RenderTarget[0].DestBlend))
			return false;
	}

	if (!jsonDoc.HasMember("blend_op") && jsonDoc["blend_op"].IsString())
	{
		std::string blendOpStr = jsonDoc["blend_op"].GetString();

		if (!this->TranslateBlendOp(blendOpStr, blendDesc.RenderTarget[0].BlendOp))
			return false;
	}

	if (jsonDoc.HasMember("src_blend_alpha") && jsonDoc["src_blend_alpha"].IsString())
	{
		std::string srcBlend = jsonDoc["src_blend_alpha"].GetString();

		if (!this->TranslateBlend(srcBlend, blendDesc.RenderTarget[0].SrcBlendAlpha))
			return false;
	}

	if (!jsonDoc.HasMember("dest_blend_alpha") && jsonDoc["dest_blend_alpha"].IsString())
	{
		std::string destBlend = jsonDoc["dest_blend_alpha"].GetString();

		if (!this->TranslateBlend(destBlend, blendDesc.RenderTarget[0].DestBlendAlpha))
			return false;
	}

	if (!jsonDoc.HasMember("blend_op_alpha") && jsonDoc["blend_op_alpha"].IsString())
	{
		std::string blendOpStr = jsonDoc["blend_op_alpha"].GetString();

		if (!this->TranslateBlendOp(blendOpStr, blendDesc.RenderTarget[0].BlendOpAlpha))
			return false;
	}

	return true;
}

bool Shader::TranslateBlend(const std::string& blendStr, D3D12_BLEND& blend)
{
	if (blendStr == "BLEND_SRC_ALPHA")
		blend = D3D12_BLEND_SRC_ALPHA;
	else if (blendStr == "BLEND_INV_SRC_ALPHA")
		blend = D3D12_BLEND_INV_SRC_ALPHA;
	else if (blendStr == "BLEND_ONE")
		blend = D3D12_BLEND_ONE;
	else if (blendStr == "BLEND_ZERO")
		blend = D3D12_BLEND_ZERO;
	else
	{
		IMZADI_LOG_ERROR("Did not recognize blend: %s", blendStr.c_str());
		return false;
	}

	return true;
}

bool Shader::TranslateBlendOp(const std::string& blendOpStr, D3D12_BLEND_OP& blendOp)
{
	if (blendOpStr == "BLEND_OP_ADD")
		blendOp = D3D12_BLEND_OP_ADD;
	else
	{
		IMZADI_LOG_ERROR("Did not recognize blend: %s", blendOpStr.c_str());
		return false;
	}

	return true;
}

/*virtual*/ bool Shader::Unload()
{
	this->pipelineState.Reset();
	this->constantsBuffer.Reset();

	return true;
}