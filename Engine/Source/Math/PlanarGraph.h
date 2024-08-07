#pragma once

#include "Vector3.h"
#include "Plane.h"
#include "LineSegment.h"
#include <vector>
#include <unordered_set>

namespace Imzadi
{
	struct PlanarGraphEdge
	{
		int i, j;
	};
}

template<>
struct std::hash<Imzadi::PlanarGraphEdge>
{
	std::size_t operator()(const Imzadi::PlanarGraphEdge& edge) const
	{
		return Imzadi::HashBuffer((const char*)&edge, sizeof(edge));
	}
};

namespace Imzadi
{
	inline bool operator==(const Imzadi::PlanarGraphEdge& edgeA, const Imzadi::PlanarGraphEdge& edgeB)
	{
		return edgeA.i == edgeB.i && edgeA.j == edgeB.j;
	}

	/**
	 * In the mathematical sense, these are indeed planar graphs, which is to say
	 * that they can be embedded in the plane in such a way that their edges intersect
	 * only at the end-points.  Further, vertices are points in 3D space, but are all
	 * coplanar, and edges are always line-segments (not curved or anything like that.)
	 * Lastly, these are directed graphs in that each edge is an ordered pair of vertices.
	 * We add the special property, however, that at most one edge can exist between any
	 * two pairs of vertices.
	 * 
	 * The idea here is that you can put polygons (convex or concave) into the graph, and
	 * then pull polygons (convex or concave) out of the graph as a means of merging a
	 * collection of coplanar polygons into simpler polygons.  (The user would be responsible
	 * for tessellation, if convex polygons are desired.)  We require all polygons to be
	 * wound CCW when viewed from the front-side of the plane (the side to which the normal
	 * to the plane points.)
	 * 
	 * Adding a polygon to the graph is just a matter of adding its edges.  Adding an edge to
	 * the graph may create or destroy edges in the graph.  It may also create vertices, or
	 * re-use existing vertices.
	 * 
	 * Reading a polygon out of the graph is a matter of finding a cycle in the graph along
	 * directed edges with the property that at each vertex of the cycle, it is not possible
	 * to make an alternative right-turn.
	 */
	class IMZADI_API PlanarGraph
	{
	public:
		PlanarGraph();
		virtual ~PlanarGraph();

		void SetPlane(const Plane& plane);
		const Plane& GetPlane() const;

		void Clear();

		bool AddPolygon(const std::vector<Vector3>& vertexArray, double epsilon = 1e-6);
		bool AddEdge(const Vector3& vertexA, const Vector3& vertexB, double epsilon = 1e-6);
		bool AddVertex(const Vector3& vertex, double epsilon = 1e-6);

		void ExtractAllPolygons(std::vector<std::vector<Vector3>*>& polygonArray) const;

	private:

		class Node
		{
		public:
			Node();
			virtual ~Node();

			const Node* FindOutGoingNode(const Node* inComingNode, const Vector3& planeNormal) const;

			Vector3 vertex;
			mutable std::vector<const Node*> adjacentNodeArray;
		};

		void RegenerateNodeAdjacencies() const;
		int FindVertex(const Vector3& vertex, double epsilon = 1e-6) const;
		LineSegment MakeLineSegment(const PlanarGraphEdge& edge) const;
		bool FindOuterCycle(const Node* node, std::vector<const Node*>& cycleArray) const;

		std::vector<Node*> nodeArray;
		std::unordered_set<PlanarGraphEdge> edgeSet;
		Plane plane;
	};
}