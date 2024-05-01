#pragma once

#include "Defines.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Shape.h"
#include <vector>
#include <unordered_map>

namespace Collision
{
	class BoundingBoxNode;
	class Shape;
	class DebugRenderResult;

	/**
	 * This class facilitates the broad-phase of collision detection.
	 * Note that it is not a user-facing class and so the collision system
	 * user will never have to interface with it directly.
	 */
	class COLLISION_LIB_API BoundingBoxTree
	{
	public:
		BoundingBoxTree(const AxisAlignedBoundingBox& collisionWorldExtents);
		virtual ~BoundingBoxTree();

		/**
		 * Insert the given shape into this bounding-box tree.  Note that
		 * it is fine for the shape to already be in the tree; in which case,
		 * the position of the shape in the tree is adjusted.  The ideal
		 * location of a shape in our tree is for it to be as deep into the
		 * tree as it can possibly fit.  Also note that it is up to the
		 * caller to know when to re-insert a shape when its bounding box
		 * changes.  This class is non-the-wiser about changes made to
		 * shapes outside of its scope that would effect their bounding boxes,
		 * and therefore, their ideal positioning within this tree.  If a shape
		 * is changed without re-insertion, then the results of algorithms
		 * that operate on this tree are left undefined.
		 * 
		 * @param[in] shape This is the shape to insert into this tree.  It must not be a member of some other tree.  I can already be a member of this tree.
		 * @return True is returned on success; false, otherwise.
		 */
		bool Insert(Shape* shape);

		/**
		 * Remove the given shape from this bounding-box tree.
		 * 
		 * @param[in] shape This is the shape to remove from this tree.  It must already be a member of this tree.
		 */
		bool Remove(Shape* shape);

		/**
		 * Remove all shapes from this tree and delete all nodes of the tree.
		 */
		void Clear();

		/**
		 * Provide a visualization of the tree for debugging purposes.
		 */
		void DebugRender(DebugRenderResult* renderResult) const;

		/**
		 * Calculate and return a minimal set of shapes, all of which have their bounding box
		 * hit by the given ray, but (most likely) only one of which is hit by that ray before
		 * any of the others.  The caller can ray-cast each of the returned shapes individually
		 * to determine which one is hit soonest by the ray.  We do not go that far in
		 * our calculations here, because we only consider the bounding boxes of the shapes.
		 */
		void RayCast(const Ray& ray, std::vector<const Shape*>& shapeArray) const;

	private:
		BoundingBoxNode* rootNode;
		AxisAlignedBoundingBox collisionWorldExtents;
	};

	/**
	 * Instances of this class form the nodes of the BoundingBoxTree class.
	 * This class is designed to have an arbitrary branching factor, but a
	 * binary tree is best, because it minimizes the chance of a shape's box
	 * having to straddle the boundary between sub-spaces which, admittedly,
	 * is a problem I'm not yet sure how to easily solve.
	 */
	class COLLISION_LIB_API BoundingBoxNode
	{
		friend class BoundingBoxTree;

	private:
		BoundingBoxNode(BoundingBoxNode* parentNode, BoundingBoxTree* tree);
		virtual ~BoundingBoxNode();

		/**
		 * Point the given shape to this node, and this node to the given shape.
		 */
		void BindToShape(Shape* shape);

		/**
		 * Unlink the pointers between this node and the given shape in both directions.
		 */
		void UnbindFromShape(Shape* shape);

		/**
		 * If this node has no children, create two children partitioning this node's
		 * space into two ideal-sized sub-spaces.
		 */
		void SplitIfNeeded(BoundingBoxTree* tree);

		/**
		 * Render this node's space as a simple wire-frame box.
		 */
		void DebugRender(DebugRenderResult* renderResult) const;

	private:
		BoundingBoxTree* tree;								//< This is the tree of which this node is a part.
		AxisAlignedBoundingBox box;							//< This is the space represented by this node.
		std::vector<BoundingBoxNode*>* childNodeArray;		//< These are the sub-space partitions of this node.
		BoundingBoxNode* parentNode;						//< This is a pointer to the parent space containing this node.
		std::unordered_map<ShapeID, Shape*>* shapeMap;		//< These are shapes in this node's space that cannot fit in a sub-space.

		// TODO: One way to overcome one of the limitations I see with this AABB tree is to replace the above shapeMap with
		//       yet another tree that partitions the 2D space forming the intersection between adjacent 3D sub-spaces.
		//       I would hold off on this optimization, though, until I can get everything else working without it.
		//       For example, we could use a BSP tree.
	};
}