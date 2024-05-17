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

/**
 * 
 */
class RenderMeshInstance : public RenderObject
{
public:
	RenderMeshInstance();
	virtual ~RenderMeshInstance();

	virtual void Render(Scene* scene) override;
	virtual void GetWorldBoundingSphere(Collision::Vector3& center, double& radius) const override;

	void SetRenderMesh(Reference<RenderMeshAsset> mesh) { this->mesh = mesh; }
	void SetBoundingBox(const Collision::AxisAlignedBoundingBox& boundingBox) { this->objectSpaceBoundingBox = boundingBox; }

private:
	Reference<RenderMeshAsset> mesh;
	Collision::AxisAlignedBoundingBox objectSpaceBoundingBox;
	Collision::Transform objectToWorld;
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

private:
	D3D_PRIMITIVE_TOPOLOGY primType;
	Reference<Buffer> vertexBuffer;
	Reference<Buffer> indexBuffer;
	Reference<Shader> shader;
	Reference<Texture> texture;
	Collision::AxisAlignedBoundingBox objectSpaceBoundingBox;
};