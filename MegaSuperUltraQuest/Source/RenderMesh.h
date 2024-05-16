#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Scene.h"
#include "AssetCache.h"
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
	virtual Collision::AxisAlignedBoundingBox GetWorldBoundingBox() const override;

	void SetRenderMesh(std::shared_ptr<Asset> mesh) { this->mesh = mesh; }

private:
	std::shared_ptr<Asset> mesh;
	Collision::AxisAlignedBoundingBox boundingBox;
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
	virtual bool MakeRenderInstance(std::shared_ptr<RenderObject>& renderObject) override;

private:
	D3D_PRIMITIVE_TOPOLOGY primType;
	std::shared_ptr<Asset> vertexBuffer;
	std::shared_ptr<Asset> indexBuffer;
	std::shared_ptr<Asset> shader;
	std::shared_ptr<Asset> texture;
};