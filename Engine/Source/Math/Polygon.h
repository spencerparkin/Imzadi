#pragma once

#include "Vector3.h"
#include "Plane.h"
#include <vector>

namespace Imzadi
{
	class Ray;

	/**
	 * These are polygons represented as a sequence of vertices.
	 * All points must be co-planar, and no edge should cross another edge.
	 * There also shouldn't be any redundant vertices.  If these conditions
	 * are not met, we leave the results of any method here as undefined.
	 * 
	 * These polygons can be convex or concave in all cases except where specified.
	 * Some methods work with both convex and concave polygons.  If a method assumes
	 * a polygon is convex, then I tried to remember to specify that in the
	 * documentation for the method.
	 */
	class IMZADI_API Polygon
	{
	public:
		Polygon();
		Polygon(const Polygon& polygon);
		virtual ~Polygon();

		void operator=(const Polygon& polygon);

		/**
		 * Here we make sure that the polygon vertices are all valid and coplanar.
		 * If we were being more therough, we would also check for more things, but
		 * this is as far as I'm going to go for now.
		 */
		bool IsValid(double tolerance = 1e-4) const;

		/**
		 * Return true if and only if this polygon is a convex polygon.
		 */
		bool IsConvex(double tolerance = 1e-4) const;

		/**
		 * Calculate and return the plane containing this polygon.
		 */
		Plane CalcPlane() const;

		/**
		 * Calculate and return the average of all the vertices of this polygon.
		 * If the polygon is convex, then this will be inside the polygon.
		 */
		Vector3 CalcCenter() const;

		/**
		 * Assuming that this polygon is convex, calculate
		 * and return the polygon's area.
		 */
		double Area() const;

		/**
		 * This method assumes the polygon is convex.
		 */
		bool ContainsPoint(const Vector3& point, double tolerance = 1e-5, bool* isInterior = nullptr) const;

		/**
		 * 
		 */
		bool ContainsPointOnEdge(const Vector3& point, double tolerance = 1e-5) const;

		/**
		 * 
		 */
		void GetEdges(std::vector<LineSegment>& edgeArray) const;

		/**
		 * This method assumes the polygon is convex.
		 */
		bool SplitAgainstPlane(const Plane& plane, Polygon& backPolygon, Polygon& frontPolygon, double planeThickness = 1e-6) const;

		/**
		 * Calculate and return the point on this polygon that is closest
		 * to the given point.  This method assumes the polygon is convex.
		 */
		Vector3 ClosestPointTo(const Vector3& point) const;

		/**
		 * Tell the caller if and where the given line segment intersects this polygon.
		 * This method assumes the polygon is convex.
		 * 
		 * @param[in] lineSegment This is the line-segment to test against this polygon.  It must be non-degenerate.
		 * @param[out] intersectionPoint This will be the point on the given line-segment where it intersects this polygon, if at all; left undefined, otherwise.
		 * @return True is returned if and only if the given line segment shares a single point in common with this polygon.
		 */
		bool IntersectsWith(const LineSegment& lineSegment, Vector3& intersectionPoint) const;

		/**
		 * Calculate and return a plane that best fits this polygon's set of vertices,
		 * which are not assumed to be a valid polygon.  They can be any cloud of points in 3D space.
		 * If, however, this polygon is valid, then there is one and only one such plane, up to sign.
		 *
		 * @param[out] plane The best fit plane is calculated and returned using a linear least-squares method.
		 * @return True is returned if successful; false, otherwise.
		 */
		bool CalculatePlaneOfBestFit(Plane& plane) const;

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
		 * @param[in] pointCloud This is the set of points, in object-space, to use in the operation.
		 */
		void SetAsConvexHull(const std::vector<Vector3>& pointCloud);

		/**
		 * Perform a ray-cast against this polygon.  This method assumes that the polygon is convex.
		 *
		 * @param[in] ray This is the ray to use in the ray-cast.
		 * @param[out] alpha This is the distance from the ray origin along the ray-direction to the point where the polygon is hit, if any.
		 * @param[out] unitSurfaceNormal This is the surface normal of the polygon at the ray hit-point, if any.  It will always make an obtuse angle with the ray direction vector.
		 * @return True is returned if and only if the given ray hits this polygon.
		 */
		bool RayCast(const Ray& ray, double& alpha, Vector3& unitSurfaceNormal) const;

		/**
		 * Merge co-planar and overlapping polygons into single polygons
		 * as far as possible.  Polygons that are purely nested within
		 * others are not caught by this, but this is the only case I'm
		 * not yet handling, as far as I'm aware.
		 */
		static void Compress(std::vector<Polygon>& polygonArray);

		/**
		 * Generate a set of pair-wise disjoint and convex polygons
		 * whose union is this polygon.
		 */
		void TessellateUntilConvex(std::vector<Polygon>& polygonArray) const;

		/**
		 * This method assumes the polygon is convex.
		 */
		void TessellateUntilTriangular(std::vector<Polygon>& polygonArray) const;

		/**
		 * Write this polygon to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this polygon from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	private:
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

	public:
		std::vector<Vector3> vertexArray;
	};
}