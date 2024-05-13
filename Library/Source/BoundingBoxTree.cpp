#include "BoundingBoxTree.h"
#include "Error.h"
#include "Result.h"
#include "Math/Ray.h"
#include "Math/Plane.h"
#include <algorithm>
#include <format>

using namespace Collision;

//--------------------------------- BoundingBoxTree ---------------------------------

BoundingBoxTree::BoundingBoxTree(const AxisAlignedBoundingBox& collisionWorldExtents)
{
	this->rootNode = nullptr;
	this->collisionWorldExtents = collisionWorldExtents;
	this->shapeMap = new std::unordered_map<ShapeID, Shape*>();
}

/*virtual*/ BoundingBoxTree::~BoundingBoxTree()
{
	this->Clear();

	delete this->shapeMap;
}

bool BoundingBoxTree::Insert(Shape* shape, uint32_t flags)
{
	if (!shape)
	{
		GetError()->AddErrorMessage("Can't insert null shape into box tree!");
		return false;
	}

	// Insertion begins either where the shape is already bound or, if not bound, at the root.
	BoundingBoxNode* node = shape->node;
	if (!node)
	{
		if (!this->rootNode)
		{
			this->rootNode = new BoundingBoxNode(nullptr, this);
			this->rootNode->box = this->collisionWorldExtents;
		}

		node = this->rootNode;
	}
	else
	{
		if (node->tree != this)
		{
			GetError()->AddErrorMessage("Can't insert node that is already a member of some other tree.");
			return false;
		}

		node->UnbindFromShape(shape);
	}

	// Bring the shape up the tree only as far as is necessary.
	while (node && !node->box.ContainsBox(shape->GetBoundingBox()))
		node = node->parentNode;
	
	// Now push the shape down the tree as far as possible.
	while (node)
	{
		// Make children for the current node if it doesn't already have them.
		node->SplitIfNotAlreadySplit(this);

		// Can the shape fit into any of the children?
		BoundingBoxNode* foundNode = nullptr;
		for (BoundingBoxNode* childNode : *node->childNodeArray)
		{
			if (childNode->box.ContainsBox(shape->GetBoundingBox()))
			{
				foundNode = childNode;
				break;
			}
		}

		// Can we push the shape deeper into the tree?
		if (foundNode)
			node = foundNode;	// Yes!
		else
		{
			// The shape is as deep as it can go.  If splitting is not allowed, we're done.
			if ((flags & COLL_SYS_ADD_FLAG_ALLOW_SPLIT) == 0)
				break;

			// Okay, splitting is allowed, but is the node too small for us to want to attempt any further splitting?
			double nodeVolume = node->box.GetVolume();
			if (nodeVolume < COLL_SYS_MIN_NODE_VOLUME)
				break;

			// Attempt to split the shape.  If we can't, we're done.
			Shape* shapeBack = nullptr;
			Shape* shapeFront = nullptr;
			if (!shape->Split(node->dividingPlane, shapeBack, shapeFront))
				break;

			// The shape was split!  Destroy the original shape and insert the sub-shapes.
			Shape::Free(shape);
			shape = nullptr;
			node->BindToShape(shapeBack);
			node->BindToShape(shapeFront);
			if (!this->Insert(shapeBack, flags))
				return false;
			if (!this->Insert(shapeFront, flags))
				return false;
			break;
		}
	}

	if (!node)
	{
		GetError()->AddErrorMessage("Failed to insert shape!  It probably does not lie within the collision world extents.");
		return false;
	}

	// At last, if we have a node/shape pair here, then complete insertion of the shape at this node and in this tree.
	if (shape)
	{
		node->BindToShape(shape);
		this->shapeMap->insert(std::pair<ShapeID, Shape*>(shape->GetShapeID(), shape));
	}

	return true;
}

bool BoundingBoxTree::Remove(ShapeID shapeID)
{
	Shape* shape = this->FindShape(shapeID);
	if (!shape)
	{
		GetError()->AddErrorMessage(std::format("No shape with ID {} found in the box tree.", shapeID));
		return false;
	}

	// If the shape was found in our map, then these conditions must also be true or something has gone wrong in our book-keeping.
	COLL_SYS_ASSERT(shape->node && shape->node->tree == this);
	shape->node->UnbindFromShape(shape);
	this->shapeMap->erase(shape->GetShapeID());
	Shape::Free(shape);
	return true;
}

Shape* BoundingBoxTree::FindShape(ShapeID shapeID)
{
	std::unordered_map<ShapeID, Shape*>::iterator iter = this->shapeMap->find(shapeID);
	if (iter == this->shapeMap->end())
		return nullptr;

	return iter->second;
}

bool BoundingBoxTree::ForAllShapes(std::function<bool(const Shape*)> callback) const
{
	for (auto pair : *this->shapeMap)
	{
		const Shape* shape = pair.second;
		if (!callback(shape))
			return false;
	}

	return true;
}

uint32_t BoundingBoxTree::GetNumShapes() const
{
	return this->shapeMap->size();
}

void BoundingBoxTree::Clear()
{
	this->collisionCache.Clear();

	delete this->rootNode;
	this->rootNode = nullptr;

	while (this->shapeMap->size() > 0)
	{
		std::unordered_map<ShapeID, Shape*>::iterator iter = this->shapeMap->begin();
		Shape* shape = iter->second;
		Shape::Free(shape);
		this->shapeMap->erase(iter);
	}
}

void BoundingBoxTree::DebugRender(DebugRenderResult* renderResult) const
{
	if (this->rootNode)
		this->rootNode->DebugRender(renderResult);
}

void BoundingBoxTree::RayCast(const Ray& ray, RayCastResult* rayCastResult) const
{
	RayCastResult::HitData hitData;
	hitData.shapeID = 0;
	hitData.alpha = std::numeric_limits<double>::max();

	if (this->rootNode && ray.HitsOrOriginatesIn(this->rootNode->box))
		this->rootNode->RayCast(ray, hitData);

	rayCastResult->SetHitData(hitData);
}

bool BoundingBoxTree::CalculateCollision(const Shape* shape, CollisionQueryResult* collisionResult) const
{
	const BoundingBoxNode* node = shape->node;
	if (!node)
	{
		GetError()->AddErrorMessage("The given shape is not inserted into an AABB tree.");
		return false;
	}

	if (node->tree != this)
	{
		GetError()->AddErrorMessage("The given shape is not a member of this AABB tree.");
		return false;
	}

	// We have to start our traversal at the root, not the node of the shape,
	// because there are some shapes that straddle boundaries at a higher level
	// in the tree that can still intersect with shapes at a lower level.
	std::list<const BoundingBoxNode*> nodeQueue;
	nodeQueue.push_back(this->rootNode);
	while (nodeQueue.size() > 0)
	{
		std::list<const BoundingBoxNode*>::iterator iter = nodeQueue.begin();
		node = *iter;
		nodeQueue.erase(iter);

		for (const BoundingBoxNode* childNode : *node->childNodeArray)
		{
			AxisAlignedBoundingBox intersection;
			if (intersection.Intersect(childNode->box, shape->GetBoundingBox()))
				nodeQueue.push_back(childNode);
		}

		for (auto pair : *node->shapeMap)
		{
			const Shape* otherShape = pair.second;
			if (shape == otherShape)
				continue;

			AxisAlignedBoundingBox intersection;
			if (intersection.Intersect(otherShape->GetBoundingBox(), shape->GetBoundingBox()))
			{
				ShapePairCollisionStatus* collisionStatus = this->collisionCache.DetermineCollisionStatusOfShapes(shape, otherShape);
				COLL_SYS_ASSERT(collisionStatus != nullptr);
				if (collisionStatus && collisionStatus->AreInCollision())
				{
					collisionResult->AddCollisionStatus(collisionStatus);
				}
			}
		}
	}

	return true;
}

//--------------------------------- BoundingBoxNode ---------------------------------

BoundingBoxNode::BoundingBoxNode(BoundingBoxNode* parentNode, BoundingBoxTree* tree)
{
	this->parentNode = parentNode;
	this->tree = tree;
	this->childNodeArray = new std::vector<BoundingBoxNode*>();
	this->shapeMap = new std::unordered_map<ShapeID, Shape*>();
}

/*virtual*/ BoundingBoxNode::~BoundingBoxNode()
{
	for (auto pair : *this->shapeMap)
	{
		Shape* shape = pair.second;
		shape->node = nullptr;
	}

	delete this->shapeMap;

	for (BoundingBoxNode* childNode : *this->childNodeArray)
		delete childNode;

	delete this->childNodeArray;
}

void BoundingBoxNode::SplitIfNotAlreadySplit(BoundingBoxTree* tree)
{
	if (this->childNodeArray->size() > 0)
		return;

	auto nodeA = new BoundingBoxNode(this, tree);
	auto nodeB = new BoundingBoxNode(this, tree);

	this->box.Split(nodeA->box, nodeB->box);

	this->childNodeArray->push_back(nodeA);
	this->childNodeArray->push_back(nodeB);

	// TODO: Calculate division plane here.
}

void BoundingBoxNode::BindToShape(Shape* shape)
{
	if (shape->node == nullptr)
	{
		this->shapeMap->insert(std::pair<ShapeID, Shape*>(shape->GetShapeID(), shape));
		shape->node = this;
	}
}

void BoundingBoxNode::UnbindFromShape(Shape* shape)
{
	if (shape->node == this)
	{
		this->shapeMap->erase(shape->GetShapeID());
		shape->node = nullptr;
	}
}

void BoundingBoxNode::DebugRender(DebugRenderResult* renderResult) const
{
	renderResult->AddLinesForBox(this->box, Vector3(1.0, 1.0, 1.0));

	for (const BoundingBoxNode* childNode : *this->childNodeArray)
		childNode->DebugRender(renderResult);
}

bool BoundingBoxNode::RayCast(const Ray& ray, RayCastResult::HitData& hitData) const
{
	struct ChildHit
	{
		const BoundingBoxNode* childNode;
		double alpha;
	};

	std::vector<ChildHit> childHitArray;
	for (const BoundingBoxNode* childNode : *this->childNodeArray)
	{
		if (childNode->box.ContainsPoint(ray.origin))
			childHitArray.push_back(ChildHit{ childNode, 0.0 });
		else
		{
			double boxHitAlpha = 0.0;
			if (ray.CastAgainst(childNode->box, boxHitAlpha))
				childHitArray.push_back(ChildHit{ childNode, boxHitAlpha });
		}
	}

	std::sort(childHitArray.begin(), childHitArray.end(), [](const ChildHit& childHitA, const ChildHit& childHitB) -> bool {
		return childHitA.alpha < childHitB.alpha;
	});

	// The main optimization here is the early-out, which allows us to disregard branches of the tree.
	for (const ChildHit& childHit : childHitArray)
	{
		const BoundingBoxNode* childNode = childHit.childNode;
		if (childNode->RayCast(ray, hitData))
			break;
	}

	// What remains is to check the current hit, if any, against what's at this node.
	bool hitOccurredAtThisNode = false;
	for (auto pair : *this->shapeMap)
	{
		const Shape* shape = pair.second;

		double shapeAlpha = 0.0;
		Vector3 unitSurfaceNormal;
		if (shape->RayCast(ray, shapeAlpha, unitSurfaceNormal) && 0.0 <= shapeAlpha && shapeAlpha < hitData.alpha)
		{
			hitOccurredAtThisNode = true;
			hitData.shapeID = shape->GetShapeID();
			hitData.surfaceNormal = unitSurfaceNormal;
			hitData.surfacePoint = ray.CalculatePoint(shapeAlpha);
			hitData.alpha = shapeAlpha;
		}
	}

	return hitOccurredAtThisNode;
}