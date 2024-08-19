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
		 * ...
		 */
		void ReduceEdgeCount(int numEdgesToRemove);

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