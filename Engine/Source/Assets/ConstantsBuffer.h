#pragma once

#include "Buffer.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"

namespace Imzadi
{
	/**
	 * This buffer is designed to hold shader constants.  The constants
	 * are setup with every draw-call, but they are constant in the sense
	 * that they don't change for the life of the shader program in that
	 * draw call.
	 */
	class IMZADI_API ConstantsBuffer : public Buffer
	{
	public:
		ConstantsBuffer();
		virtual ~ConstantsBuffer();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;

		bool StoreShaderConstant(const std::string& name, const double* scalar);
		bool StoreShaderConstant(const std::string& name, const Vector2* vector);
		bool StoreShaderConstant(const std::string& name, const Vector3* vector);
		bool StoreShaderConstant(const std::string& name, const Vector4* vector);
		bool StoreShaderConstant(const std::string& name, const Matrix3x3* matrix);
		bool StoreShaderConstant(const std::string& name, const Matrix4x4* matrix);

	private:
		struct Constant
		{
			UINT offset;
			UINT size;
			DXGI_FORMAT format;
		};

		const Constant* LookupConstant(const std::string& name);

		typedef std::unordered_map<std::string, Constant> ConstantsMap;
		ConstantsMap constantsMap;
	};
}