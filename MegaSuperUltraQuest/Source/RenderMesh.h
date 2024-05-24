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
 * These are instances of a renderable mesh.  See the RenderMeshAsset class.
 */
class RenderMeshInstance : public RenderObject
{
public:
	RenderMeshInstance();
	virtual ~RenderMeshInstance();

	virtual void Render(Camera* camera, RenderPass renderPass) override;
	virtual void GetWorldBoundingSphere(Collision::Vector3& center, double& radius) const override;

	void SetRenderMesh(Reference<RenderMeshAsset> mesh) { this->mesh = mesh; }
	void SetBoundingBox(const Collision::AxisAlignedBoundingBox& boundingBox) { this->objectSpaceBoundingBox = boundingBox; }
	void SetObjectToWorldTransform(const Collision::Transform& objectToWorld) { this->objectToWorld = objectToWorld; }
	const Collision::Transform& GetObjectToWorldTransform() const { return this->objectToWorld; }

	/**
	 * These parameters are used in the lighting calculations of the surface of the mesh.
	 */
	struct SurfaceProperties
	{
		//double roughnessFactor;
		double shininessExponent;
	};

	const SurfaceProperties& GetSurfaceProperties() const { return this->surfaceProperties; }
	void SetSurfaceProperties(const SurfaceProperties& surfaceProperties) { this->surfaceProperties = surfaceProperties; }

private:
	Reference<RenderMeshAsset> mesh;
	Collision::AxisAlignedBoundingBox objectSpaceBoundingBox;
	Collision::Transform objectToWorld;
	Collision::Vector4 color;
	SurfaceProperties surfaceProperties;
};

/**
 * This is everything that defines a renderable mesh without the
 * particulars of an instance of such.
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
	Shader* GetShader() { return this->mainPassShader.Get(); }
	Shader* GetShadowShader() { return this->shadowPassShader.Get(); }
	Buffer* GetVertexBuffer() { return this->vertexBuffer.Get(); }
	Buffer* GetIndexBuffer() { return this->indexBuffer.Get(); }
	Texture* GetTexture() { return this->texture.Get(); }

private:
	D3D_PRIMITIVE_TOPOLOGY primType;
	Reference<Buffer> vertexBuffer;
	Reference<Buffer> indexBuffer;
	Reference<Shader> mainPassShader;
	Reference<Shader> shadowPassShader;
	Reference<Texture> texture;
	Collision::AxisAlignedBoundingBox objectSpaceBoundingBox;
};