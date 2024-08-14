#pragma once

#include "Vector3.h"
#include <vector>

namespace Imzadi
{
	class Plane;
	class LineSegment;

	/**
	 * An AABB for short, these boxes whose sides are parallel to the XY, YZ or XZ planes.
	 * Being axis-aligned, many operations can be performed between them more efficiently.
	 * A general box might be defined in terms of an AABB and rigid-body transform.
	 * As a point-set, we consider these to be closed in the sense that corner, edge and
	 * face points of the box are members of the set.  Note that all methods are left
	 * undefined if the stored corners of this AABB are invalid.
	 */
	class IMZADI_API AxisAlignedBoundingBox
	{
	public:
		/**
		 * The default box is at origin with zero volume.
		 */
		AxisAlignedBoundingBox();

		/**
		 * Construct a box containing the given point and only the given point.
		 */
		AxisAlignedBoundingBox(const Vector3& point);

		/**
		 * Construct a box that is a copy of the given AABB.
		 */
		AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb);

		/**
		 * Do nothing.
		 */
		virtual ~AxisAlignedBoundingBox();

		/**
		 * Set this AABB as a copy of the given AABB.
		 * 
		 * @param[in] aabb This is the AABB to copy.
		 */
		void operator=(const AxisAlignedBoundingBox& aabb);

		/**
		 * Tell the caller if this AABB is valid.  We also check for Inf and NaN here.
		 * 
		 * @return True is returned if the stored minimum corner is less-than (component-wise) the stored maximum corner.
		 */
		bool IsValid() const;

		/**
		 * Tell the caller if the given point is a member of this AABB's point-set.
		 * This box is considered a closed set in that the sides, edges and corners
		 * are considered members of the set.  A border thickness can be given to
		 * extend the box outward a distance equal to the thickness value.
		 * 
		 * @param[in] borderThickness If non-zero, this allows a point to approximately lie on the sides, edges or corners of the box.
		 * @return True is returned if the given point is interior to, or on the edge of, this AABB.
		 */
		bool ContainsPoint(const Vector3& point, double borderThickness = 0.0) const;

		/**
		 * Return true if and only if the given point is on this box's border.
		 * 
		 * @param[in] borderThickness This is used to define the approximate border of the box.  It's a radius about the zero-width border in reality.
		 */
		bool ContainsPointOnSurface(const Vector3& point, double borderThickness = 0.0) const;

		/**
		 * Return true if and only if the given point is in this box, but not
		 * on its border.
		 * 
		 * @param[in] borderThickness This is used to define the approximate border of the box.  It's a radius about the zero-width border in reality.
		 */
		bool ContainsInteriorPoint(const Vector3& point, double borderThickness = 0.0) const;

		/**
		 * Return true if and only if the given point is contained in a plane containing one of the faces of this box.
		 * 
		 * @param[in] borderThickness This is the thickness of the face plane.
		 */
		bool PointOnFacePlane(const Vector3& point, double borderThickness = 0.0) const;

		/**
		 * Tell the caller if the given AABB is contained within this AABB.
		 * 
		 * @return True is returned if the given box is a sub-box of this box; false, otherwise.
		 */
		bool ContainsBox(const AxisAlignedBoundingBox& box) const;

		/**
		 * Set this AABB to be the intersection, if any, of the two given AABBs.
		 * This is a commutative operation.
		 * 
		 * @param[in] aabbA The first AABB taken in the intersection operation.
		 * @param[in] aabbB The second AABB taken in the intersection operation.
		 * @return True is returned on success; false, otherwise, and this AABB is left undefined.
		 */
		bool Intersect(const AxisAlignedBoundingBox& aabbA, const AxisAlignedBoundingBox& aabbB);

		/**
		 * Set this box to an invalid box, but one that is ready to be expanded
		 * using repeated calls to the @ref Expand method.
		 */
		void MakeReadyForExpansion();

		/**
		 * Minimally expand this AABB so that it includes the given point.
		 * 
		 * @param[in] point This is the point to include in this box.
		 */
		void Expand(const Vector3& point);

		/**
		 * Minimally expand this AABB so that it includes the given set of points.
		 * 
		 * @param[in] pointArray This is the set of points to include in this box.
		 */
		void Expand(const std::vector<Vector3>& pointArray);

		/**
		 * Minimally expand this AABB so that it includes the given AABB.
		 * 
		 * @param[in] box This is the AABB to include in this box.
		 */
		void Expand(const AxisAlignedBoundingBox& box);

		/**
		 * Scale this box about its center uniformly.
		 * 
		 * @param[in] scale This box is scaled by this factor.
		 */
		void Scale(double scale);

		/**
		 * Scale this box about its center non-uniformly.
		 * 
		 * @param[in] scaleX This box is scaled by this factor in the X dimension.
		 * @param[in] scaleY This box is scaled by this factor in the Y dimension.
		 * @param[in] scaleZ This box is scaled by this factor in the Z dimension.
		 */
		void Scale(double scaleX, double scaleY, double scaleZ);

		/**
		 * Return the center point of the box.
		 */
		Vector3 GetCenter() const;

		/**
		 * Cut this AABB exactly in half along a plane such that the resulting two
		 * halfs are as close to cubical as possible.  That is, the longest dimension
		 * of this AABB is determined, and then the plane is made orthogonal to this
		 * dimension.
		 * 
		 * @param[out] aabbA This will hold the first half.
		 * @param[out] aabbB This will hold the second half.
		 * @param[out] divisionPlane This is an optional paramter that, if given, will be set to the plane separating the two returned AABBs with normal pointing toward the second of these.
		 */
		void Split(AxisAlignedBoundingBox& aabbA, AxisAlignedBoundingBox& aabbB, Plane* divisionPlane = nullptr) const;

		/**
		 * Calculate and return the dimensions (lengths) of the sides of this AABB.
		 * 
		 * @param[out] xSize This is the size of the box along the X dimension.
		 * @param[out] ySize This is the size of the box along the Y dimension.
		 * @param[out] zSize This is the size of the box along the Z dimension.
		 */
		void GetDimensions(double& xSize, double& ySize, double& zSize) const;

		/**
		 * Calculate this AABB as the one that most tightly fits the given set of points.
		 * Nothing is done if the given array is empty.
		 * 
		 * @param[in] pointCloud This is the set of points to use for the operation.
		 */
		void SetToBoundPointCloud(const std::vector<Vector3>& pointCloud);

		/**
		 * Calculate and return the 6 planes that form the sides of this box.
		 * If, for example, the box has no depth, but it does have width and
		 * height, then only 4 planes are returned.  The results are left
		 * undefined if this AABB is invalid.  The plane normals always
		 * face away from the box.
		 * 
		 * @param[out] sidePlaneArray This array is populated with the planes containing the sides of this box.
		 */
		void GetSidePlanes(std::vector<Plane>& sidePlaneArray) const;

		/**
		 * Calculate and return the 8 vertices of this box.
		 * 
		 * @param[out] vertexArray This array is populated with the vertices of this box.
		 */
		void GetVertices(std::vector<Vector3>& vertexArray) const;

		/**
		 * Calculate and return the 12 line-segments forming the edges of this box.
		 * 
		 * @param[out] edgeSegmentArray This array is populated with the line-segments continaing the edges of this box.
		 */
		void GetEdgeSegments(std::vector<LineSegment>& edgeSegmentArray) const;

		/**
		 * Return the volume of this AABB.
		 */
		double GetVolume() const;

		/**
		 * Calculate and return the tightest sphere containing this AABB.
		 * One application here is to check the returned sphere (once transformed
		 * into camera space) against a frustum.  If it intersects, then we
		 * approximate the AABB as also intersecting the frustum.
		 * 
		 * @param[out] center This is the center of the returned sphere.
		 * @param[out] radius This is the radius of the returned sphere.
		 */
		void GetSphere(Vector3& center, double& radius) const;

		/**
		 * Calculate and return the point on this box's boundary that is closest
		 * to the given point.
		 */
		Vector3 ClosestPointTo(const Vector3& point, double borderThickness = 0.0) const;

		/**
		 * Find and return the points on each face and edge that are closest
		 * to the given point.
		 */
		void GatherClosestPointsTo(const Vector3& point, std::vector<Vector3>& closestPointsArray, double borderThickness = 0.0, bool returnListSorted = false) const;

		/**
		 * Return a random point inside this box.
		 */
		Vector3 GetRandomContainingPoint() const;

		/**
		 * Write this AABB to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this AABB from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

	public:
		Vector3 minCorner;
		Vector3 maxCorner;
	};
}