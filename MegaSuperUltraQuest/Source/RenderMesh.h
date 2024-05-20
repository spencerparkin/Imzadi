#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Scene.h"
#include "AssetCache.h"
#include "Shader.h"
#include "Texture.h"
#include "Buffer.h"
#include <d3dcommon.h>

class RenderMeshAsset;

// TODO: Add texture mapping.
// TODO: Add lighting.
// TODO: A bit ambitious, but what about shadows?  This requires a render pass of the
//       scene from the perspective of a light source to capture a depth-buffer which
//       is then used in the main render pass.

/**
 * 
 */
class RenderMeshInstance : public RenderObject
{
public:
	RenderMeshInstance();
	virtual ~RenderMeshInstance();

	virtual void Render(Camera* camera) override;
	virtual void GetWorldBoundingSphere(Collision::Vector3& center, double& radius) const override;

	void SetRenderMesh(Reference<RenderMeshAsset> mesh) { this->mesh = mesh; }
	void SetBoundingBox(const Collision::AxisAlignedBoundingBox& boundingBox) { this->objectSpaceBoundingBox = boundingBox; }
	void SetObjectToWorldTransform(const Collision::Transform& objectToWorld) { this->objectToWorld = objectToWorld; }

	const Collision::Vector4& GetColor() const { return this->color; }
	void SetColor(const Collision::Vector4& color) { this->color = color; }

private:
	Reference<RenderMeshAsset> mesh;
	Collision::AxisAlignedBoundingBox objectSpaceBoundingBox;
	Collision::Transform objectToWorld;
	Collision::Vector4 color;
};

/**
 * 
 */
class RenderMeshAsset : public Asset
{
public:
	RenderMeshAsset();
	virtual ~RenderMeshAsset();

	virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
	virtual bool Unload() override;
	virtual bool MakeRenderInstance(Reference<RenderObject>& renderObject) override;

	D3D_PRIMITIVE_TOPOLOGY GetPrimType() const { return this->primType; }
	Shader* GetShader() { return this->shader.Get(); }
	Buffer* GetVertexBuffer() { return this->vertexBuffer.Get(); }
	Buffer* GetIndexBuffer() { return this->indexBuffer.Get(); }
	Texture* GetTexture() { return this->texture.Get(); }

private:
	D3D_PRIMITIVE_TOPOLOGY primType;
	Reference<Buffer> vertexBuffer;
	Reference<Buffer> indexBuffer;
	Reference<Shader> shader;
	Reference<Texture> texture;
	Collision::AxisAlignedBoundingBox objectSpaceBoundingBox;
};