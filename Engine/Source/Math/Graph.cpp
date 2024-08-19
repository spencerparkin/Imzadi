#include "Graph.h"
#include "PolygonMesh.h"
#include "Polygon.h"
#include <map>

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

void Graph::ReduceEdgeCount(int numEdgesToRemove)
{
	if (numEdgesToRemove <= 0)
		return;

	// TODO: Think about it.
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