#pragma once

#include "Shape.h"
#include "Math/Vector3.h"
#include "Math/Plane.h"
#include <vector>

namespace Collision
{
	/**
	 * This collision shape is a polygon determined by a sequence of points.
	 * All points must be coplanar, and they must form a convex polygon.
	 * If these constraints are not satisfied, then the results of using
	 * this shape, in most cases, are all left undefined.  The winding of the polygon
	 * is used to determine the "inside" and "outside" space of the polygon.
	 * This is necessary for the penetration depth of a collision query
	 * result to make any sense.  If viewing the polygon from a perspective
	 * where the vertices are ordered counter-clock-wise, then the viewer
	 * is in the "outside" space, and the "inside" space is on the other
	 * side of the polygon.
	 * 
	 * Note that the point-sequence is considered cyclical.  Modular arithematic
	 * is often used to index into this sequence.
	 */
	class COLLISION_LIB_API PolygonShape : public Shape
	{
	public:
		PolygonShape();
		virtual ~PolygonShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void CalcBoundingBox(AxisAlignedBoundingBox& boundingBox) const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;

		/**
		 * Remove all vertices from this polygon.  Note that the
		 * vacuous case is considered invalid.
		 */
		void Clear();

		/**
		 * Append a vertex to this shape's internal list (or sequence) of vertices.
		 * This always grows the size of the sequence.
		 * 
		 * @param[in] point This is the vertex point to be added.
		 */
		void AddVertex(const Vector3& point);

		/**
		 * Resize the internal vertex array to the given size.
		 * Note that this is potentially a destructive operation.
		 * Once done, the caller should set all vertices in the
		 * newly sized polygon.
		 * 
		 * @param[in] vertexCount This will be the number of stored vertices.
		 */
		void SetNumVertices(uint32_t vertexCount);

		/**
		 * Assign the given point as the vertex at position i in the stored sequence of such.
		 * 
		 * @param[in] i This is the vertex position to use.  It is taken mod N, where N is the size of the vertex sequence, before it is used as an index.  Negative integers are fine.
		 * @param[in] point This point will be assigned as the vertex at position i.
		 */
		void SetVertex(int i, const Vector3& point);

		/**
		 * Get the point at vertex position i in the stored sequence of such.
		 * 
		 * @param[in] i This is the vertex position to use.  It is used the same was as that described in SetVertex.
		 * @return The vertex point at position i is returned.
		 */
		const Vector3& GetVertex(int i) const;

		/**
		 * Return the plane containing this polygon.  Of course, there
		 * is no SetPlane call, because this plane is determined by this
		 * polygon's set of vertices.  Note that the returned result is
		 * cached so that subsequence calls to this function are faster.
		 * 
		 * @return The polygon's plane is returned.  Its normal will point toward the "outside" space of this polygon.
		 */
		const Plane& GetPlane() const;

		/**
		 * Calculate and return the average of all this polygon's vertices.
		 * This is always a point of the polygon if the polygon is valid.
		 */
		Vector3 GetCenter() const;

		/**
		 * Calculate and return a plane that best fits this polygon's sequence vertices.
		 * If this polygon is valid, then there is one and only one such plane.
		 * 
		 * @return The best fit plane is calcualted and returned using a least-squares method.
		 */
		Plane CalculatePlaneOfBestFit() const;

		/**
		 * Orthogonally project all vertices of this polygon onto the given plane.
		 * This might be done using the result of the CalculatePlaneOfBestFit function.
		 * 
		 * @param[in] plane This is the plane to use for the snapping process.
		 */
		void SnapToPlane(const Plane& plane);

		/**
		 * Set this polygon as the convex hull of the given set of points.
		 * A plane of best fit is found for the given point-cloud, and then
		 * the cloud is snapped to this plane, before the convex hull is computed.
		 * 
		 * @param[in] pointCloud This is the set of points to use in the operation.
		 */
		void SetAsConvexHull(const std::vector<Vector3>& pointCloud);

	private:
		int ModIndex(int i) const;

		std::vector<Vector3>* vertexArray;
		mutable Plane cachedPlane;
		mutable bool cachedPlaneValid;
	};
}