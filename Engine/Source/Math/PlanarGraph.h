#pragma once

#include "Polygon.h"
#include "Plane.h"
#include "LineSegment.h"
#include <vector>
#include <unordered_set>

namespace Imzadi
{
	struct PlanarGraphEdge
	{
		int i, j;

		void Swap()
		{
			int k = this->i;
			this->i = this->j;
			this->j = k;
		}
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
	 * 
	 * The idea here is that you can put polygons (convex or concave) into the graph, and
	 * then pull polygons (convex or concave) out of the graph as a means of merging a
	 * collection of coplanar polygons into simpler polygons.  (The user would be responsible
	 * for tessellation, if convex polygons are desired.)  We require all polygons to be
	 * wound CCW when viewed from the front-side of the plane (the side to which the normal
	 * to the plane points.)
	 */
	class IMZADI_API PlanarGraph
	{
	public:
		PlanarGraph();
		virtual ~PlanarGraph();

		void SetPlane(const Plane& plane);
		const Plane& GetPlane() const;

		void Clear();

		bool AddPolygon(const Polygon& polygon, double epsilon = 1e-6);
		bool AddEdge(const Vector3& vertexA, const Vector3& vertexB, double epsilon = 1e-6);
		bool AddVertex(const Vector3& vertex, double epsilon = 1e-6);

		bool ExtractAllPolygons(std::vector<Polygon>& polygonArray);

	private:

		class Node
		{
		public:
			Node();
			virtual ~Node();

			const Node* FindOutGoingNode(const Node* inComingNode, const Vector3& planeNormal) const;
			bool HasAdjacency(const Node* node) const;

			int i;
			Vector3 vertex;
			mutable std::vector<const Node*> adjacentNodeArray;
		};

		void RegenerateNodeAdjacencies() const;
		int FindVertex(const Vector3& vertex, double epsilon = 1e-6) const;
		LineSegment MakeLineSegment(const PlanarGraphEdge& edge) const;
		bool FindOuterCycle(const Node* node, std::vector<const Node*>& cycleArray) const;
		int CountConnectedComponents() const;
		void FindComponent(const Node* node, std::vector<const Node*>& nodeArray) const;

		std::vector<Node*> nodeArray;
		std::unordered_set<PlanarGraphEdge> edgeSet;
		Plane plane;
	};
}