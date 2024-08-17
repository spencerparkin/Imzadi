#pragma once

#include "Vector3.h"
#include "Plane.h"
#include <vector>

namespace Imzadi
{
	class Polygon;

	/**
	 * These are sets of polygons that share a set of vertices.
	 */
	class IMZADI_API PolygonMesh
	{
	public:
		PolygonMesh();
		PolygonMesh(const PolygonMesh& polygonMesh);
		virtual ~PolygonMesh();

		void operator=(const PolygonMesh& polygonMesh);

		/**
		 * Here we check for NaN and Inf as well as make sure that
		 * all indices are valid, but that's as far as we go.  The
		 * mesh can be invalid in a lot of other ways that cause some
		 * method's results to be undefined.
		 */
		bool IsValid() const;

		/**
		 * Find and return the index for the given vertex.  If not found,
		 * -1 is returned.  Note that this search is linear, so this can
		 * get slow for large meshes.
		 */
		int FindVertex(const Vector3& vertex, double epsilon = 1e-6);

		/**
		 * This function finds and returns the index for the given vertex,
		 * if found.  If not found, the vertex is added and its index is
		 * returned.  Note that the search is linear, so this can get slow
		 * for large meshes.
		 */
		int FindOrAddVertex(const Vector3& vertex, double epsilon = 1e-6);

		/**
		 * Immediately add a vertex to the mesh without first checking to see if,
		 * approximately, this point already exists in the mesh.  The new index
		 * of the vertex is returned.
		 */
		int AddVertex(const Vector3& vertex);

		/**
		 * Delete all vertices and polygons in this mesh.
		 */
		void Clear();

		/**
		 * Generate a mesh that fits the given point-cloud as tightly as possible.
		 */
		bool GenerateConvexHull(const std::vector<Vector3>& pointArray);

		/**
		 * If the given percentage if less than one, then collapse edges to points in this mesh to
		 * decrease the number of vertices and polygons in the mesh.  If the given percentage is
		 * greater than one, then expand points to edges in this mesh to increase the number of
		 * vertices and polygons in the mesh.
		 */
		bool ModifyDetail(double percentage);

		/**
		 * If topologically, the given meshes have descernable insides and outsides,
		 * calculate this mesh as their union.  (Klein bottles don't work.)
		 */
		void CalculateUnion(const PolygonMesh& polygonMeshA, const PolygonMesh& polygonMeshB);

		/**
		 * If topologically, the given meshes have descernable insides and outsides,
		 * calculate this mesh as their intersection.  (Klein bottles don't work.)
		 */
		void CalculateIntersection(const PolygonMesh& polygonMeshA, const PolygonMesh& polygonMeshB);

		/**
		 * If topologically, the given meshes have descernable insides and outsides,
		 * calculate this mesh as the first minus the second.  (Klein bottles don't work.)
		 */
		void CalculateDifference(const PolygonMesh& polygonMeshA, const PolygonMesh& polygonMeshB);

		/**
		 * Generate from this mesh a list of all its polygons, each having its own vertex data.
		 */
		void ToStandalonePolygonArray(std::vector<Imzadi::Polygon>& standalonePolygonArray) const;

		/**
		 * Clear this mesh and then add to it all polygons from the given array.
		 */
		void FromStandalonePolygonArray(const std::vector<Imzadi::Polygon>& standalonePolygonArray, double epsilon = 1e-6);

		/**
		 * Make sure that all faces use exactly one polygon.
		 */
		void SimplifyFaces(bool mustBeConvex, double epsilon = 1e-6);

		/**
		 * Convert this mesh into a triangle mesh.
		 */
		void TessellateFaces(double epsilon = 1e-6);

		/**
		 * Write this mesh to the given stream in binary form.
		 */
		void Dump(std::ostream& stream) const;

		/**
		 * Read this mesh from the given stream in binary form.
		 */
		void Restore(std::istream& stream);

		/**
		 * These are just sequences of indices into the vertex array.
		 * We use a CCW winding to indicate the front-side of the polygon.
		 */
		class IMZADI_API Polygon
		{
		public:
			Polygon();
			Polygon(const Polygon& polygon);
			virtual ~Polygon();

			void operator=(const Polygon& polygon);

			/**
			 * Return the vertex at position i modulo the number of vertices in this polygon.
			 */
			int operator()(int i) const;

			/**
			 * Remove all vertices from this polygon.
			 */
			void Clear();

			/**
			 * Get a polygon with its own vertex data from this polygon.
			 */
			void ToStandalonePolygon(Imzadi::Polygon& polygon, const PolygonMesh* mesh) const;

			/**
			 * Set this polygon using the given stand-alone polygon and mesh.
			 */
			void FromStandalonePolygon(const Imzadi::Polygon& polygon, PolygonMesh* mesh, double epsilon = 1e-6);

			/**
			 * Return the given i in the range [0,N-1], where N is the number of vertices in this polygon,
			 * by adding an appropriate multiple of N to i.
			 */
			int Mod(int i) const;

			/**
			 * Reverse the winding of this polygon from CCW to CW, or vice-versa.
			 */
			void Reverse();

			/**
			 * Write this polygon to the given stream in binary form.
			 */
			void Dump(std::ostream& stream) const;

			/**
			 * Read this polygon from the given stream in binary form.
			 */
			void Restore(std::istream& stream);

		public:
			std::vector<int> vertexArray;
		};

		const std::vector<Vector3>& GetVertexArray() const { return this->vertexArray; }
		const std::vector<Polygon>& GetPolygonArray() const { return this->polygonArray; }

		const Vector3& GetVertex(int i) const { return this->vertexArray[i]; }
		void SetVertex(int i, const Vector3& vertex) { this->vertexArray[i] = vertex; }

		const Polygon& GetPolygon(int i) const { return this->polygonArray[i]; }
		void SetPolygon(int i, const Polygon& polygon) { this->polygonArray[i] = polygon; }
		void AddPolygon(const Polygon& polygon) { this->polygonArray.push_back(polygon); }

		int GetNumVertices() const { return (int)this->vertexArray.size(); }
		int GetNumPolygons() const { return (int)this->polygonArray.size(); }

	protected:

		/**
		 * This class is occationally used internally by certain algorithms.
		 */
		class Triangle
		{
		public:
			Triangle();
			Triangle(int i, int j, int k);
			Triangle(const Triangle& triangle);
			virtual ~Triangle();

			void operator=(const Triangle& triangle);

			Plane MakePlane(const PolygonMesh* mesh) const;
			bool Cancels(const Triangle& triangle) const;
			bool SameAs(const Triangle& triangle) const;
			void Reverse();
			Triangle Reversed() const;
			void ToPolygon(Polygon& polygon) const;

		public:
			int vertex[3];
		};

	protected:
		std::vector<Vector3> vertexArray;
		std::vector<Polygon> polygonArray;
	};
}