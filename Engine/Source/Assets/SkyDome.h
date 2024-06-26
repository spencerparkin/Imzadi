#pragma once

#include "RenderMesh.h"
#include "CubeTexture.h"

namespace Imzadi
{
	class IMZADI_API SkyDome : public RenderMeshAsset
	{
	public:
		SkyDome();
		virtual ~SkyDome();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;
		virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) override;

		CubeTexture* GetCubeTexture() { return this->cubeTexture.Get(); }

	private:
		Reference<CubeTexture> cubeTexture;
	};
}