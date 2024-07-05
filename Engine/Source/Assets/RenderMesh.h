#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Scene.h"
#include "AssetCache.h"
#include "Shader.h"
#include "Texture.h"
#include "Buffer.h"
#include <d3dcommon.h>

namespace Imzadi
{
	/**
	 * This is everything that defines a renderable mesh without the
	 * particulars of an instance of such.
	 */
	class IMZADI_API RenderMeshAsset : public Asset
	{
	public:
		RenderMeshAsset();
		virtual ~RenderMeshAsset();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Unload() override;
		virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) override;

		D3D_PRIMITIVE_TOPOLOGY GetPrimType() const { return this->primType; }
		Shader* GetShader() { return this->mainPassShader.Get(); }
		Shader* GetShadowShader() { return this->shadowPassShader.Get(); }
		Buffer* GetVertexBuffer() { return this->vertexBuffer.Get(); }
		Buffer* GetIndexBuffer() { return this->indexBuffer.Get(); }
		Texture* GetTexture() { return this->texture.Get(); }
		const Transform& GetObjectToWorldTransform() const { return this->objectToWorld; }

	protected:
		D3D_PRIMITIVE_TOPOLOGY primType;
		Reference<Buffer> vertexBuffer;
		Reference<Buffer> indexBuffer;
		Reference<Shader> mainPassShader;
		Reference<Shader> shadowPassShader;
		Reference<Texture> texture;
		AxisAlignedBoundingBox objectSpaceBoundingBox;
		Transform objectToWorld;
	};
}