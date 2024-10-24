#include "NavGraph.h"
#include "Log.h"
#include "RenderObjects/DebugLines.h"
#include <queue>
#include <unordered_map>

using namespace Imzadi;

//------------------------------------- NavGraph -------------------------------------

NavGraph::NavGraph()
{
}

/*virtual*/ NavGraph::~NavGraph()
{
	this->Clear();
}

/*virtual*/ bool NavGraph::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	if (!jsonDoc.IsObject())
	{
		IMZADI_LOG_ERROR("Expected nav-graph JSON doc to be an object.");
		return false;
	}

	this->Clear();

	if (!jsonDoc.HasMember("node_array") || !jsonDoc["node_array"].IsArray())
	{
		IMZADI_LOG_ERROR("The \"node_array\" member was not found or it was not an array.");
		return false;
	}

	if (!jsonDoc.HasMember("path_array") || !jsonDoc["path_array"].IsArray())
	{
		IMZADI_LOG_ERROR("The \"path_array\" member was not found or it was not an array.");
		return false;
	}

	const rapidjson::Value& nodeArrayValue = jsonDoc["node_array"];
	const rapidjson::Value& pathArrayValue = jsonDoc["path_array"];

	for (int i = 0; i < nodeArrayValue.Size(); i++)
	{
		const rapidjson::Value& nodeValue = nodeArrayValue[i];

		Node* node = new Node();
		this->nodeArray.push_back(node);

		if (!nodeValue.IsObject())
		{
			IMZADI_LOG_ERROR("Node %d is not an object in the JSON.", i);
			return false;
		}

		if (!nodeValue.HasMember("location"))
		{
			IMZADI_LOG_ERROR("Node %d has no \"location\" value in the JSON.", i);
			return false;
		}

		Asset::LoadVector(nodeValue["location"], node->location);
	}

	for (int i = 0; i < pathArrayValue.Size(); i++)
	{
		const rapidjson::Value& pathValue = pathArrayValue[i];

		Path* path = new Path();
		this->pathArray.push_back(path);

		if (!pathValue.IsObject())
		{
			IMZADI_LOG_ERROR("Path %d is not an object in the JSON.", i);
			return false;
		}

		if (!pathValue.HasMember("i") || !pathValue["i"].IsInt() || !pathValue.HasMember("j") || !pathValue["j"].IsInt())
		{
			IMZADI_LOG_ERROR("For path %d, both \"i\" and \"j\" members must be integers.", i);
			return false;
		}

		int index_i = pathValue["i"].GetInt();
		int index_j = pathValue["j"].GetInt();

		if (!(0 <= index_i && index_i < (int)this->nodeArray.size()) ||
			!(0 <= index_j && index_j < (int)this->nodeArray.size()))
		{
			IMZADI_LOG_ERROR("Path %d has node indices (%d, %d) out of range.", i, index_i, index_j);
			return false;
		}

		path->terminalNode[0] = this->nodeArray[index_i];
		path->terminalNode[1] = this->nodeArray[index_j];

		path->UpdateLength();
	}

	for (int i = 0; i < nodeArrayValue.Size(); i++)
	{
		const rapidjson::Value& nodeValue = nodeArrayValue[i];

		if (!nodeValue.HasMember("adj_path_array"))
			continue;

		const rapidjson::Value& adjPathArrayValue = nodeValue["adj_path_array"];
		if (!adjPathArrayValue.IsArray())
		{
			IMZADI_LOG_ERROR("For node %d, expected \"adj_path_array\" member to be an array.", i);
			return false;
		}

		Node* node = this->nodeArray[i];

		for (int j = 0; j < adjPathArrayValue.Size(); j++)
		{
			const rapidjson::Value& adjPathValue = adjPathArrayValue[j];
			if (!adjPathValue.IsInt())
			{
				IMZADI_LOG_ERROR("For node %d, adjacency %d, it is not an integer as expected.", i, j);
				return false;
			}

			int index = adjPathValue.GetInt();
			if (!(0 <= index && index < (int)this->pathArray.size()))
			{
				IMZADI_LOG_ERROR("For node %d, adjacency %d, the index %d is out of range.", i, j, index);
				return false;
			}

			node->adjacentPathArray.push_back(this->pathArray[index]);
		}
	}

	if (!this->IsValid())
	{
		IMZADI_LOG_ERROR("The loaded nav-graph did not check-out as valid.");
		return false;
	}

	return true;
}

/*virtual*/ bool NavGraph::Save(rapidjson::Document& jsonDoc) const
{
	std::unordered_map<const Node*, int> nodeIndexMap;
	std::unordered_map<const Path*, int> pathIndexMap;

	for (int i = 0; i < (int)this->nodeArray.size(); i++)
		nodeIndexMap.insert(std::pair(this->nodeArray[i].Get(), i));

	for (int i = 0; i < (int)this->pathArray.size(); i++)
		pathIndexMap.insert(std::pair(this->pathArray[i].Get(), i));

	jsonDoc.SetObject();

	rapidjson::Value nodeArrayValue;
	nodeArrayValue.SetArray();

	rapidjson::Value pathArrayValue;
	pathArrayValue.SetArray();

	for (const Reference<Node>& node : this->nodeArray)
	{
		rapidjson::Value nodeValue;
		nodeValue.SetObject();

		rapidjson::Value locationValue;
		Asset::SaveVector(locationValue, node->location, &jsonDoc);

		rapidjson::Value adjacentPathArrayValue;
		adjacentPathArrayValue.SetArray();

		for (int i = 0; i < (int)node->adjacentPathArray.size(); i++)
		{
			const Path* path = node->adjacentPathArray[i];
			int j = pathIndexMap.find(path)->second;
			adjacentPathArrayValue.PushBack(rapidjson::Value().SetInt(j), jsonDoc.GetAllocator());
		}

		nodeValue.AddMember("location", locationValue, jsonDoc.GetAllocator());
		nodeValue.AddMember("adj_path_array", adjacentPathArrayValue, jsonDoc.GetAllocator());

		nodeArrayValue.PushBack(nodeValue, jsonDoc.GetAllocator());
	}

	for (const Reference<Path>& path : this->pathArray)
	{
		rapidjson::Value pathValue;
		pathValue.SetObject();

		int i = nodeIndexMap.find(path->terminalNode[0].Get())->second;
		int j = nodeIndexMap.find(path->terminalNode[1].Get())->second;

		pathValue.AddMember("i", rapidjson::Value().SetInt(i), jsonDoc.GetAllocator());
		pathValue.AddMember("j", rapidjson::Value().SetInt(j), jsonDoc.GetAllocator());

		pathArrayValue.PushBack(pathValue, jsonDoc.GetAllocator());
	}

	jsonDoc.AddMember("node_array", nodeArrayValue, jsonDoc.GetAllocator());
	jsonDoc.AddMember("path_array", pathArrayValue, jsonDoc.GetAllocator());

	return true;
}

/*virtual*/ bool NavGraph::Unload()
{
	this->Clear();
	return true;
}

void NavGraph::Clear()
{
	for (Reference<Node>& node : this->nodeArray)
		node->Clear();

	for (Reference<Path>& path : this->pathArray)
		path->Clear();

	this->nodeArray.clear();
	this->pathArray.clear();
}

bool NavGraph::IsValid() const
{
	for (int i = 0; i < (int)this->pathArray.size(); i++)
	{
		const Reference<Path>& path = this->pathArray[i];

		if (!path->terminalNode[0] || !path->terminalNode[1])
			return false;

		if (!path->terminalNode[0]->IsAdjacentTo(path->terminalNode[1]))
			return false;

		if (!path->terminalNode[1]->IsAdjacentTo(path->terminalNode[0]))
			return false;
	}

	return true;
}

bool NavGraph::AddPathSegment(const Polygon& worldPolygon, double snapTolerance /*= 0.5*/)
{
	if (worldPolygon.vertexArray.size() < 2)
		return false;

	Vector3 upVector(0.0, 1.0, 0.0);
	Plane plane = worldPolygon.CalcPlane(true);
	double angle = plane.unitNormal.AngleBetween(upVector);
	if (angle > M_PI / 3.0)
		return false;

	std::vector<LineSegment> edgeArray;
	worldPolygon.GetEdges(edgeArray);

	std::sort(edgeArray.begin(), edgeArray.end(), [](const LineSegment& edgeA, const LineSegment& edgeB) -> bool
		{
			return edgeA.Length() > edgeB.Length();
		});

	LineSegment edgeA = edgeArray[0];
	LineSegment edgeB = edgeArray[1];

	LineSegment pathSegment;
	pathSegment.point[0] = (edgeA.point[0] + edgeB.point[1]) / 2.0;
	pathSegment.point[1] = (edgeA.point[1] + edgeB.point[0]) / 2.0;

	return this->AddPathSegment(pathSegment);
}

bool NavGraph::AddPathSegment(const LineSegment& pathSegment, double snapTolerance /*= 0.5*/)
{
	Node* nodeA = this->AddLocation(pathSegment.point[0], snapTolerance);
	Node* nodeB = this->AddLocation(pathSegment.point[1], snapTolerance);

	IMZADI_ASSERT(nodeA && nodeB);

	return this->AddPathBetweenNodes(nodeA, nodeB);
}

NavGraph::Node* NavGraph::AddLocation(const Vector3& location, double snapTolerance /*= 0.5*/)
{
	Node* node = const_cast<Node*>(this->FindNearestNodeWithinDistance(location, snapTolerance));
	if (!node)
	{
		node = new Node();
		node->location = location;
		this->nodeArray.push_back(node);

		for (int i = 0; i < (int)this->pathArray.size(); i++)
		{
			Reference<Path> path = this->pathArray[i];
			LineSegment pathSegment = path->GetPathSegment();
			double distance = pathSegment.ShortestDistanceTo(location);
			if (distance <= snapTolerance)
			{
				this->RemovePath(i);

				this->AddPathBetweenNodes(path->terminalNode[0], node);
				this->AddPathBetweenNodes(path->terminalNode[1], node);

				break;
			}
		}
	}

	return node;
}

bool NavGraph::AddPathBetweenNodes(Node* nodeA, Node* nodeB)
{
	if (nodeA->IsAdjacentTo(nodeB))
		return false;

	IMZADI_ASSERT(!nodeB->IsAdjacentTo(nodeA));

	Reference<Path> path(new Path());
	path->terminalNode[0].Set(nodeA);
	path->terminalNode[1].Set(nodeB);
	this->pathArray.push_back(path);
	nodeA->adjacentPathArray.push_back(path);
	nodeB->adjacentPathArray.push_back(path);
	return true;
}

bool NavGraph::RemovePathBetweenNodes(Node* nodeA, Node* nodeB)
{
	if (!nodeA->IsAdjacentTo(nodeB))
		return false;

	IMZADI_ASSERT(nodeB->IsAdjacentTo(nodeA));

	int i = this->FindPathJoiningNodes(nodeA, nodeB);
	IMZADI_ASSERT(i != -1);

	return this->RemovePath(i);
}

int NavGraph::FindPathJoiningNodes(Node* nodeA, Node* nodeB)
{
	for (int i = 0; i < (int)this->pathArray.size(); i++)
	{
		Path* path = this->pathArray[i];
		if (path->JoinsNodes(nodeA, nodeB))
			return i;
	}

	return -1;
}

bool NavGraph::RemovePath(int i)
{
	if (i < 0 || i >= (int)this->pathArray.size())
		return false;

	Path* path = this->pathArray[i];
	bool removed0 = path->terminalNode[0]->RemovePath(path);
	bool removed1 = path->terminalNode[1]->RemovePath(path);
	IMZADI_ASSERT(removed0 && removed1);

	if (i < int(this->pathArray.size() - 1))
		this->pathArray[i].Set(this->pathArray[this->pathArray.size() - 1]);

	this->pathArray.pop_back();
	return true;
}

void NavGraph::DebugDraw(DebugLines& debugLines) const
{
	for (const Reference<Path>& path : this->pathArray)
	{
		DebugLines::Line line;
		line.color.SetComponents(1.0, 0.0, 0.0);
		line.segment = path->GetPathSegment();
		debugLines.AddLine(line);
	}
}

const NavGraph::Node* NavGraph::FindNearestNode(const Vector3& location) const
{
	double minSquareDistance = std::numeric_limits<double>::max();
	const Node* foundNode = nullptr;
	for (const Reference<Node>& node : this->nodeArray)
	{
		double squareDistance = (node->location - location).SquareLength();
		if (squareDistance < minSquareDistance)
		{
			minSquareDistance = squareDistance;
			foundNode = node;
		}
	}

	return foundNode;
}

const NavGraph::Node* NavGraph::FindNearestNodeWithinDistance(const Vector3& location, double maxDistance) const
{
	const Node* foundNode = this->FindNearestNode(location);
	if (!foundNode)
		return nullptr;

	double distance = (foundNode->location - location).Length();
	return (distance <= maxDistance) ? foundNode : nullptr;
}

const NavGraph::Path* NavGraph::FindNearestPath(const Vector3& location, int* i /*= nullptr*/) const
{
	double shortestDistance = std::numeric_limits<double>::max();
	const Path* foundPath = nullptr;
	for (const Path* path : this->pathArray)
	{
		double distance = path->GetPathSegment().ShortestDistanceTo(location);
		if (distance < shortestDistance)
		{
			foundPath = path;
			shortestDistance = distance;
		}
	}

	if (i)
	{
		if (!foundPath)
			*i = -1;
		else
		{
			double distanceA = (location - foundPath->terminalNode[0]->location).Length();
			double distanceB = (location - foundPath->terminalNode[1]->location).Length();
			*i = (distanceA < distanceB) ? 0 : 1;
		}
	}

	return foundPath;
}

// See Chapter 25, Single-Source Shortest Paths of "Intro to Algorithms" by Rivest, et. al.
bool NavGraph::FindShortestPath(const Node* nodeA, const Node* nodeB, std::list<int>& pathList) const
{
	pathList.clear();
	if (nodeA == nodeB)
		return true;

	for (const Reference<Node>& node : this->nodeArray)
	{
		node->distance = (node.Get() == nodeA) ? 0.0 : std::numeric_limits<double>::max();
		node->parentNode = nullptr;
	}

	struct NodeCompare
	{
		bool operator()(const Node* left, const Node* right) const
		{
			return left->distance > right->distance;
		}
	};

	// Note that we can't terminate early here if we run into node B.
	// We have to keep going until we've found a spanning tree of the whole graph.
	// This is because parent nodes can get both assigned and re-assigned during
	// the course of the algorithm.
	std::priority_queue<const Node*, std::vector<const Node*>, NodeCompare> queue;
	queue.push(nodeA);
	while (queue.size() > 0)
	{
		const Node* parentNode = queue.top();
		queue.pop();

		for (const Reference<Path>& path : parentNode->adjacentPathArray)
		{
			const Node* childNode = path->Follow(parentNode);
			double distance = parentNode->distance + path->length;
			if (distance < childNode->distance)
			{
				childNode->distance = distance;
				childNode->parentNode = parentNode;
				queue.push(childNode);
			}
		}
	}

	while (nodeB->parentNode)
	{
		const Node* parentNode = nodeB->parentNode;
		int i = parentNode->FindAdjacencyIndex(nodeB);
		IMZADI_ASSERT(i != -1);
		pathList.push_front(i);
		nodeB = parentNode;
	}

	return pathList.size() > 0;
}

const NavGraph::Node* NavGraph::GetRandomNode(Random& random) const
{
	if (this->nodeArray.size() == 0)
		return nullptr;

	int i = random.InRange(0, this->nodeArray.size() - 1);
	return this->nodeArray[i];
}

//------------------------------------- NavGraph::Node -------------------------------------

NavGraph::Node::Node()
{
	this->distance = 0.0;
	this->parentNode = nullptr;
}

/*virtual*/ NavGraph::Node::~Node()
{
}

void NavGraph::Node::Clear()
{
	this->adjacentPathArray.clear();
}

bool NavGraph::Node::IsAdjacentTo(const Node* node) const
{
	return this->FindAdjacencyIndex(node) != -1;
}

int NavGraph::Node::FindAdjacencyIndex(const Node* adjacentNode) const
{
	for (int i = 0; i < (int)this->adjacentPathArray.size(); i++)
		if (this->adjacentPathArray[i]->Follow(this) == adjacentNode)
			return i;
	
	return -1;
}

bool NavGraph::Node::RemovePath(const Path* path)
{
	for (int i = 0; i < (int)this->adjacentPathArray.size(); i++)
	{
		if (this->adjacentPathArray[i] == path)
		{
			if (i + 1 < (int)this->adjacentPathArray.size())
				this->adjacentPathArray[i] = this->adjacentPathArray[this->adjacentPathArray.size() - 1];

			this->adjacentPathArray.pop_back();
			return true;
		}
	}

	return false;
}

const NavGraph::Node* NavGraph::Node::GetAdjacentNode(int i) const
{
	IMZADI_ASSERT(0 <= i && i < (int)this->adjacentPathArray.size());
	return this->adjacentPathArray[i]->Follow(this);
}

//------------------------------------- NavGraph::Path -------------------------------------

NavGraph::Path::Path()
{
	this->length = 0.0;
}

/*virtual*/ NavGraph::Path::~Path()
{
}

void NavGraph::Path::Clear()
{
	this->terminalNode[0].Reset();
	this->terminalNode[1].Reset();
}

void NavGraph::Path::UpdateLength() const
{
	this->length = (this->terminalNode[0]->location - this->terminalNode[1]->location).Length();
}

const NavGraph::Node* NavGraph::Path::Follow(const Node* node) const
{
	if (node == this->terminalNode[0])
		return this->terminalNode[1];

	if (node == this->terminalNode[1])
		return this->terminalNode[0];

	return nullptr;
}

LineSegment NavGraph::Path::GetPathSegment() const
{
	LineSegment pathSegment;
	pathSegment.point[0] = this->terminalNode[0]->location;
	pathSegment.point[1] = this->terminalNode[1]->location;
	return pathSegment;
}

bool NavGraph::Path::JoinsNodes(const Node* nodeA, const Node* nodeB) const
{
	if (this->terminalNode[0] == nodeA && this->terminalNode[1] == nodeB)
		return true;

	if (this->terminalNode[1] == nodeA && this->terminalNode[0] == nodeB)
		return true;

	return false;
}