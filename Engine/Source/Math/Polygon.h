#pragma once

#include "Vector3.h"
#include "Plane.h"
#include <vector>

namespace Imzadi
{
	class Ray;

	/**
	 * These are polygons represented as a sequence of 3D vertices, each one
	 * adjacent to the vertex preceeding or following it in the sequence.
	 * (Modular arithmetic is used on sequence locations to determine these.)
	 * All points must be co-planar, and no edge should touch another edge
	 * in a non-trivial way.  There also shouldn't be any redundant vertices.
	 * (To make matters even more complicated, sometimes a polygon should not
	 * repeat a vertex in its sequence, even if doing so does not constitute a
	 * redundancy; but then again, sometimes it's both okay and desired.)  The
	 * interior area of the polygon should be non-zero.  If these conditions (or
	 * some subset of them depending on the case) are not met, then we leave the
	 * results of a method undefined.
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
		 * this is as far as I'm going to go for now.  (E.g., we could check to
		 * make sure that the polygon doesn't self-intersect itself, but we don't.)
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
		 * Calculate and return the area of this polygon.
		 * 
		 * @param[in] assumeConvex If true, we do not try to tessellate and recurse.
		 */
		double Area(bool assumeConvex = true) const;

		/**
		 * Tell the caller if the given point is a member of the set
		 * of points consituting this polygon.  This method assumes
		 * the polygon is convex.
		 * 
		 * @param[in] point This is the point being tested against this polygon for membership in its set of points.
		 * @param[in] tolerance This is a thickness of the plane of the polygon and a girth of the polygon's edges.
		 * @param[out] isInterior If given, this is set to true if and only if the given point is approximately on the polygon, but not on an edge or vertex boundary.
		 * @return True is returned if the given point is approximately on this polygon; false, otherwise.
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
		 * others are not caught by this, as are other cases too numerable
		 * to state here.  In general, if polygons are "well behaved" and
		 * just "thouching" one another, we can get a desirable result here.
		 * 
		 * @param[in,out] polygonArray These are the polygons to compress.  This array is modified, hopefully reduced in size.
		 * @param[in] mustBeConvex If true, polygons returned will all be convex.  I false, this is not necessarily the case.
		 * @param[in] sanityCheck If true, the total area of polygons in a plane is measured before being compressed, then compared against the total area afterword.
		 */
		static void Compress(std::vector<Polygon>& polygonArray, bool mustBeConvex, bool sanityCheck = false);

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
		 * Here we try to merge the two given polygons into a single polygon.
		 * Neither polygon is assumed to be convex.  Both are assumed to reside
		 * in the same plane.  Note that this is a very non-trivial problem, and
		 * not all conceivable cases are accounted for here.  Rather, what we're
		 * looking for is an edge from A and an edge from B that touch in a non-
		 * trivial way.  No check is made to ensure that the polygons don't
		 * overlap with non-zero area, in which case, we would produce an invalid
		 * polygon here.
		 */
		bool MergeCoplanarPolygonPair(const Polygon& polygonA, const Polygon& polygonB);

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
		 * Remove as many redundancies in this polygon as we can.  The shape and
		 * area of the polygon are not changed by this routine.
		 */
		void Reduce(double tolerance = 1e-7);

		/**
		 * Return true if and only if the given vertex is, up
		 * to a distance of the given epsilon, equal to one of
		 * the vertices of this polygon.
		 */
		bool HasVertex(const Vector3& givenVertex, double epsilon = 1e-5) const;

		/**
		 * Write an array of polygons to the given stream in binary form.
		 */
		static void DumpArray(const std::vector<Polygon>& polygonArray, std::ostream& stream);

		/**
		 * Read an array of polygons from the stream in binary form.
		 */
		static void RestoreArray(std::vector<Polygon>& polygonArray, std::istream& stream);

		/**
		 * Return true if and only if area interior to the polygon
		 * overlaps with other area interior to the polygon.
		 * We assume that the polygon is reduced.  To ensure this,
		 * call the @ref Reduce method.
		 * 
		 * Of course, if a polygon is convex, then it doesn't self-overlap.
		 * But the converse of this statement is not true.
		 */
		bool SelfOverlaps(double epsilon = 1e-6) const;

		/**
		 * Write this polygon to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this polygon from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

		/**
		 * Add vertices to this polygon from another polygon.  The given
		 * polygon should not point to this polygon.  All vertices from
		 * vertex i to vertex j of the given polygon are added to this
		 * polygon.
		 */
		void AddVerticesFrom(const Polygon& polygon, int i, int j, bool includeI = true, bool includeJ = true);

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

		/**
		 * This function is used internally by the @ref Compress static method.
		 */
		static void MergeCoplanarPolygons(std::vector<Polygon>& coplanarPolygonArray);

		/**
		 * This function is used internally by the @ref MergeCoplanarPolygonPair method.
		 */
		bool MergeCoplanarPolygonPairInternal(const Polygon& polygonA, const Polygon& polygonB);

		/**
		 * Look for an instance of a vertex repeated in the sequence.  If found, remove it.
		 * 
		 * @return True is returned if and only if a removal occurred.
		 */
		bool FindAndRemoveRepeatedPoint(double epsilon);

		/**
		 * Look for an instance of a vertex colinear with the two vertices
		 * immediately preceding and following it.  If found, remove it.
		 * 
		 * @return True is returned if and only if a removal occurred.
		 */
		bool FindAndRemoveVertexOnEdge(double epsilon);

		/**
		 * Look for an instance of two edges, the same length, adjacent,
		 * and overlapping one another.  If found, remove it.
		 * 
		 * @return True is returned if and onlyf if a removal occurred.
		 */
		bool FindAndRemoveSymmetricSpike(double epsilon);

		/**
		 * Look for an instance of two edges, different lengths, adjacent,
		 * and overlapping one another.  If found, remove it.
		 * 
		 * @return True is returned if and onlyf if a removal occurred.
		 */
		bool FindAndRemoveNonSymmetricSpike(double epsilon);

	public:
		std::vector<Vector3> vertexArray;
	};
}