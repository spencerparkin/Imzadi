#include "Graph.h"
#include "PolygonMesh.h"
#include "Polygon.h"
#include "Function.h"
#include <map>
#include <algorithm>

using namespace Imzadi;

//--------------------------------- Graph ---------------------------------

Graph::Graph()
{
}

Graph::Graph(const Graph& graph)
{
	*this = graph;
}

/*virtual*/ Graph::~Graph()
{
	this->Clear();
}

void Graph::operator=(const Graph& graph)
{
	this->Clear();

	graph.AssignIndicesForNodes();

	for (const Node* otherNode : graph.nodeArray)
	{
		auto node = new Node();
		node->vertex = otherNode->vertex;
		node->normal = otherNode->normal;
		node->i = otherNode->i;
		this->nodeArray.push_back(node);
	}

	for (const Node* otherNode : graph.nodeArray)
	{
		Node* node = this->nodeArray[otherNode->i];

		for (const Node* otherAdjacentNode : otherNode->adjacentNodeSet)
		{
			Node* adjacentNode = this->nodeArray[otherAdjacentNode->i];
			node->adjacentNodeSet.insert(adjacentNode);
		}
	}
}

void Graph::Clear()
{
	for (Node* node : this->nodeArray)
		delete node;

	this->nodeArray.clear();
}

bool Graph::FromPolygohMesh(const PolygonMesh& mesh)
{
	this->Clear();

	for(int i = 0; i < mesh.GetNumVertices(); i++)
	{
		auto node = new Node();
		node->vertex = mesh.GetVertex(i);
		node->normal.SetComponents(0.0, 0.0, 0.0);
		this->nodeArray.push_back(node);
	}

	for (const PolygonMesh::Polygon& polygon : mesh.GetPolygonArray())
	{
		for (int i = 0; i < (signed)polygon.vertexArray.size(); i++)
		{
			int j = polygon.Mod(i + 1);

			Node* nodeA = this->nodeArray[i];
			Node* nodeB = this->nodeArray[j];

			if (!nodeA->IsAdjacentTo(nodeB))
				nodeA->adjacentNodeSet.insert(nodeB);

			if (!nodeB->IsAdjacentTo(nodeA))
				nodeB->adjacentNodeSet.insert(nodeA);
		}
	}

	for (const PolygonMesh::Polygon& polygon : mesh.GetPolygonArray())
	{
		Polygon standalonePolygon;
		polygon.ToStandalonePolygon(standalonePolygon, &mesh);
		Plane plane = standalonePolygon.CalcPlane();
		for (int i : polygon.vertexArray)
			this->nodeArray[i]->normal += plane.unitNormal;
	}

	for (Node* node : this->nodeArray)
		if (!node->normal.Normalize())
			return false;

	return true;
}

bool Graph::ToPolygonMesh(PolygonMesh& mesh) const
{
	Graph graph = *this;
	return graph.ToPolygonMesh(mesh);
}

bool Graph::ToPolygonMesh(PolygonMesh& mesh)
{
	mesh.Clear();

	for (const Node* node : this->nodeArray)
		mesh.AddVertex(node->vertex);

	this->AssignIndicesForNodes();

	PolygonMesh::Polygon polygon;
	while (this->FindAndRemovePolygonCycleForMesh(polygon.vertexArray))
		mesh.AddPolygon(polygon);

	for (const Node* node : this->nodeArray)
		if (node->GetNumAdjacencies() > 0)
			return false;

	return true;
}

void Graph::AssignIndicesForNodes() const
{
	for (int i = 0; i < (signed)this->nodeArray.size(); i++)
		this->nodeArray[i]->i = i;
}

bool Graph::FindAndRemovePolygonCycleForMesh(std::vector<int>& cycleArray)
{
	cycleArray.clear();

	Node* initialNode = nullptr;
	for (Node* node : this->nodeArray)
	{
		if (node->adjacentNodeSet.size() > 0)
		{
			initialNode = node;
			break;
		}
	}

	if (!initialNode)
		return false;

	Node* nodeIn = nullptr;
	Node* nodeOut = nullptr;
	Node* node = initialNode;

	do
	{
		for (int i : cycleArray)
			if (cycleArray[i] == node->i)
				return false;

		cycleArray.push_back(node->i);

		if (node->adjacentNodeSet.size() == 0)
			return false;

		Node* chosenNode = *node->adjacentNodeSet.begin();
		if (nodeIn)
		{
			Vector3 vectorIn = (nodeIn->vertex - node->vertex).RejectedFrom(node->normal).Normalized();
			double smallestAngle = std::numeric_limits<double>::max();
			for (Node* adjacentNode : node->adjacentNodeSet)
			{
				nodeOut = adjacentNode;
				Vector3 vectorOut = (nodeOut->vertex - node->vertex).RejectedFrom(node->normal).Normalized();
				double angle = vectorOut.AngleBetween(vectorIn, node->normal);
				if (angle > smallestAngle)
				{
					smallestAngle = angle;
					chosenNode = nodeOut;
				}
			}
		}
		
		nodeOut = chosenNode;
		node->adjacentNodeSet.erase(nodeOut);
		nodeIn = node;
		node = nodeOut;
		nodeOut = nullptr;
	} while (node->i != initialNode->i);

	return true;
}

bool Graph::ReduceEdgeCount(int numEdgesToRemove)
{
	// I'm not sure if this algorithm will be any good.
	// It's something to try.

	if (numEdgesToRemove <= 0)
		return true;

	std::set<UnorderedEdge, UnorderedEdge> edgeSet;
	this->GenerateEdgeSet<UnorderedEdge>(edgeSet);

	std::vector<UnorderedEdge> edgeArray;
	for (auto edge : edgeSet)
		edgeArray.push_back(edge);

	std::sort(edgeArray.begin(), edgeArray.end(), [this](const UnorderedEdge& edgeA, const UnorderedEdge& edgeB) -> bool
	{
		double lengthA = this->CalcEdgeLength(edgeA);
		double lengthB = this->CalcEdgeLength(edgeB);
		return lengthA < lengthB;
	});

	for (int i = 0; i < (signed)edgeArray.size(); i++)
	{
		if (numEdgesToRemove == 0)
			break;

		const Edge& edge = edgeArray[i];
		Node* nodeA = this->nodeArray[edge.i];
		Node* nodeB = this->nodeArray[edge.j];
		Node* newNode = this->MergeVertices(nodeA, nodeB);
		if (newNode)
			numEdgesToRemove--;
	}

	return numEdgesToRemove == 0;
}

double Graph::CalcEdgeLength(const Edge& edge) const
{
	return (this->nodeArray[edge.i]->vertex - this->nodeArray[edge.j]->vertex).Length();
}

Graph::Node* Graph::MergeVertices(Node* nodeA, Node* nodeB)
{
	if (!nodeA->IsAdjacentTo(nodeB) || !nodeB->IsAdjacentTo(nodeA))
		return nullptr;

	Vector3 unitDirection = nodeB->vertex - nodeA->vertex;
	if (!unitDirection.Normalize())
		return nullptr;

	Node* nodeU = nodeA->FindAdjacencyInDirection(-unitDirection);
	Node* nodeV = nodeB->FindAdjacencyInDirection(unitDirection);

	if (!nodeU || !nodeV)
		return nullptr;

	if (nodeU == nodeB || nodeV == nodeA)
		return nullptr;

	CubicSpaceCurve spaceCurve;
	if (!spaceCurve.FitToPoints(nodeU->vertex, nodeA->vertex, nodeB->vertex, nodeV->vertex, 0.0, 1.0 / 3.0, 2.0 / 3.0, 1.0))
		return nullptr;

	auto newNode = new Node();
	newNode->vertex = spaceCurve.Evaluate(0.5);
	newNode->normal = (nodeA->normal + nodeB->normal).Normalized();
	this->nodeArray.push_back(newNode);

	this->DeleteNode(nodeA, newNode);
	this->DeleteNode(nodeB, newNode);

	return newNode;
}

void Graph::DeleteNode(Node* node, Node* alternativeNode /*= nullptr*/)
{
	for (Node* existingNode : this->nodeArray)
	{
		if (existingNode->IsAdjacentTo(node))
		{
			existingNode->adjacentNodeSet.erase(node);
			if (alternativeNode)
				existingNode->adjacentNodeSet.insert(alternativeNode);
		}
	}

	for (int i = 0; i < (signed)this->nodeArray.size(); i++)
	{
		if (nodeArray[i] == node)
		{
			if (i != (signed)this->nodeArray.size() - 1)
				this->nodeArray[i] = this->nodeArray[this->nodeArray.size() - 1];
			this->nodeArray.pop_back();
			break;
		}
	}

	delete node;
}

//--------------------------------- Graph::Node ---------------------------------

Graph::Node::Node()
{
	this->i = -1;
}

/*virtual*/ Graph::Node::~Node()
{
}

bool Graph::Node::IsAdjacentTo(const Node* node) const
{
	return this->adjacentNodeSet.find(const_cast<Node*>(node)) != this->adjacentNodeSet.end();
}

Graph::Node* Graph::Node::FindAdjacencyInDirection(const Vector3& unitDirection)
{
	Node* foundNode = nullptr;
	double minAngle = std::numeric_limits<double>::max();
	for (Node* adjacentNode : this->adjacentNodeSet)
	{
		Vector3 unitAdjacentNodeDirection = (adjacentNode->vertex - this->vertex).Normalized();
		double angle = unitDirection.AngleBetween(unitAdjacentNodeDirection);
		if (angle < minAngle)
		{
			minAngle = angle;
			foundNode = adjacentNode;
		}
	}

	return foundNode;
}