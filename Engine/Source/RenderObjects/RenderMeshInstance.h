#pragma once

#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Transform.h"
#include "Math/Vector4.h"
#include "Scene.h"
#include <map>

namespace Imzadi
{
	class RenderMeshAsset;

	/**
	 * These are instances of a renderable mesh.  See the RenderMesh class.
	 */
	class IMZADI_API RenderMeshInstance : public RenderObject
	{
	public:
		RenderMeshInstance();
		virtual ~RenderMeshInstance();

		virtual void Render(Camera* camera, RenderPass renderPass) override;
		virtual void GetWorldBoundingSphere(Vector3& center, double& radius) const override;
		virtual void PreRender() override;

		void SetRenderMesh(Reference<RenderMeshAsset> mesh, int lodNumber = 0);
		RenderMeshAsset* GetRenderMesh(int lodNumber = 0);
		void SetBoundingBox(const AxisAlignedBoundingBox& boundingBox) { this->objectSpaceBoundingBox = boundingBox; }
		void SetObjectToWorldTransform(const Transform& objectToWorld) { this->objectToWorld = objectToWorld; }
		const Transform& GetObjectToWorldTransform() const { return this->objectToWorld; }
		void SetDrawPorts(bool drawPorts) { this->drawPorts = drawPorts; }
		bool GetDrawPorts() const { return this->drawPorts; }

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

	protected:
		std::map<int, Reference<RenderMeshAsset>> meshMap;
		AxisAlignedBoundingBox objectSpaceBoundingBox;
		Transform objectToWorld;
		SurfaceProperties surfaceProperties;
		bool drawPorts;
	};
}