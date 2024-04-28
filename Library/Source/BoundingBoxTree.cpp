#include "BoundingBoxTree.h"
#include "Error.h"
#include "Result.h"
#include "Math/Ray.h"

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
	delete this->rootNode;
	this->rootNode = nullptr;
}

void BoundingBoxTree::DebugRender(DebugRenderResult* renderResult) const
{
	if (this->rootNode)
		this->rootNode->DebugRender(renderResult);
}

void BoundingBoxTree::RayCast(const Ray& ray, std::vector<const Shape*>& shapeArray) const
{
	shapeArray.clear();

	if (!this->rootNode)
		return;

	AxisAlignedBoundingBox shapeArrayBox;
	double smallestAlpha = std::numeric_limits<double>::max();

	// Use a non-recursive, breadth-first traversal of the tree.
	std::list<const BoundingBoxNode*> nodeQueue;
	nodeQueue.push_back(this->rootNode);
	while (nodeQueue.size() > 0)
	{
		// Grab our next node off the queue for processing.
		std::list<const BoundingBoxNode*>::iterator iter = nodeQueue.begin();
		const BoundingBoxNode* node = *iter;
		nodeQueue.erase(iter);

		// We can skip this branch of the tree if the ray doesn't intersect it.
		if (!ray.HitsOrOriginatesIn(node->box))
			continue;
		
		// Queue up this node's branches for later.
		for (const BoundingBoxNode* childNode : *node->childNodeArray)
			nodeQueue.push_back(childNode);

		// For now, we need to process all this node's shapes.
		for (auto pair : *node->shapeMap)
		{
			const Shape* shape = pair.second;

			// If the ray doesn't hit the shape's bounding box, then it doesn't hit the shape.
			double alpha = 0.0;
			if (!ray.CastAgainst(shape->GetBoundingBox(), alpha))
				continue;
			
			// In this case, it's the first hit, so add it to our list.
			if (shapeArray.size() == 0)
			{
				shapeArray.push_back(shape);
				shapeArrayBox = shape->GetBoundingBox();
				smallestAlpha = alpha;
				continue;
			}
			
			// In this case, even if the ray hits the box closer, a shape in our
			// existing list may still hit the ray closer than the shape in the
			// newly hit box, and that is why we add to the list.
			AxisAlignedBoundingBox intersection;
			if (intersection.Intersect(shapeArrayBox, shape->GetBoundingBox()))
			{
				shapeArray.push_back(shape);
				shapeArrayBox.Expand(shape->GetBoundingBox());
				if (alpha < smallestAlpha)
					smallestAlpha = alpha;
				continue;
			}
			
			// In this case, we can conclude that a shape in our existing list
			// hits closer than whatever is in the box we just hit.
			if (alpha > smallestAlpha)
				continue;

			// In this case, we can conclude that the shape in the box we just hit
			// must hit closer than anything in our current list of shapes, so start
			// the list over again with the new shape.
			shapeArray.clear();
			shapeArray.push_back(shape);
			shapeArrayBox = shape->GetBoundingBox();
			smallestAlpha = alpha;
		}
	}
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