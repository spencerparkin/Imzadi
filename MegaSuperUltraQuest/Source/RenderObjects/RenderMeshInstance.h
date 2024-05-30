#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Math/Vector4.h"
#include "Scene.h"

class RenderMeshAsset;

/**
 * These are instances of a renderable mesh.  See the RenderMesh class.
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