#pragma once

#include "Defines.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Math/Plane.h"
#include "Shape.h"
#include "Result.h"
#include "CollisionCache.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace Imzadi
{
	class BoundingBoxNode;

	/**
	 * This class facilitates the broad-phase of collision detection.
	 * Note that it is not a user-facing class and so the collision system
	 * user will never have to interface with it directly.  This class,
	 * when appropriate, calls directly into the narrow-phase.
	 */
	class IMZADI_API BoundingBoxTree
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
		 * and therefore, their ideal positioning within the tree.  If a shape
		 * is changed without re-insertion, then the results of algorithms
		 * that operate on this tree are left undefined.
		 * 
		 * Nodes of the tree are created as needed to get optimal insertion.
		 * Also, it's okay for a give shape to not fit in the tree's overall
		 * bounds, in which case, it's tracked in our map, but does not occupy
		 * a place in the tree.  Note that collision queries will not work against
		 * anything that is not in the tree, even if it is in this class's map.
		 * You can check to see if a shape landed in the tree by calling the
		 * Shape::IsBound method on the shape.
		 * 
		 * Note that if the IMZADI_ADD_FLAG_ALLOW_SPLIT is passed in, then
		 * we try to split the given shape up as needed to get it as deep into
		 * the tree as possible, with a reasonable limit on how small a leaf
		 * node can get.  Also, only on successful insertion is the ownership
		 * of the memory of the given shape taken on by the tree.  If it gets
		 * split, then it will be deleted, and its ID will become invalid.
		 * If splitting is not allowed and insertion is successful, then you
		 * can continue to refer to the shape on the main thread by its ID.
		 * Thus, splitting is designed for static collision shapes.  It doesn't
		 * make sense to split dynamic collision shapes.
		 * 
		 * @param[in] shape This is the shape to insert (or re-insert) into this tree.
		 * @param[in] flags This is an OR-ing of flags of the form IMZADI_ADD_FLAG_*.  In particular, we look at the IMZADI_ADD_FLAG_ALLOW_SPLIT flag to see if shape splitting is allowed.
		 * @return True is returned on success; false, otherwise.
		 */
		bool Insert(Shape* shape, uint32_t flags);

		/**
		 * Remove the shape having the given ID from this bounding-box tree.
		 * 
		 * @param[in] shapeID This is the shape to remove from the tree.  It must already be a member of this tree.
		 */
		bool Remove(ShapeID shapeID);

		/**
		 * Find and return the shape having the given shape ID.
		 *
		 * @param[in] shapeID This is the ID of the shape to find within the collision world.
		 * @return If found, a pointer to the shape is returned; null, otherwise.
		 */
		Shape* FindShape(ShapeID shapeID);

		/**
		 * Provide a convenient way to iterate all shapes of the tree.
		 * 
		 * @param[in] callback This is a lambda that is given each shape of the tree and expected to return true if and only if iteration should continue.
		 * @return True is returned if every invocation of the callback returned true.
		 */
		bool ForAllShapes(std::function<bool(const Shape*)> callback) const;

		/**
		 * Return the number of shapes being stored in the tree.
		 */
		uint32_t GetNumShapes() const;

		/**
		 * Remove all shapes from this tree and delete all nodes of the tree.
		 */
		void Clear();

		/**
		 * Provide a visualization of the tree for debugging purposes.
		 */
		void DebugRender(DebugRenderResult* renderResult) const;

		/**
		 * Perform a ray-cast against all collision shapes within the tree.
		 * 
		 * @param[in] ray This is the ray with which to perform the cast.
		 * @param[out] rayCastResult The hit result, if any, is put into the given RayCastResult instance.  If no hit, then the result will indicate as much.
		 */
		void RayCast(const Ray& ray, RayCastResult* rayCastResult) const;

		/**
		 * Determine the collision status of the given shape.
		 * 
		 * @param[in] shape This is the shape in question.
		 * @param[out] collisionResult The collision status is returned in this instance of the CollisionQueryResult class.
		 * @return True is returned on success; false, otherwise.
		 */
		bool CalculateCollision(const Shape* shape, CollisionQueryResult* collisionResult) const;

	private:
		std::unordered_map<ShapeID, Shape*> shapeMap;		///< We keep a map here of all shapes stored in the tree.
		BoundingBoxNode* rootNode;							///< The root note represents the entire space managed by the collision system.
		AxisAlignedBoundingBox collisionWorldExtents;		///< When the root note is created, it takes on this extent.
		mutable CollisionCache collisionCache;				///< This is used to speed up the narrow-phase of collision detection.
	};

	/**
	 * Instances of this class form the nodes of the BoundingBoxTree class.
	 * This class is designed to have an arbitrary branching factor, but a
	 * binary tree is best, because it minimizes the chance of a shape's box
	 * having to straddle the boundary between sub-spaces which, admittedly,
	 * is a problem I'm not yet sure how to easily solve.
	 */
	class IMZADI_API BoundingBoxNode
	{
		friend class BoundingBoxTree;

	private:
		BoundingBoxNode(BoundingBoxNode* parentNode);
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
		void SplitIfNotAlreadySplit();

		/**
		 * Render this node's space as a simple wire-frame box.
		 */
		void DebugRender(DebugRenderResult* renderResult) const;

		/**
		 * Descend the tree, performing a ray-cast as we go.
		 * 
		 * @param[in] ray This is the ray with which to perform the ray-cast.
		 * @param[out] hitData This will contain info about what shape was hit and how, if any.
		 * @return True is returned if and only if a hit ocurred in this node of the tree.
		 */
		bool RayCast(const Ray& ray, RayCastResult::HitData& hitData) const;

	private:
		AxisAlignedBoundingBox box;							///< This is the space represented by this node.
		std::vector<BoundingBoxNode*> childNodeArray;		///< These are the sub-space partitions of this node.
		BoundingBoxNode* parentNode;						///< This is a pointer to the parent space containing this node.
		std::unordered_map<ShapeID, Shape*> shapeMap;		///< These are shapes in this node's space that cannot fit in a sub-space.
		Plane dividingPlane;								///< This is a plane dividing this node's space into two sub-spaces, but not dividing any of this node's sub-nodes.
	};
}