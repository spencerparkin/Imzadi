#pragma once

#include "AssetCache.h"
#include "Math/Vector3.h"
#include "Math/Polygon.h"
#include "Math/Random.h"
#include "Reference.h"

namespace Imzadi
{
	class DebugLines;

	/**
	 * These are spacial graphs designed for the purpose of helping entities
	 * navigate a level.  Spacial, because each vertex represents a location
	 * in space, while the edges between vertices indicate paths that can be
	 * taken from one place to another.  It is easier for an entity to follow
	 * a graph rather than continuously sense the world through the collision
	 * system.  Further, trying to find shortest paths, etc., without such a
	 * data-structure is not really feasible.
	 * 
	 * For my purposes, these are undirected graphs.  That is, if an edge joins
	 * two vertices, then the vertices are adjacent to one another.
	 */
	class IMZADI_API NavGraph : public Asset
	{
	public:
		NavGraph();
		virtual ~NavGraph();

		virtual bool Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache) override;
		virtual bool Save(rapidjson::Document& jsonDoc) const override;
		virtual bool Unload() override;

		/**
		 * Make sure that this data-structure is sound.  Specifically,
		 * if a node points to an edge, then the edge should point to
		 * the node, and vice-versa.
		 */
		bool IsValid() const;

		class Path;

		/**
		 * These are the vertices of the nav-graph.
		 */
		class IMZADI_API Node : public ReferenceCounted
		{
		public:
			Node();
			virtual ~Node();

			void Clear();
			bool IsAdjacentTo(const Node* node) const;
			int FindAdjacencyIndex(const Node* adjacentNode) const;
			bool RemovePath(const Path* path);
			const Node* GetAdjacentNode(int i) const;

		public:
			Vector3 location;
			std::vector<Reference<Path>> adjacentPathArray;
			mutable double distance;
			mutable const Node* parentNode;
		};

		/**
		 * These are the edges of the nav-graph.
		 */
		class IMZADI_API Path : public ReferenceCounted
		{
		public:
			Path();
			virtual ~Path();

			void Clear();
			const Node* Follow(const Node* node) const;
			bool JoinsNodes(const Node* nodeA, const Node* nodeB) const;
			LineSegment GetPathSegment() const;
			void UpdateLength() const;

		public:
			Reference<Node> terminalNode[2];
			mutable double length;
		};

		/**
		 * Free all memory and reset this graph to the empty-set graph.
		 */
		void Clear();

		/**
		 * Approximate the given polygon with a line-segment, and then add that line-segment
		 * to the graph as a pair of vertices and and edge connecting those vertices.  This is
		 * by no means fool-proof, but it may be close enough.
		 * 
		 * @param[in] worldPolygon This is the polygon to approximate.
		 * @param[in] snapTolerance See the @ref AddPathSegment method taking a line-segment as its first argument.
		 * @return True is returned if a new edge was created in the graph; false, otherwise.
		 */
		bool AddPathSegment(const Polygon& worldPolygon, double snapTolerance = 0.5);

		/**
		 * Expand this graph by a single line segment.
		 * 
		 * @param[in] pathSegment Make sure that the two points of this segment are connected in this graph, creating the vertices if necessary.
		 * @param[in] snapTolerance Vertices are not created if existing ones already exist within this distance of the given vertices.  Or, a vertex is created on an edge within this distance, if possible.
		 * @return True is returned if a new edge was created in the graph; false, otherwise.
		 */
		bool AddPathSegment(const LineSegment& pathSegment, double snapTolerance = 0.5);

		/**
		 * Add a vertex to the graph at the given location.
		 * 
		 * @param[in] location This is the location of the new node.
		 * @param[in] snapTolerance An existing node is returned if it is within this distance of the given location, or a vertex is created along an edge if the given location is within this distance of an existing edge.
		 * @return The new (or existing) node is returned.
		 */
		Node* AddLocation(const Vector3& location, double snapTolerance = 0.5);

		/**
		 * Add an edge to the graph between the two given vertices.
		 * Note that the order of the nodes given doesn't matter.
		 * 
		 * @param[in] nodeA The first of the two given vertices.
		 * @param[in] nodeB The second of the two given vertices.
		 * @return False is returned if such an edge already exists.  Otherwise, the edge is created, and true is returned.
		 */
		bool AddPathBetweenNodes(Node* nodeA, Node* nodeB);

		/**
		 * Remove an edge from the graph that exists between the two given vertices.
		 * Note that the order of the nodes given doesn't matter.
		 * 
		 * @param[in] nodeA The first of the two given vertices.
		 * @param[in] nodeB The second of the two given vertices.
		 * @return False is returned if no such edge exists.  Otherwise, the edge is destroyed, and true is returned.
		 */
		bool RemovePathBetweenNodes(Node* nodeA, Node* nodeB);

		/**
		 * Look for an existing edge joining the two given vertices.
		 * The order of the nodes given doesn't matter.
		 * 
		 * @param[in] nodeA The first of the two given vertices.
		 * @param[in] nodeB The second of the two given vertices.
		 * @return The index (or offset) into this graph's path array is returned, if found; -1 otherwise.
		 */
		int FindPathJoiningNodes(Node* nodeA, Node* nodeB);

		/**
		 * Remove the edge in this graph's path array at the given offset.
		 * 
		 * @return True is returned on success; false, otherwise.
		 */
		bool RemovePath(int i);

		/**
		 * This method is used for debugging purposes so that we can visualize the nav-graph.
		 * 
		 * @param[in,out] debugLines Pass an interface to the debug-drawing facility here.
		 */
		void DebugDraw(DebugLines& debugLines) const;

		/**
		 * Find the node in this graph nearest the given location.
		 * If more than one such node exists, the first found is returned,
		 * which is really undefined since no order on the nodes is enforced.
		 * Note that for now, we're doing a linear search here, which is
		 * reasonable, because the number of vertices in a nav-graph should
		 * never be so many that such a search becomes too slow.
		 * 
		 * @param[in] location All nodes of the graph are tested against this location.
		 * @return A pointer to the nearest node is returned.  Don't delete it.  Null is returned if the graph is empty.
		 */
		const Node* FindNearestNode(const Vector3& location) const;

		/**
		 * This works just like the @ref FindNearestNode method, but requires
		 * that the returned node be within the given maximum distance.  If no
		 * such node exists, null is returned.
		 */
		const Node* FindNearestNodeWithinDistance(const Vector3& location, double maxDistance) const;

		/**
		 * Find the edge of the graph nearest the given location.
		 * 
		 * @param[in] location This location is checked against the paths of the graph.
		 * @param[out] i If given, this tells you which terminal of the returned edge is nearest the given location.
		 * @return A pointer to the found edge is returned.
		 */
		const Path* FindNearestPath(const Vector3& location, int* i = nullptr) const;

		/**
		 * Find and return the shortest path between two nodes of the nav-graph
		 * using Dijkstra's algorihm.  (There can possibly exist more than one
		 * path of the shortest possible length, but for now, we don't define here
		 * which one we return.)  Note that the path is returned as a sequence of
		 * indices (or turns, if you will) into the adjacency arrays of the nodes
		 * of the graph, starting at the first given node.
		 * 
		 * @parma[in] nodeA This is the node where the path should start.
		 * @param[in] nodeB This is the node where the path should end.
		 * @param[out] pathList This will be populated with the turns made in the found path.
		 * @return True is returned on success; false, otherwise.  Failure can occur here if there is no path between the two given nodes.
		 */
		bool FindShortestPath(const Node* nodeA, const Node* nodeB, std::list<int>& pathList) const;

		/**
		 * Return a random node in the graph.
		 * 
		 * @param[in,out] random This is the random number generator to use.
		 * @return A pointer to the randomly selected node is returned.  Null is returned if there are no nodes in the graph.
		 */
		const Node* GetRandomNode(Random& random) const;

	private:

		// Note that for the memory to get freed correctly, it is
		// not enough to just clear these arrays.  Each node and path
		// must be cleared individually as well.  This is because we
		// have to account for the possibility of circular references.
		std::vector<Reference<Node>> nodeArray;
		std::vector<Reference<Path>> pathArray;
	};
}