#include "BoundingBoxTree.h"
#include "Error.h"
#include "Result.h"
#include "Math/Ray.h"
#include <algorithm>

using namespace Collision;

//--------------------------------- BoundingBoxTree ---------------------------------

BoundingBoxTree::BoundingBoxTree(const AxisAlignedBoundingBox& collisionWorldExtents)
{
	this->rootNode = nullptr;
	this->collisionWorldExtents = collisionWorldExtents;
}

/*virtual*/ BoundingBoxTree::~BoundingBoxTree()
{
	this->Clear();
}

bool BoundingBoxTree::Insert(Shape* shape)
{
	if (!this->rootNode)
	{
		this->rootNode = new BoundingBoxNode(nullptr, this);
		this->rootNode->box = this->collisionWorldExtents;
	}

	BoundingBoxNode* node = shape->node;
	if (!node)
		node = this->rootNode;
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
	
	// Now put the shape down the tree as far as possible.
	while (node)
	{
		node->SplitIfNeeded(this);

		BoundingBoxNode* foundNode = nullptr;
		for (BoundingBoxNode* childNode : *node->childNodeArray)
		{
			if (childNode->box.ContainsBox(shape->GetBoundingBox()))
			{
				foundNode = childNode;
				break;
			}
		}

		if (foundNode)
			node = foundNode;
		else
			break;
	}

	if (!node)
	{
		GetError()->AddErrorMessage("Failed to insert shape!  It probably does not lie within the collision world extents.");
		return false;
	}

	node->BindToShape(shape);
	return true;
}

bool BoundingBoxTree::Remove(Shape* shape)
{
	if (!shape->node)
	{
		GetError()->AddErrorMessage("The given shape is not a member of any tree!");
		return false;
	}

	if (shape->node->tree != this)
	{
		GetError()->AddErrorMessage("The given shape can't be removed from this tree, because it is not a member of this tree.");
		return false;
	}

	shape->node->UnbindFromShape(shape);
	return true;
}

void BoundingBoxTree::Clear()
{
	this->collisionCache.Clear();

	delete this->rootNode;
	this->rootNode = nullptr;
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

void BoundingBoxNode::SplitIfNeeded(BoundingBoxTree* tree)
{
	if (this->childNodeArray->size() > 0)
		return;

	auto nodeA = new BoundingBoxNode(this, tree);
	auto nodeB = new BoundingBoxNode(this, tree);

	this->box.Split(nodeA->box, nodeB->box);

	this->childNodeArray->push_back(nodeA);
	this->childNodeArray->push_back(nodeB);
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