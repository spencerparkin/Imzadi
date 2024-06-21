#include "Shader.h"
#include "Game.h"
#include "Log.h"
#include <d3dcompiler.h>
#include <codecvt>
#include <locale>

using namespace Imzadi;

Shader::Shader()
{
	this->inputLayout = nullptr;
	this->pixelShader = nullptr;
	this->vertexShader = nullptr;
	this->vsBlob = nullptr;
	this->psBlob = nullptr;
	this->constantsBuffer = nullptr;
	this->constantsBufferSize = 0;
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

	HRESULT result = 0;

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

		result = D3DReadFileToBlob(vsShaderObjFileW.c_str(), &this->vsBlob);
		if (FAILED(result))
		{
			IMZADI_LOG_ERROR(std::format("Failed to read VS file blob ({}) with error code: {}", vsShaderObjFile.c_str(), result));
			return false;
		}

		result = D3DReadFileToBlob(psShaderObjFileW.c_str(), &this->psBlob);
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

		if (!this->CompileShader(shaderCodeFile, vsEntryPoint, vsModel, this->vsBlob))
		{
			IMZADI_LOG_ERROR("VS compilation failed.");
			return false;
		}

		if (!this->CompileShader(shaderCodeFile, psEntryPoint, psModel, this->psBlob))
		{
			IMZADI_LOG_ERROR("PS compilation failed.");
			return false;
		}
	}

	if (!this->vsBlob || !this->psBlob)
	{
		IMZADI_LOG_ERROR("Did not acquire both a VS and PS shader blob.");
		return false;
	}

	result = Game::Get()->GetDevice()->CreateVertexShader(this->vsBlob->GetBufferPointer(), this->vsBlob->GetBufferSize(), nullptr, &this->vertexShader);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("Failed to create VS with error code: {}", result));
		return false;
	}

	result = Game::Get()->GetDevice()->CreatePixelShader(this->psBlob->GetBufferPointer(), this->psBlob->GetBufferSize(), nullptr, &this->pixelShader);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("Failed to create PS with error code: {}", result));
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

	std::unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> inputElementDescArray(new D3D11_INPUT_ELEMENT_DESC[inputLayoutValue.Size()]);
	std::vector<std::string> semanticArray;
	semanticArray.reserve(inputLayoutValue.Size());
	if (!this->PopulateInputLayout(inputElementDescArray.get(), inputLayoutValue, semanticArray))
	{
		IMZADI_LOG_ERROR("Failed to populate VS input layout info.");
		return false;
	}

	result = Game::Get()->GetDevice()->CreateInputLayout(inputElementDescArray.get(), inputLayoutValue.Size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &this->inputLayout);
	if (FAILED(result))
	{
		IMZADI_LOG_ERROR(std::format("CreateInputLayout() call failed with error code: {}", result));
		return false;
	}

	this->vsBlob->Release();
	this->vsBlob = nullptr;

	this->psBlob->Release();
	this->psBlob = nullptr;

	if (jsonDoc.HasMember("constants") && jsonDoc["constants"].IsObject())
	{
		this->constantsBufferSize = 0;

		const rapidjson::Value& constantsValue = jsonDoc["constants"];
		for (auto iter = constantsValue.MemberBegin(); iter != constantsValue.MemberEnd(); ++iter)
		{
			const rapidjson::Value& constantsEntryName = iter->name;
			const rapidjson::Value& constantsEntryValue = iter->value;

			if (!constantsEntryValue.HasMember("offset") || !constantsEntryValue["offset"].IsInt())
			{
				IMZADI_LOG_ERROR("No \"offset\" member found or it's not an int.");
				return false;
			}

			if (!constantsEntryValue.HasMember("size") || !constantsEntryValue["size"].IsInt())
			{
				IMZADI_LOG_ERROR("No \"size\" member found or it's not an int.");
				return false;
			}

			if (!constantsEntryValue.HasMember("type") || !constantsEntryValue["type"].IsString())
			{
				IMZADI_LOG_ERROR("No \"type\" member found or it's not an int.");
				return false;
			}

			Constant constant;
			constant.offset = constantsEntryValue["offset"].GetInt();
			constant.size = constantsEntryValue["size"].GetInt();
			std::string type = constantsEntryValue["type"].GetString();
			if (type == "float")
				constant.format = DXGI_FORMAT_R32_FLOAT;
			else
			{
				IMZADI_LOG_ERROR(std::format("Did not recognize type \"{}\" or it is not yet supported.", type.c_str()));
				return false;
			}

			std::string name = constantsEntryName.GetString();
			this->constantsMap.insert(std::pair<std::string, Constant>(name, constant));

			if (this->constantsBufferSize < constant.offset + constant.size)
				this->constantsBufferSize = constant.offset + constant.size;
		}

		// The buffer size must be a multiple of 16.
		this->constantsBufferSize = Align16(this->constantsBufferSize);

		// Sanity check the data in the constants map.  Note that we don't
		// check for any overlap here, but we do check for proper bounds.
		for (auto pair : this->constantsMap)
		{
			const Constant& constant = pair.second;
			if (constant.size == 0)
			{
				IMZADI_LOG_ERROR("Shader constant was of size zero.");
				return false;
			}

			if (constant.offset >= this->constantsBufferSize)
			{
				IMZADI_LOG_ERROR(std::format("Shader constant offset ({}) is out of range ([0,{}]).", constant.offset, this->constantsBufferSize - 1));
				return false;
			}

			if (constant.offset + constant.size > this->constantsBufferSize)
			{
				IMZADI_LOG_ERROR(std::format("Shader constant at offset {} with size {} overflows the constant buffer size {}.", constant.offset, constant.size, this->constantsBufferSize));
				return false;
			}
			
			// Make sure the constant doesn't straddle a 16-byte boundary.
			UINT boundaryA = Align16(constant.offset);
			UINT boundaryB = Align16(constant.offset + constant.size);
			if (boundaryA != boundaryB && boundaryB < constant.offset + constant.size)
			{
				IMZADI_LOG_ERROR(std::format("Shader constant at offset {} with size {} straddles a 16-byte boundary.", constant.offset, constant.size));
				return false;
			}
		}

		if (this->constantsBufferSize > 0)
		{
			D3D11_BUFFER_DESC bufferDesc{};
			bufferDesc.ByteWidth = this->constantsBufferSize;
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			result = Game::Get()->GetDevice()->CreateBuffer(&bufferDesc, NULL, &this->constantsBuffer);
			if (FAILED(result))
			{
				IMZADI_LOG_ERROR(std::format("CreateBuffer() call failed with error code: {}", result));
				return false;
			}
		}
	}

	return true;
}

bool Shader::GetConstantInfo(const std::string& name, const Constant*& constant)
{
	ConstantsMap::iterator iter = this->constantsMap.find(name);
	if (iter == this->constantsMap.end())
		return false;

	constant = &iter->second;
	return true;
}

bool Shader::PopulateInputLayout(D3D11_INPUT_ELEMENT_DESC* inputLayoutArray, const rapidjson::Value& inputLayoutArrayValue, std::vector<std::string>& semanticArray)
{
	for (int i = 0; i < inputLayoutArrayValue.Size(); i++)
	{
		D3D11_INPUT_ELEMENT_DESC* inputElementDesc = &inputLayoutArray[i];
		::memset(inputElementDesc, 0, sizeof(D3D11_INPUT_ELEMENT_DESC));

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

		inputElementDesc->AlignedByteOffset = (i == 0) ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		inputElementDesc->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

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

	ID3DBlob* errorsBlob = nullptr;
	HRESULT result = D3DCompileFromFile(shaderFileW.c_str(), nullptr, nullptr, entryPoint.c_str(), shaderModel.c_str(), flags, 0, &blob, &errorsBlob);
	if (FAILED(result))
	{
		const char* errorMsg = "Unknown error!";
		if (errorsBlob)
			errorMsg = (const char*)errorsBlob->GetBufferPointer();
		else if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			errorMsg = "File not found!";

		IMZADI_LOG_ERROR(std::format("Shader compilation of file {} failed with error code {}, because: {}", shaderFile.c_str(), result, errorMsg));
		
		if (errorsBlob)
			errorsBlob->Release();

		return false;
	}

	return true;
}

/*virtual*/ bool Shader::Unload()
{
	SafeRelease(this->vsBlob);
	SafeRelease(this->psBlob);
	SafeRelease(this->vertexShader);
	SafeRelease(this->pixelShader);
	SafeRelease(this->inputLayout);
	SafeRelease(this->constantsBuffer);

	return true;
}