#pragma once

#include "Vector3.h"
#include <set>

namespace Imzadi
{
	class PolygonMesh;

	/**
	 * These are directed graphs of points in 3D space.  We associate a
	 * normal with each point in the case that the graph represents a mesh,
	 * but its use is generally optional.  For conversion to/from a mesh,
	 * we need the normal as a means of preserving the winding order of
	 * the polygons.  Also, some algorithms that operate on the graph make
	 * use of the vertex normal.
	 */
	class IMZADI_API Graph
	{
	public:
		class Node;

		Graph();
		Graph(const Graph& graph);
		virtual ~Graph();

		void operator=(const Graph& graph);

		/**
		 * Delete all nodes from this graph.
		 */
		void Clear();

		/**
		 * Construct a graph from a polygon mesh.  The graph's vertices
		 * will be the mesh's vertices.  The graph's edges will be the
		 * edges of the polygons in the mesh.
		 * 
		 * Of course, this isn't the only way to construct a graph from
		 * a mesh.  You could also let the mesh polygons be the vertices
		 * and the edges of the graph indicate where polygons are adjacent
		 * to other polygons.  This has applications in other algorithms,
		 * such as the union, intersection and difference of meshes.
		 */
		bool FromPolygohMesh(const PolygonMesh& mesh);

		/**
		 * Construct a polygon mesh from this graph.  Cycles in this
		 * graph become polygons of the mesh.
		 */
		bool ToPolygonMesh(PolygonMesh& mesh) const;

		/**
		 * Construct a polygon mesh from this graph, but also destroy
		 * this graph in the process of doing so.  Note that no check
		 * is made here to ensure that the resulting polygons are valid
		 * in the sense that their vertices are all coplanar.
		 */
		bool ToPolygonMesh(PolygonMesh& mesh);

		/**
		 * Uniformly(?) merge adjacent vertices of the mesh until the given
		 * number of edges have been removed.  Note that if the graph does
		 * not represent a triangle mesh, then the result, when converted
		 * back into a mesh, may contain polygons that are non-planar.
		 * If, however, you just stick with triangle meshes, then this
		 * shouldn't be a problem.
		 */
		bool ReduceEdgeCount(int numEdgesToRemove);

		/**
		 * Remove a node from the graph.  Fixup any node that pointed to it
		 * by removing the adjacency or having the node point to the given
		 * alternative node.
		 */
		void DeleteNode(Node* node, Node* alternativeNode = nullptr);

		/**
		 * These are not part of the graph data-structure, but can sometimes
		 * be useful in algorithm that deal with the graph.
		 */
		struct Edge
		{
			int i, j;

			Edge()
			{
				this->i = 0;
				this->j = 0;
			}

			Edge(const Edge& edge)
			{
				this->i = edge.i;
				this->j = edge.j;
			}

			Edge(int i, int j)
			{
				this->i = i;
				this->j = j;
			}

			virtual uint64_t MakeKey() const = 0;

			bool operator()(const Edge& edgeA, const Edge& edgeB) const
			{
				return edgeA.MakeKey() < edgeB.MakeKey();
			}
		};

		struct OrderedEdge : public Edge
		{
			OrderedEdge()
			{
			}

			OrderedEdge(int i, int j) : Edge(i, j)
			{
			}

			virtual uint64_t MakeKey() const override
			{
				return (uint64_t(this->i) << 32) | uint64_t(this->j);
			}
		};

		struct UnorderedEdge : public Edge
		{
			UnorderedEdge()
			{
			}

			UnorderedEdge(int i, int j) : Edge(i, j)
			{
			}

			virtual uint64_t MakeKey() const override
			{
				if (this->i < this->j)
					return (uint64_t(this->i) << 32) | uint64_t(this->j);
				else
					return (uint64_t(this->j) << 32) | uint64_t(this->i);
			}
		};

		/**
		 * Generate a set of edges for the graph.  These can be ordered edges
		 * or unordered edges.  (I.e., directional edges or non-directional.)
		 */
		template<typename T>
		void GenerateEdgeSet(std::set<T, T>& edgeSet) const
		{
			this->AssignIndicesForNodes();
			for (const Node* node : this->nodeArray)
				for (const Node* adjacentNode : node->adjacentNodeSet)
					edgeSet.insert(T(node->i, adjacentNode->i));
		}

		/**
		 * Calculate and return the length of the given edge.
		 */
		double CalcEdgeLength(const Edge& edge) const;

		/**
		 * These are the vertices of the graph, and they each
		 * have a position in 3D space.
		 */
		class IMZADI_API Node
		{
			friend Graph;

		public:
			Node();
			virtual ~Node();

			const Vector3& GetVertex() const { return this->vertex; }
			void SetVertex(const Vector3& vertex) { this->vertex = vertex; }

			const std::set<Node*>& GetAdjacentNodeSet() const { return this->adjacentNodeSet; }
			int GetNumAdjacencies() const { return (int)this->adjacentNodeSet.size(); }

			bool IsAdjacentTo(const Node* node) const;

			/**
			 * Find and return a node adjacent to this node that is as close as
			 * being in the given direction as possible relative to this node.
			 */
			Node* FindAdjacencyInDirection(const Vector3& unitDirection);

		private:
			Vector3 vertex;
			Vector3 normal;
			mutable int i;
			std::set<Node*> adjacentNodeSet;
		};

		const Node* GetNode(int i) const { return this->nodeArray[i]; }
		int GetNumNodes() const { return (int)this->nodeArray.size(); }

	private:

		/**
		 * This is used by the edge reduction algorithm.
		 */
		Node* MergeVertices(Node* nodeA, Node* nodeB);

		/**
		 * Make each node aware of where it is in the node array.
		 */
		void AssignIndicesForNodes() const;

		/**
		 * This is used in the graph to mesh conversion process, and
		 * it is destructive to the graph.
		 */
		bool FindAndRemovePolygonCycleForMesh(std::vector<int>& cycleArray);

	private:
		std::vector<Node*> nodeArray;
	};
}