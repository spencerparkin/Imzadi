#pragma once

#include "Shape.h"
#include "Math/Vector3.h"
#include "Math/Plane.h"
#include <vector>

namespace Collision
{
	/**
	 * This collision shape is a polygon determined by a sequence of object-space points
	 * and an object-to-world transform.  All points must be coplanar, and they must form
	 * a convex polygon.  If these constraints are not satisfied, then the results of using
	 * this shape for collision purposes are all left undefined.  (Intermediate uses of
	 * this class may contain point sequences that do not necessarily satisfy these constraints.)
	 * The winding of the polygon will not matter as far as collision detection is concerned.
	 * However, the correctness of many operations requires that we consider the "front" space of
	 * the polygon to be where, if you viewed the polygon from this place, you'd see its points
	 * wound CCW.  The "back" space is where you would view the polygon's points wound clock-wise.
	 * A "thickness" of the polygon is necessary for some calculations.
	 * 
	 * Note that the point-sequence is considered cyclical.  Modular arithematic
	 * is often used to index into this sequence.  The polygon in world space
	 * is found by applying an object-to-world transform to the point-sequence.
	 */
	class COLLISION_LIB_API PolygonShape : public Shape
	{
	public:
		PolygonShape(bool temporary);
		virtual ~PolygonShape();

		virtual TypeID GetShapeTypeID() const override;
		virtual void RecalculateCache() const override;
		virtual bool IsValid() const override;
		virtual double CalcSize() const override;
		virtual bool ContainsPoint(const Vector3& point) const override;
		virtual void DebugRender(DebugRenderResult* renderResult) const override;
		virtual bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const override;

		/**
		 * Allocate and return a new PolygonShape class instance.
		 */
		static PolygonShape* Create();

		/**
		 * Remove all object-space vertices from this polygon.  Note that the
		 * vacuous case is considered invalid.
		 */
		void Clear();

		/**
		 * Append an object-space vertex to this shape's internal list (or sequence) of vertices.
		 * This always grows the size of the sequence.
		 * 
		 * @param[in] point This is the vertex point to be added.
		 */
		void AddVertex(const Vector3& point);

		/**
		 * Resize the internal object-space vertex array to the given size.
		 * Note that this is potentially a destructive operation.
		 * Once done, the caller should set all vertices in the
		 * newly sized polygon.
		 * 
		 * @param[in] vertexCount This will be the number of stored vertices.
		 */
		void SetNumVertices(uint32_t vertexCount);

		/**
		 * Assign the given object-space point as the vertex at position i in the stored sequence of such.
		 * 
		 * @param[in] i This is the vertex position to use.  It is taken mod N, where N is the size of the vertex sequence, before it is used as an index.  Negative integers are fine.
		 * @param[in] point This point in object space will be assigned as the vertex at position i.
		 */
		void SetVertex(int i, const Vector3& point);

		/**
		 * Get the object-space point at vertex position i in the stored sequence of such.
		 * 
		 * @param[in] i This is the vertex position to use.  It is used the same was as that described in SetVertex.
		 * @return The object-space vertex point at position i is returned.
		 */
		const Vector3& GetVertex(int i) const;

		/**
		 * Return the object-space plane containing this polygon.  Of course, there
		 * is no corresponding set call, because this plane is determined by this
		 * polygon's set of vertices.  Note that the returned result is
		 * cached so that subsequence calls to this function are faster.
		 * 
		 * @return The polygon's plane is returned in object-space.  Its normal will point toward the "front" space of this polygon.
		 */
		const Plane& GetPlane() const;

		/**
		 * Calculate and return the average of all this polygon's object-space vertices.
		 * This is always a point of the polygon in object-space if the polygon is valid.
		 */
		Vector3 GetCenter() const;

		/**
		 * Calculate and return a plane in object space that best fits this polygon's set of vertices,
		 * which are not assumed to be a valid polygon.  They can be any cloud of points in 3D space.
		 * If, however, this polygon is valid, then there is one and only one such plane, up to sign.
		 * 
		 * @param[out] plane The best fit object-space plane is calcualted and returned using a linear least-squares method.
		 * @return True is returned if successful; false, otherwise.
		 */
		bool CalculatePlaneOfBestFit(Plane& plane) const;

		/**
		 * Orthogonally project all vertices of this polygon onto the given object-space plane.
		 * This might be done using the result of the CalculatePlaneOfBestFit function.
		 * 
		 * @param[in] plane This is the plane, in object space, to use for the snapping process.
		 */
		void SnapToPlane(const Plane& plane);

		/**
		 * Set this polygon as the convex hull of the given set of object-space points.
		 * A plane of best fit is found for the given point-cloud, and then
		 * the cloud is snapped to this plane, before the convex hull is computed.
		 * 
		 * @param[in] pointCloud This is the set of points, in object-space, to use in the operation.
		 */
		void SetAsConvexHull(const std::vector<Vector3>& pointCloud);

		/**
		 * Return the vertices of this polygon (in CCW order) transformed into world space.
		 * 
		 * @param[out] worldVertexArray This array is populated with this polygon's transformed vertices.
		 */
		void GetWorldVertices(std::vector<Vector3>& worldVertexArray) const;

	private:
		/**
		 * Return the given index (or offset) mod N, where N is the number of vertices in this polygon.
		 * The returned index will always be non-negative.
		 */
		int ModIndex(int i) const;

		/**
		 * This function is used internally and exclusively by the SetAsConvexHull function.
		 * 
		 * @param[in] planarPointCloud It is assumed that all points in this set are distinct and coplanar.
		 * @param[in] plane It is assumed that all given points lie on this plane.
		 */
		void CalculateConvexHullInternal(const std::vector<Vector3>& planarPointCloud, const Plane& plane);

		/**
		 * This function is used internally and exclusively by the CalculateConvexHullInternal function.
		 * It just makes sure that, in the case this polygon is a triangle, that its front-side has a normal
		 * making an acute angle with the given normal.
		 */
		void FixWindingOfTriangle(const Vector3& desiredNormal);

	private:
		std::vector<Vector3>* vertexArray;
		mutable Plane cachedPlane;
		mutable bool cachedPlaneValid;
	};
}