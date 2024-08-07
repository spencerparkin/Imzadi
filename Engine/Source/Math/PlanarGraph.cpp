#include "PlanarGraph.h"
#include "LineSegment.h"

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
	for (const Vector3& vertex : polygon.vertexArray)
		if (!this->AddVertex(vertex))
			return false;

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
			{
				this->edgeSet.erase(existingEdge);
				addNewEdge = false;
				break;
			}

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

LineSegment PlanarGraph::MakeLineSegment(const PlanarGraphEdge& edge) const
{
	LineSegment lineSeg;
	lineSeg.point[0] = this->nodeArray[edge.i]->vertex;
	lineSeg.point[1] = this->nodeArray[edge.j]->vertex;
	return lineSeg;
}

void PlanarGraph::ExtractAllPolygons(std::vector<Polygon>& polygonArray) const
{
	this->RegenerateNodeAdjacencies();

	std::unordered_set<const Node*> pendingNodesSet;
	for (const Node* node : this->nodeArray)
		pendingNodesSet.insert(node);

	std::unordered_set<const Node*> usedNodeSet;

	// I can see this failing to give me exactly what I want in
	// the case of nested polygons, but this may be good enough.
	// Nested polygons shouldn't show up in my particular use-case.
	while (pendingNodesSet.size() > 0)
	{
		const Node* node = *pendingNodesSet.begin();
		pendingNodesSet.erase(node);

		bool addCycle = true;
		std::vector<const Node*> cycleArray;
		if (this->FindOuterCycle(node, cycleArray))
		{
			for (const Node* cycleNode : cycleArray)
			{
				pendingNodesSet.erase(cycleNode);

				if (usedNodeSet.find(cycleNode) != usedNodeSet.end())
					addCycle = false;
				else
					usedNodeSet.insert(cycleNode);
			}
		}

		if (addCycle)
		{
			Polygon polygon;
			for (const Node* node : cycleArray)
				polygon.vertexArray.push_back(node->vertex);

			polygonArray.push_back(polygon);
		}
	}
}

bool PlanarGraph::FindOuterCycle(const Node* node, std::vector<const Node*>& cycleArray) const
{
	std::unordered_set<const Node*> visitedNodesSet;
	std::vector<const Node*> pathArray;

	const Node* inComingNode = nullptr;
	int i = -1;

	while (node)
	{
		if (visitedNodesSet.find(node) != visitedNodesSet.end())
		{
			for (i = 0; i < (signed)pathArray.size(); i++)
				if (pathArray[i] == node)
					break;

			IMZADI_ASSERT(i != (signed)pathArray.size());
			break;
		}

		visitedNodesSet.insert(node);
		pathArray.push_back(node);

		const Node* outGoingNode = node->FindOutGoingNode(inComingNode, this->plane.unitNormal);
		inComingNode = node;
		node = outGoingNode;
	}

	cycleArray.clear();
	for (int j = i; j < (signed)pathArray.size(); j++)
		cycleArray.push_back(pathArray[j]);

	return true;
}

void PlanarGraph::RegenerateNodeAdjacencies() const
{
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
}

/*virtual*/ PlanarGraph::Node::~Node()
{
}

const PlanarGraph::Node* PlanarGraph::Node::FindOutGoingNode(const Node* inComingNode, const Vector3& planeNormal) const
{
	if (this->adjacentNodeArray.size() == 0)
		return nullptr;

	if (!inComingNode)
		return this->adjacentNodeArray[0];	// Choose the first one arbitrarily.

	Vector3 vectorA = (inComingNode->vertex - this->vertex).Normalized();

	const PlanarGraph::Node* outGoingNode = nullptr;
	double maxAngle = 0.0;
	for (const Node* adjacentNode : this->adjacentNodeArray)
	{
		Vector3 vectorB = (adjacentNode->vertex - this->vertex).Normalized();

		double angle = vectorA.AngleBetween(vectorB);

		if (vectorA.Cross(vectorB).Dot(planeNormal) > 0.0)
			angle = 2.0 * M_PI - angle;

		if (angle > maxAngle)
		{
			maxAngle = angle;
			outGoingNode = adjacentNode;
		}
	}

	return outGoingNode;
}