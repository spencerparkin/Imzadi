#include "PlanarGraph.h"
#include "LineSegment.h"
#include <algorithm>

using namespace Imzadi;

//---------------------------- PlanarGraph ----------------------------

PlanarGraph::PlanarGraph()
{
}

/*virtual*/ PlanarGraph::~PlanarGraph()
{
	this->Clear();
}

void PlanarGraph::SetPlane(const Plane& plane)
{
	this->plane = plane;
}

const Plane& PlanarGraph::GetPlane() const
{
	return this->plane;
}

void PlanarGraph::Clear()
{
	for (Node* node : this->nodeArray)
		delete node;

	this->nodeArray.clear();
	this->edgeSet.clear();
}

bool PlanarGraph::AddPolygon(const Polygon& polygon, double epsilon /*= 1e-6*/)
{
	std::vector<LineSegment> edgeArray;
	polygon.GetEdges(edgeArray);
	for (const LineSegment& lineSeg : edgeArray)
		if (!this->AddEdge(lineSeg.point[0], lineSeg.point[1]))
			return false;

	return true;
}

bool PlanarGraph::AddEdge(const Vector3& vertexA, const Vector3& vertexB, double epsilon /*= 1e-6*/)
{
	if (!this->AddVertex(vertexA, epsilon))
		return false;

	if (!this->AddVertex(vertexB, epsilon))
		return false;

	// This is the new edge we want to add the graph, but it may already
	// be intersected by pre-existing vertices and pre-existing edges.
	PlanarGraphEdge edge;
	edge.i = this->FindVertex(vertexA, epsilon);
	edge.j = this->FindVertex(vertexB, epsilon);
	IMZADI_ASSERT(edge.i >= 0 && edge.j >= 0);
	
	std::list<PlanarGraphEdge> edgeQueueA;
	std::list<PlanarGraphEdge> edgeQueueB;

	// First, break-up the edge based on pre-existing interior vertices.
	edgeQueueA.push_back(edge);
	while (edgeQueueA.size() > 0)
	{
		std::list<PlanarGraphEdge>::iterator iter = edgeQueueA.begin();
		PlanarGraphEdge newEdge = *iter;
		edgeQueueA.erase(iter);

		bool addNewEdge = true;

		LineSegment newLineSeg = this->MakeLineSegment(newEdge);

		for (const Node* node : this->nodeArray)
		{
			if (newLineSeg.point[0].IsPoint(node->vertex, epsilon) ||
				newLineSeg.point[1].IsPoint(node->vertex, epsilon))
			{
				continue;
			}

			double alpha = 0.0;
			if (newLineSeg.Alpha(node->vertex, alpha, epsilon) && 0.0 <= alpha && alpha <= 1.0)
			{
				int i = this->FindVertex(node->vertex, epsilon);
				IMZADI_ASSERT(i >= 0);
				PlanarGraphEdge newEdgeA{ newEdge.i, i };
				PlanarGraphEdge newEdgeB{ i, newEdge.j };
				edgeQueueA.push_back(newEdgeA);
				edgeQueueA.push_back(newEdgeB);
				addNewEdge = false;
				break;
			}
		}

		if (addNewEdge)
			edgeQueueB.push_back(newEdge);
	}

	// Finally, add the broken-up edges to the graph, breaking them up
	// further based on non-trivial intersections with other edges.
	while (edgeQueueB.size() > 0)
	{
		std::list<PlanarGraphEdge>::iterator iter = edgeQueueB.begin();
		PlanarGraphEdge newEdge = *iter;
		edgeQueueB.erase(iter);

		bool addNewEdge = true;

		LineSegment newLineSeg = this->MakeLineSegment(newEdge);

		for (PlanarGraphEdge existingEdge : this->edgeSet)
		{
			if (existingEdge.i == newEdge.i && existingEdge.j == newEdge.j)
			{
				addNewEdge = false;
				break;
			}

			if (existingEdge.i == newEdge.j && existingEdge.j == newEdge.i)
				break;

			if (existingEdge.i == newEdge.i || existingEdge.i == newEdge.j ||
				existingEdge.j == newEdge.i || existingEdge.j == newEdge.j)
			{
				continue;
			}

			LineSegment existingLineSeg = this->MakeLineSegment(existingEdge);

			LineSegment shortestConnector;
			if (shortestConnector.SetAsShortestConnector(newLineSeg, existingLineSeg) && shortestConnector.Length() < epsilon)
			{
				Vector3 vertex = shortestConnector.Lerp(0.5);
				if (!this->AddVertex(vertex, epsilon))
					return false;

				int i = this->FindVertex(vertex, epsilon);
				IMZADI_ASSERT(i >= 0);
				PlanarGraphEdge newEdgeA{ newEdge.i, i };
				PlanarGraphEdge newEdgeB{ i, newEdge.j };
				edgeQueueB.push_back(newEdgeA);
				edgeQueueB.push_back(newEdgeB);
				addNewEdge = false;
				break;
			}
		}

		if (addNewEdge)
			this->edgeSet.insert(newEdge);
	}

	return true;
}

bool PlanarGraph::AddVertex(const Vector3& vertex, double epsilon /*= 1e-6*/)
{
	int i = this->FindVertex(vertex, epsilon);
	if (i >= 0)
		return true;

	auto node = new Node();
	node->vertex = vertex;
	this->nodeArray.push_back(node);

	// Any newly added point could be interior to at most one pre-existing edge.
	for (PlanarGraphEdge edge : this->edgeSet)
	{
		LineSegment lineSeg = this->MakeLineSegment(edge);

		if (lineSeg.ShortestDistanceTo(vertex) < epsilon &&
			!lineSeg.point[0].IsPoint(vertex) &&
			!lineSeg.point[1].IsPoint(vertex))
		{
			this->edgeSet.erase(edge);
			PlanarGraphEdge edgeA{ edge.i, (int)this->nodeArray.size() - 1 };
			PlanarGraphEdge edgeB{ (int)this->nodeArray.size() - 1, edge.j };
			this->edgeSet.insert(edgeA);
			this->edgeSet.insert(edgeB);
			break;
		}
	}

	return true;
}

int PlanarGraph::FindVertex(const Vector3& vertex, double epsilon /*= 1e-6*/) const
{
	for (int i = 0; i < (signed)this->nodeArray.size(); i++)
	{
		const Node* node = this->nodeArray[i];
		if (vertex.IsPoint(node->vertex, epsilon))
			return i;
	}

	return -1;
}

int PlanarGraph::CountConnectedComponents() const
{
	this->RegenerateNodeAdjacencies();

	std::unordered_set<const Node*> nodeSet;
	int count = 0;
	for (const Node* node : this->nodeArray)
	{
		if (nodeSet.find(node) != nodeSet.end())
			continue;

		count++;
		std::vector<const Node*> nodeArray;
		this->FindComponent(node, nodeArray);
		for (const Node* node : nodeArray)
			nodeSet.insert(node);
	}

	return count;
}

void PlanarGraph::FindComponent(const Node* node, std::vector<const Node*>& nodeArray) const
{
	nodeArray.clear();

	std::list<const Node*> nodeQueue;
	std::unordered_set<const Node*> nodeSet;
	nodeQueue.push_back(node);
	nodeSet.insert(node);

	while (nodeQueue.size() > 0)
	{
		std::list<const Node*>::iterator iter = nodeQueue.begin();
		const Node* node = *iter;
		nodeQueue.erase(iter);
		nodeArray.push_back(node);

		for (const Node* adjacentNode : node->adjacentNodeArray)
		{
			if (nodeSet.find(adjacentNode) == nodeSet.end())
			{
				nodeQueue.push_back(adjacentNode);
				nodeSet.insert(adjacentNode);
			}
		}
	}
}

LineSegment PlanarGraph::MakeLineSegment(const PlanarGraphEdge& edge) const
{
	LineSegment lineSeg;
	lineSeg.point[0] = this->nodeArray[edge.i]->vertex;
	lineSeg.point[1] = this->nodeArray[edge.j]->vertex;
	return lineSeg;
}

bool PlanarGraph::ExtractAllPolygons(std::vector<Polygon>& polygonArray)
{
	int initialCount = this->CountConnectedComponents();
	
	std::unordered_set<PlanarGraphEdge> bidirectionalEdgeSet;

	for (const PlanarGraphEdge& edge : this->edgeSet)
	{
		PlanarGraphEdge otherEdge = edge;
		otherEdge.Swap();

		if (this->edgeSet.find(otherEdge) != this->edgeSet.end())
		{
			if (otherEdge.i > otherEdge.j)
				otherEdge.Swap();

			bidirectionalEdgeSet.insert(otherEdge);
		}
	}

	// Cancel as many edges as we can without increasing the number of connected components.
	for (const PlanarGraphEdge& bidirectionalEdge : bidirectionalEdgeSet)
	{
		PlanarGraphEdge edgeA = bidirectionalEdge;
		PlanarGraphEdge edgeB = bidirectionalEdge;

		edgeB.Swap();

		this->edgeSet.erase(edgeA);
		this->edgeSet.erase(edgeB);

		int count = this->CountConnectedComponents();
		if (count > initialCount)
		{
			this->edgeSet.insert(edgeA);
			this->edgeSet.insert(edgeB);
		}
	}
	
	this->RegenerateNodeAdjacencies();

	std::unordered_set<const Node*> pendingNodesSet;
	for (const Node* node : this->nodeArray)
		pendingNodesSet.insert(node);

	// I can see this failing to give me exactly what I want in
	// the case of nested polygons, but this may be good enough.
	// Nested polygons shouldn't show up in my particular use-case.
	// TODO: Oops, are we going to find the same cycle more than once?  Just destroy the graph as we go?
	while (pendingNodesSet.size() > 0)
	{
		const Node* node = *pendingNodesSet.begin();
		pendingNodesSet.erase(node);

		std::vector<const Node*> cycleArray;
		if (this->FindOuterCycle(node, cycleArray))
		{
			for (const Node* cycleNode : cycleArray)
				pendingNodesSet.erase(cycleNode);

			Polygon polygon;
			for (const Node* node : cycleArray)
				polygon.vertexArray.push_back(node->vertex);

			Polygon reducedPolygon;
			reducedPolygon.ReduceVerticesOf(polygon);

			polygonArray.push_back(reducedPolygon);
		}
	}

	return true;
}

bool PlanarGraph::FindOuterCycle(const Node* node, std::vector<const Node*>& cycleArray) const
{
	std::vector<const Node*> pathArray;
	std::unordered_set<PlanarGraphEdge> edgeSet;

	const Node* inComingNode = nullptr;
	const Node* terminalNode = nullptr;

	while (node)
	{
		pathArray.push_back(node);

		if (pathArray.size() > 2 * this->nodeArray.size())
			return false;

		const Node* outGoingNode = node->FindOutGoingNode(inComingNode, this->plane.unitNormal);

		PlanarGraphEdge edge{ node->i, outGoingNode->i };
		if (edgeSet.find(edge) != edgeSet.end())
		{
			terminalNode = pathArray.back();
			pathArray.pop_back();
			break;
		}

		edgeSet.insert(edge);

		inComingNode = node;
		node = outGoingNode;
	}

	bool foundTerminal = false;
	cycleArray.clear();
	for (int j = 0; j < (signed)pathArray.size(); j++)
	{
		if (pathArray[j] == terminalNode)
			foundTerminal = true;

		if (foundTerminal)
			cycleArray.push_back(pathArray[j]);
	}

	return true;
}

void PlanarGraph::RegenerateNodeAdjacencies() const
{
	for (int i = 0; i < (signed)this->nodeArray.size(); i++)
		this->nodeArray[i]->i = i;

	for (const Node* node : this->nodeArray)
		node->adjacentNodeArray.clear();

	for (const PlanarGraphEdge& edge : this->edgeSet)
	{
		const Node* nodeA = this->nodeArray[edge.i];
		const Node* nodeB = this->nodeArray[edge.j];

		nodeA->adjacentNodeArray.push_back(nodeB);
	}
}

//---------------------------- PlanarGraph::Node ----------------------------

PlanarGraph::Node::Node()
{
	this->i = -1;
}

/*virtual*/ PlanarGraph::Node::~Node()
{
}

bool PlanarGraph::Node::HasAdjacency(const Node* node) const
{
	for (const Node* adjacentNode : this->adjacentNodeArray)
		if (adjacentNode == node)
			return true;

	return false;
}

const PlanarGraph::Node* PlanarGraph::Node::FindOutGoingNode(const Node* inComingNode, const Vector3& planeNormal) const
{
	if (this->adjacentNodeArray.size() == 0)
		return nullptr;

	if (!inComingNode)
		return this->adjacentNodeArray[0];	// Choose the first one arbitrarily.

	Vector3 vectorA = (inComingNode->vertex - this->vertex).Normalized();

	struct Choice
	{
		const Node* node;
		double angle;
		bool bidirectional;
	};

	std::vector<Choice> choiceArray;

	const PlanarGraph::Node* outGoingNode = nullptr;
	double maxAngle = 0.0;
	for (const Node* adjacentNode : this->adjacentNodeArray)
	{
		if (adjacentNode == inComingNode)
			continue;

		Vector3 vectorB = (adjacentNode->vertex - this->vertex).Normalized();

		Choice choice;
		choice.node = adjacentNode;
		choice.angle = vectorB.AngleBetween(vectorA, planeNormal);
		choice.bidirectional = adjacentNode->HasAdjacency(this);
		choiceArray.push_back(choice);
	}

	std::sort(choiceArray.begin(), choiceArray.end(), [](const Choice& choiceA, const Choice& choiceB) -> bool {
		return choiceA.angle < choiceB.angle;
	});

	for (const Choice& choice : choiceArray)
	{
		outGoingNode = choice.node;
		if (choice.bidirectional)
			break;
	}

	return outGoingNode;
}