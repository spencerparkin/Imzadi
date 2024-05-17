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
}

/*virtual*/ Shader::~Shader()
{
}

/*virtual*/ bool Shader::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
		return false;

	if (!jsonDoc.HasMember("shader_code") || !jsonDoc["shader_code"].IsString())
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

	if (!this->CompileShader(shaderCodeFile, vsEntryPoint, vsModel, vsBlob))
		return false;

	HRESULT result = Game::Get()->GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &this->vertexShader);
	if (FAILED(result))
		return false;

	if (!this->CompileShader(shaderCodeFile, psEntryPoint, psModel, psBlob))
		return false;

	result = Game::Get()->GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &this->pixelShader);
	if (FAILED(result))
		return false;

	if (!jsonDoc.HasMember("vs_input_layout"))
		return false;

	const rapidjson::Value& inputLayoutValue = jsonDoc["vs_input_layout"];
	if (!inputLayoutValue.IsArray() || inputLayoutValue.Size() == 0)
		return false;

	std::unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> inputElementDescArray(new D3D11_INPUT_ELEMENT_DESC[inputLayoutValue.Size()]);
	std::vector<std::string> semanticArray;
	if (!this->PopulateInputLayout(inputElementDescArray.get(), inputLayoutValue, semanticArray))
		return false;

	result = Game::Get()->GetDevice()->CreateInputLayout(inputElementDescArray.get(), inputLayoutValue.Size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &this->inputLayout);
	if (FAILED(result))
		return false;

	this->vsBlob->Release();
	this->vsBlob = nullptr;

	this->psBlob->Release();
	this->psBlob = nullptr;

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

	ID3DBlob* errorsBlob = nullptr;
	HRESULT result = D3DCompileFromFile(shaderFileW.c_str(), nullptr, nullptr, entryPoint.c_str(), shaderModel.c_str(), 0, 0, &blob, &errorsBlob);
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

	return false;
}