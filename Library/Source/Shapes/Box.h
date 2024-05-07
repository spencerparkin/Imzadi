#pragma once

#include "Shape.h"
#include "Math/Vector3.h"

namespace Collision
{
	class PolygonShape;

	/**
	 * This collision shape is simply a box with a given width, height and depth.
	 * In object-space, the box is centered at origin having a width in the x-dimension,
	 * height in the y-dimension, and depth in the z-dimension.
	 */
	class COLLISION_LIB_API BoxShape : public Shape
	{
	public:
		BoxShape(bool temporary);
		virtual ~BoxShape();

		typedef Vector3 BoxVertexMatrix[2][2][2];

		/**
		 * See Shape::GetShapeTypeID.
		 */
		virtual TypeID GetShapeTypeID() const override;

		/**
		 * Return what we do in GetShapeTypeID().
		 */
		static TypeID StaticTypeID();

		/**
		 * See Shape::RecalculateCache.
		 */
		virtual void RecalculateCache() const override;

		/**
		 * Tell the caller if this box is valid.  A valid box
		 * will have non-zero, positive extents.
		 * 
		 * @return True is returned if valid; false, otherwise.
		 */
		virtual bool IsValid() const override;

		/**
		 * Calculate and return the volume of this box.
		 */
		virtual double CalcSize() const override;

		/**
		 * Tell the caller if the given point is contained within or on the surface of this box.
		 */
		virtual bool ContainsPoint(const Vector3& point) const override;

		/**
		 * Render this box as wire-frame in the given result.
		 */
		virtual void DebugRender(DebugRenderResult* renderResult) const override;

		/**
		 * Perform a ray-cast against this box.
		 * 
		 * @param[in] ray This is the ray to use in the ray-cast.
		 * @param[out] alpha This is the distance from the ray origin along the ray-direction to the point where this box is hit, if any.
		 * @param[out] unitSurfaceNormal This is the surface normal of the box at the ray hit-point, if any.
		 */
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const override;

		/**
		 * Write this box to given stream in binary form.
		 */
		virtual bool Dump(std::ostream& stream) const override;

		/**
		 * Read this box from the given stream in binary form.
		 */
		virtual bool Restore(std::istream& stream) override;

		/**
		 * Allocate and return a new BoxShape class instance.
		 */
		static BoxShape* Create();

		/**
		 * Set this box's half-width, half-height, and half-depth.  You can think of the
		 * extent vector as pointing from the center of the box to one of its corners.
		 * All components of the given vector should be non-zero and non-negative.
		 * 
		 * @param[in] extents Here, the x-component is the half-size of the box in the x-dimension while in object space.  The same goes for the other components.
		 */
		void SetExtents(const Vector3& extents) { this->extents = extents; }

		/**
		 * Get this box's half-width, half-height, and half-depth.
		 * 
		 * @return See the description for SetExtents.
		 */
		const Vector3& GetExtents() const { return this->extents; }

		/**
		 * Return the AABB that represents this box in object-space.
		 * 
		 * @param[out] box This box is returned in the given AABB.
		 */
		void GetAxisAlignedBox(AxisAlignedBoundingBox& box) const;

		/**
		 * Calculate and return the corner points of this box in a 2x2x2 matrix.
		 * 
		 * @param[out] boxVertices This is expected to be a 2x2x2 matrix of vectors.  If it's not, we stomp memory!
		 * @param[in] worldSpace If true, the world-space corners are calculated; the object-space corners, otherwise.
		 */
		void GetCornerMatrix(BoxVertexMatrix& boxVertices, bool worldSpace) const;

		/**
		 * Calculate and return the corner points of this box.
		 * 
		 * @param[out] cornerPointArray This is populated with this box's corners.
		 * @param[in] worldSpace If true, the world-space corners are calculated; the object-space corners, otherwise.
		 */
		void GetCornerPointArray(std::vector<Vector3>& cornerPointArray, bool worldSpace) const;

		/**
		 * Calculate and return the line segments forming the edges of this box.
		 * 
		 * @param[out] edgeSegmentArray This is populated with the box's edges.
		 * @param[in] worldSpace If true, the world-space edges are calculated; the object-space corners, otherwise.
		 */
		void GetEdgeSegmentArray(std::vector<LineSegment>& edgeSegmentArray, bool worldSpace) const;

		/**
		 * Calculate and return the polygons forming the faces (rectangles) of this box.
		 * 
		 * @param[out] facePolygonArray This is populated with the box's faces.
		 * @param[in] worldSpace If true, the world-space faces are calculated; the object-space faces, otherwise.
		 */
		void GetFacePolygonArray(std::vector<PolygonShape>& facePolygonArray, bool worldSpace) const;

	private:
		Vector3 extents;
	};
}