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

	// TODO: Write this.
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
				nodeA->adjacentNodeArray.push_back(nodeB);

			if (!nodeB->IsAdjacentTo(nodeA))
				nodeB->adjacentNodeArray.push_back(nodeA);
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

	for (int i = 0; i < (signed)this->nodeArray.size(); i++)
		this->nodeArray[i]->i = i;

	PolygonMesh::Polygon polygon;
	while (this->FindAndRemovePolygonCycleForMesh(polygon.vertexArray))
		mesh.AddPolygon(polygon);

	for (const Node* node : this->nodeArray)
		if (node->adjacentNodeArray.size() > 0)
			return false;

	return true;
}

bool Graph::FindAndRemovePolygonCycleForMesh(std::vector<int>& cycleArray)
{
	cycleArray.clear();

	Node* initialNode = nullptr;
	for (Node* node : this->nodeArray)
	{
		if (node->adjacentNodeArray.size() > 0)
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

		if (node->adjacentNodeArray.size() == 0)
			return false;

		int i = 0;
		if (nodeIn)
		{
			Vector3 vectorIn = (nodeIn->vertex - node->vertex).RejectedFrom(node->normal).Normalized();
			double smallestAngle = std::numeric_limits<double>::max();
			for (int j = 0; j < (signed)node->adjacentNodeArray.size(); j++)
			{
				nodeOut = node->adjacentNodeArray[j];
				Vector3 vectorOut = (nodeOut->vertex - node->vertex).RejectedFrom(node->normal).Normalized();
				double angle = vectorOut.AngleBetween(vectorIn, node->normal);
				if (angle > smallestAngle)
				{
					smallestAngle = angle;
					i = j;
				}
			}
		}
		
		nodeOut = node->adjacentNodeArray[i];
		node->adjacentNodeArray.erase(node->adjacentNodeArray.begin() + i);
		nodeIn = node;
		node = nodeOut;
		nodeOut = nullptr;
	} while (node->i != initialNode->i);

	return true;
}

void Graph::ModifyDetail(double percentage)
{
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
	for (int i = 0; i < (signed)this->adjacentNodeArray.size(); i++)
		if (this->adjacentNodeArray[i] == node)
			return true;

	return false;
}