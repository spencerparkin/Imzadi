#pragma once

#include "Vector3.h"
#include "Plane.h"
#include <vector>

namespace Imzadi
{
	class Ray;

	/**
	 * These are polygons represented as a sequence of vertices, each one
	 * adjacent to the vertex preceeding or following it in the sequence.
	 * (Modular arithmetic is used on sequence locations to determine these.)
	 * All points must be co-planar, and no edge should touch another edge
	 * in a non-trivial way.  There also shouldn't be any redundant vertices.
	 * The interior area of the polygon should be non-zero.  If these conditions
	 * are not met, we leave the results of any method here as undefined.
	 * 
	 * These polygons can be convex or concave in all cases except where specified.
	 * Some methods work with both convex and concave polygons.  If a method assumes
	 * a polygon is convex, then I tried to remember to specify that in the
	 * documentation for the method.
	 * 
	 * We consider the front face of a polygon to be the face that, when viewed,
	 * has its vertices wound CCW (counter-clock-wise) in the plane of the polygon.
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
		 * If passed to the @ref IsConvex method, you can get further information
		 * about how convex or how concave the polygon is.
		 */
		struct ConvexityInfo
		{
			std::vector<int> convexVertexArray;
			std::vector<int> concaveVertexArray;
		};

		/**
		 * Return true if and only if this polygon is a convex polygon.
		 */
		bool IsConvex(ConvexityInfo* convexityInfo = nullptr, double tolerance = 1e-4) const;

		/**
		 * Calculate and return the plane containing this polygon.
		 * This assumes that the vertices of the polygon are co-planar,
		 * but does not assume the polygon is convex unless told that
		 * the polygon is convex.
		 */
		Plane CalcPlane(bool assumeConvex = false) const;

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
		 * This method does not assume the polygon is convex.
		 */
		bool ContainsPointOnEdge(const Vector3& point, double tolerance = 1e-5) const;

		/**
		 * Return an array of line-segments, each an edge of this polygon.
		 * The edges are returned in CCW order.
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
		 * 
		 * @param[in,out] polygonArray These are the polygons to compress.  This array is modified, hopefully reduced in size.
		 * @param[in] mustBeConvex If true, polygons returned will all be convex.  I false, this is not necessarily the case.
		 */
		static void Compress(std::vector<Polygon>& polygonArray, bool mustBeConvex);

		/**
		 * Generate a set of pair-wise disjoint and convex polygons
		 * whose union is this polygon.  Of course, there can be more
		 * than one answer to this problem.  We try here to come up
		 * with a reasonable tessellation based on some heuristics.
		 */
		bool TessellateUntilConvex(std::vector<Polygon>& polygonArray) const;

		/**
		 * Generate a set of pair-wise disjoint triangles whose union
		 * is this polygon.  Of course, there can be more than one
		 * answer to this problem.  We try here to come up with a
		 * reasonable tessellation based on some hueristics.
		 * 
		 * This method assumes the polygon is convex.  If you need to tessellate
		 * a concave polygon, see then @ref TessellateUntilConvex method.
		 */
		bool TessellateUntilTriangular(std::vector<Polygon>& polygonArray) const;

		/**
		 * Return the given integer in the range [0,N-1], where N is the number
		 * of vertices in this polygon (or N-gon, if you will.)
		 * 
		 * @param i This can be any integer.
		 * @return Return i mod N for this N-gon.
		 */
		int Mod(int i) const;

		/**
		 * Split this polygon into two disjoint/adjacent polygons whose union
		 * would be this polygon by cutting this polygon at vertex i to vertex j.
		 * By default, we do not assume the polygon is convex here, and we fail
		 * if the resulting polygons would be degenerate or if they would not sum
		 * to the original polygon as described.
		 */
		bool Split(int i, int j, Polygon& polygonA, Polygon& polygonB, bool assumeConvex = false) const;

		/**
		 * Write an array of polygons to the given stream in binary form.
		 */
		static void DumpArray(const std::vector<Polygon>& polygonArray, std::ostream& stream);

		/**
		 * Read an array of polygons from the stream in binary form.
		 */
		static void RestoreArray(std::vector<Polygon>& polygonArray, std::istream& stream);

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