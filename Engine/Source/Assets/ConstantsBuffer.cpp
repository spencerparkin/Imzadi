#include "ConstantsBuffer.h"
#include "Log.h"

using namespace Imzadi;

ConstantsBuffer::ConstantsBuffer()
{
}

/*virtual*/ ConstantsBuffer::~ConstantsBuffer()
{
}

/*virtual*/ bool ConstantsBuffer::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!Buffer::Load(jsonDoc, assetCache))
		return false;

	if (this->bufferType != Type::DYNAMIC_SMALL)
	{
		IMZADI_LOG_ERROR("Constants buffers must always be designated dynamic-small.");
		return false;
	}

	if (this->bufferSize % 256 != 0)
	{
		IMZADI_LOG_ERROR("Buffer size must be a multiple of 256.");
		return false;
	}

	if (!jsonDoc.HasMember("constants") || !jsonDoc["constants"].IsObject())
	{
		IMZADI_LOG_ERROR("Constants buffers must have a \"constants\" entry.");
		return false;
	}

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

		if (this->bufferSize < constant.offset + constant.size)
			this->bufferSize = constant.offset + constant.size;
	}

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

		if (constant.offset >= this->bufferSize)
		{
			IMZADI_LOG_ERROR(std::format("Shader constant offset ({}) is out of range ([0,{}]).", constant.offset, this->bufferSize - 1));
			return false;
		}

		if (constant.offset + constant.size > this->bufferSize)
		{
			IMZADI_LOG_ERROR(std::format("Shader constant at offset {} with size {} overflows the constant buffer size {}.", constant.offset, constant.size, this->bufferSize));
			return false;
		}

		// Make sure the constant doesn't straddle a 16-byte boundary.
		UINT boundaryA = Align(constant.offset, 16);
		UINT boundaryB = Align(constant.offset + constant.size, 16);
		if (boundaryA != boundaryB && boundaryB < constant.offset + constant.size)
		{
			IMZADI_LOG_ERROR(std::format("Shader constant at offset {} with size {} straddles a 16-byte boundary.", constant.offset, constant.size));
			return false;
		}
	}

	return true;
}

const ConstantsBuffer::Constant* ConstantsBuffer::LookupConstant(const std::string& name)
{
	ConstantsMap::iterator iter = this->constantsMap.find(name);
	if (iter == this->constantsMap.end())
		return nullptr;

	return &iter->second;
}

bool ConstantsBuffer::StoreShaderConstant(const std::string& name, const double* scalar)
{
	const Constant* constant = this->LookupConstant(name);
	if (!constant)
		return false;

	*(float*)&(this->mappedBufferData)[constant->offset] = float(*scalar);
	return true;
}

bool ConstantsBuffer::StoreShaderConstant(const std::string& name, const Vector2* vector)
{
	const Constant* constant = this->LookupConstant(name);
	if (!constant)
		return false;

	float* floatArray = (float*)&(this->mappedBufferData)[constant->offset];
	floatArray[0] = vector->x;
	floatArray[1] = vector->y;
	return true;
}

bool ConstantsBuffer::StoreShaderConstant(const std::string& name, const Vector3* vector)
{
	const Constant* constant = this->LookupConstant(name);
	if (!constant)
		return false;

	float* floatArray = (float*)&(this->mappedBufferData)[constant->offset];
	floatArray[0] = vector->x;
	floatArray[1] = vector->y;
	floatArray[2] = vector->z;
	return true;
}

bool ConstantsBuffer::StoreShaderConstant(const std::string& name, const Vector4* vector)
{
	const Constant* constant = this->LookupConstant(name);
	if (!constant)
		return false;

	float* floatArray = (float*)&((uint8_t*)this->mappedBufferData)[constant->offset];
	floatArray[0] = vector->x;
	floatArray[1] = vector->y;
	floatArray[2] = vector->z;
	floatArray[3] = vector->w;
	return true;
}

bool ConstantsBuffer::StoreShaderConstant(const std::string& name, const Matrix3x3* matrix)
{
	const Constant* constant = this->LookupConstant(name);
	if (!constant)
		return false;

	float* ele = (float*)&(this->mappedBufferData)[constant->offset];
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			*ele++ = float(matrix->ele[j][i]);	// Note the swap of i and j here!

	return true;
}

bool ConstantsBuffer::StoreShaderConstant(const std::string& name, const Matrix4x4* matrix)
{
	const Constant* constant = this->LookupConstant(name);
	if (!constant)
		return false;

	float* ele = (float*)&(this->mappedBufferData)[constant->offset];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			*ele++ = float(matrix->ele[j][i]);	// Note the swap of i and j here!

	return true;
}