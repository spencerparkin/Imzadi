#include "Shader.h"
#include "Game.h"
#include <d3dcompiler.h>
#include <codecvt>
#include <locale>

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
		return false;

	HRESULT result = 0;

	if (jsonDoc.HasMember("vs_shader_object") && jsonDoc.HasMember("ps_shader_object"))
	{
		if (!jsonDoc["vs_shader_object"].IsString() || !jsonDoc["ps_shader_object"].IsString())
			return false;

		std::string vsShaderObjFile = jsonDoc["vs_shader_object"].GetString();
		std::string psShaderObjFile = jsonDoc["ps_shader_object"].GetString();

		if (!assetCache->ResolveAssetPath(vsShaderObjFile))
			return false;

		if (!assetCache->ResolveAssetPath(psShaderObjFile))
			return false;

		std::wstring vsShaderObjFileW = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(vsShaderObjFile);
		std::wstring psShaderObjFileW = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(psShaderObjFile);

		result = D3DReadFileToBlob(vsShaderObjFileW.c_str(), &this->vsBlob);
		if (FAILED(result))
			return false;

		result = D3DReadFileToBlob(psShaderObjFileW.c_str(), &this->psBlob);
		if (FAILED(result))
			return false;
	}
	else if (jsonDoc.HasMember("shader_code"))
	{
		if (!jsonDoc["shader_code"].IsString())
			return false;

		std::string shaderCodeFile = jsonDoc["shader_code"].GetString();
		if (!assetCache->ResolveAssetPath(shaderCodeFile))
			return false;

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
			return false;

		if (!this->CompileShader(shaderCodeFile, psEntryPoint, psModel, this->psBlob))
			return false;
	}

	if (!this->vsBlob || !this->psBlob)
		return false;

	result = Game::Get()->GetDevice()->CreateVertexShader(this->vsBlob->GetBufferPointer(), this->vsBlob->GetBufferSize(), nullptr, &this->vertexShader);
	if (FAILED(result))
		return false;

	result = Game::Get()->GetDevice()->CreatePixelShader(this->psBlob->GetBufferPointer(), this->psBlob->GetBufferSize(), nullptr, &this->pixelShader);
	if (FAILED(result))
		return false;

	if (!jsonDoc.HasMember("vs_input_layout"))
		return false;

	const rapidjson::Value& inputLayoutValue = jsonDoc["vs_input_layout"];
	if (!inputLayoutValue.IsArray() || inputLayoutValue.Size() == 0)
		return false;

	std::unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> inputElementDescArray(new D3D11_INPUT_ELEMENT_DESC[inputLayoutValue.Size()]);
	std::vector<std::string> semanticArray;
	semanticArray.reserve(inputLayoutValue.Size());
	if (!this->PopulateInputLayout(inputElementDescArray.get(), inputLayoutValue, semanticArray))
		return false;

	result = Game::Get()->GetDevice()->CreateInputLayout(inputElementDescArray.get(), inputLayoutValue.Size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &this->inputLayout);
	if (FAILED(result))
		return false;

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
				return false;

			if (!constantsEntryValue.HasMember("size") || !constantsEntryValue["size"].IsInt())
				return false;

			if (!constantsEntryValue.HasMember("type") || !constantsEntryValue["type"].IsString())
				return false;

			Constant constant;
			constant.offset = constantsEntryValue["offset"].GetInt();
			constant.size = constantsEntryValue["size"].GetInt();
			std::string type = constantsEntryValue["type"].GetString();
			if (type == "float")
				constant.format = DXGI_FORMAT_R32_FLOAT;
			else
				return false;

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
				return false;
			if (constant.offset >= this->constantsBufferSize)
				return false;
			if (constant.offset + constant.size > this->constantsBufferSize)
				return false;
			
			// Make sure the constant doesn't straddle a 16-byte boundary.
			UINT boundaryA = Align16(constant.offset);
			UINT boundaryB = Align16(constant.offset + constant.size);
			if (boundaryA != boundaryB && boundaryB < constant.offset + constant.size)
				return false;
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
				return false;
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
			return false;

		if (!inputLayoutElementValue.HasMember("semantic") || !inputLayoutElementValue["semantic"].IsString())
			return false;

		std::string semantic = inputLayoutElementValue["semantic"].GetString();
		semanticArray.push_back(semantic);
		inputElementDesc->SemanticName = semanticArray[semanticArray.size() - 1].c_str();

		if (inputLayoutElementValue.HasMember("semantic_index") && inputLayoutElementValue["semantic_index"].IsInt())
			inputElementDesc->SemanticIndex = inputLayoutElementValue["semantic_index"].GetInt();

		if (!inputLayoutElementValue.HasMember("element_format") || !inputLayoutElementValue["element_format"].IsString())
			return false;

		std::string format = inputLayoutElementValue["element_format"].GetString();
		if (format == "R32G32_FLOAT")
			inputElementDesc->Format = DXGI_FORMAT_R32G32_FLOAT;
		else if (format == "R32G32B32_FLOAT")
			inputElementDesc->Format = DXGI_FORMAT_R32G32B32_FLOAT;
		else if (format == "R32G32B32A32_FLOAT")
			inputElementDesc->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		else
			return false;

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

		MessageBoxA(Game::Get()->GetMainWindowHandle(), errorMsg, "Shader Compile Error!", MB_OK | MB_ICONERROR);
		
		if (errorsBlob)
			errorsBlob->Release();

		return false;
	}

	return true;
}

/*virtual*/ bool Shader::Unload()
{
	if (this->vsBlob)
	{
		this->vsBlob->Release();
		this->vsBlob = nullptr;
	}

	if (this->psBlob)
	{
		this->psBlob->Release();
		this->psBlob = nullptr;
	}

	if (this->vertexShader)
	{
		this->vertexShader->Release();
		this->vertexShader = nullptr;
	}

	if (this->pixelShader)
	{
		this->pixelShader->Release();
		this->pixelShader = nullptr;
	}

	if (this->inputLayout)
	{
		this->inputLayout->Release();
		this->inputLayout = nullptr;
	}

	if (this->constantsBuffer)
	{
		this->constantsBuffer->Release();
		this->constantsBuffer = nullptr;
	}

	return false;
}