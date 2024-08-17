#pragma once

#include "Vector3.h"

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
		virtual ~Graph();

		/**
		 * Delete all nodes from this graph.
		 */
		void Clear();

		/**
		 * Construct a graph from a polygon mesh.  The graph's vertices
		 * will be the mesh's vertices.  The graph's edges will be the
		 * edges of the polygons in the mesh.
		 */
		bool FromPolygohMesh(const PolygonMesh& mesh);

		/**
		 * Construct a polygon mesh from this graph.  Cycles in this
		 * graph become polygons of the mesh.
		 */
		bool ToPolygonMesh(PolygonMesh& mesh) const;

		/**
		 * This is where we increase or decrease the number of vertices and edges
		 * in the graph under the assumption that the graph represents a mesh.
		 */
		void ModifyDetail(double percentage);

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

			const Node* GetAdjacentNode(int i) const { return this->adjacentNodeArray[i]; }
			int GetNumAdjacencies() const { return (int)this->adjacentNodeArray.size(); }

			bool IsAdjacentTo(const Node* node) const;

		private:
			Vector3 vertex;
			Vector3 normal;
			std::vector<Node*> adjacentNodeArray;
		};

		const Node* GetNode(int i) const { return this->nodeArray[i]; }
		int GetNumNodes() const { return (int)this->nodeArray.size(); }

	private:
		std::vector<Node*> nodeArray;
	};
}