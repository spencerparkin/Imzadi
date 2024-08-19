#include "PolygonMesh.h"
#include "Polygon.h"
#include "Graph.h"

using namespace Imzadi;

//--------------------------- PolygonMesh::Polygon ---------------------------

PolygonMesh::PolygonMesh()
{
}

PolygonMesh::PolygonMesh(const PolygonMesh& polygonMesh)
{
	*this = polygonMesh;
}

/*virtual*/ PolygonMesh::~PolygonMesh()
{
}

void PolygonMesh::operator=(const PolygonMesh& polygonMesh)
{
	this->Clear();

	for (const Polygon& polygon : polygonMesh.polygonArray)
		this->polygonArray.push_back(polygon);
}

bool PolygonMesh::IsValid() const
{
	for (const Vector3& vertex : this->vertexArray)
		if (!vertex.IsValid())
			return false;

	for (const Polygon& polygon : this->polygonArray)
		for (int i : polygon.vertexArray)
			if (i < 0 || i >= (signed)this->vertexArray.size())
				return false;

	return true;
}

int PolygonMesh::FindVertex(const Vector3& vertex, double epsilon /*= 1e-6*/)
{
	// TODO: We might consider optimizing this with a spacial index.

	for (int i = 0; i < (signed)this->vertexArray.size(); i++)
		if (this->vertexArray[i].IsPoint(vertex, epsilon))
			return i;

	return -1;
}

int PolygonMesh::FindOrAddVertex(const Vector3& vertex, double epsilon /*= 1e-6*/)
{
	int i = this->FindVertex(vertex, epsilon);
	if (i >= 0)
		return i;

	return this->AddVertex(vertex);
}

int PolygonMesh::AddVertex(const Vector3& vertex)
{
	int i = (signed)this->vertexArray.size();
	this->vertexArray.push_back(vertex);
	return i;
}

void PolygonMesh::Clear()
{
	this->vertexArray.clear();
	this->polygonArray.clear();
}

bool PolygonMesh::GenerateConvexHull(const std::vector<Vector3>& pointArray)
{
	this->Clear();

	std::list<Triangle> triangleList;

	auto findInitialTetrahedron = [&pointArray, &triangleList, this]() -> bool
	{
		// Is there a better approach to this problem?  This looks really
		// slow, but we only loop until we find a non-negative determinant.
		for (int i = 0; i < (signed)pointArray.size(); i++)
		{
			const Vector3& vertexA = pointArray[i];

			for (int j = 0; j < (signed)pointArray.size(); j++)
			{
				if (j == i)
					continue;

				const Vector3& vertexB = pointArray[j];

				for (int k = 0; k < (signed)pointArray.size(); k++)
				{
					if (k == i || k == j)
						continue;

					const Vector3& vertexC = pointArray[k];

					for (int l = 0; l < (signed)pointArray.size(); l++)
					{
						if (l == i || l == j || l == k)
							continue;

						const Vector3& vertexD = pointArray[l];

						Vector3 xAxis = vertexB - vertexA;
						Vector3 yAxis = vertexC - vertexA;
						Vector3 zAxis = vertexD - vertexA;

						double det = xAxis.Cross(yAxis).Dot(zAxis);

						if (det != 0.0)
						{
							this->vertexArray.push_back(vertexA);
							this->vertexArray.push_back(vertexB);
							this->vertexArray.push_back(vertexC);
							this->vertexArray.push_back(vertexD);
						}

						if (det < 0.0)
						{
							triangleList.push_back(Triangle(0, 1, 2));
							triangleList.push_back(Triangle(0, 2, 3));
							triangleList.push_back(Triangle(0, 3, 1));
							triangleList.push_back(Triangle(1, 3, 2));

							return true;
						}
						else if (det > 0.0)
						{
							triangleList.push_back(Triangle(0, 2, 1));
							triangleList.push_back(Triangle(0, 3, 2));
							triangleList.push_back(Triangle(0, 1, 3));
							triangleList.push_back(Triangle(1, 2, 3));

							return true;
						}
					}
				}
			}
		}

		return false;
	};

	if (!findInitialTetrahedron())
		return false;

	std::list<Vector3> pointList;
	for (const Vector3& point : pointArray)
		pointList.push_back(point);

	const double planeThickness = 1e-6;

	while (pointList.size() > 0)
	{
		std::list<Vector3>::iterator iter = pointList.begin();
		Vector3 point = *iter;
		pointList.erase(iter);

		std::list<Triangle> newTriangleList;
		for (const Triangle& triangle : triangleList)
		{
			Plane plane = triangle.MakePlane(this);
			if (plane.GetSide(point, planeThickness) == Plane::Side::FRONT)
			{
				int i = (signed)this->vertexArray.size();

				Triangle triangleA = triangle.Reversed();
				Triangle triangleB(i, triangle.vertex[0], triangle.vertex[1]);
				Triangle triangleC(i, triangle.vertex[1], triangle.vertex[2]);
				Triangle triangleD(i, triangle.vertex[2], triangle.vertex[0]);

				newTriangleList.push_back(triangleA);
				newTriangleList.push_back(triangleB);
				newTriangleList.push_back(triangleC);
				newTriangleList.push_back(triangleD);
			}
		}

		if (newTriangleList.size() == 0)
			continue;
		
		this->vertexArray.push_back(point);

		for (const Triangle& newTriangle : newTriangleList)
		{
			bool addTriangle = true;
			for (std::list<Triangle>::iterator iter = triangleList.begin(); iter != triangleList.end(); iter++)
			{
				const Triangle& triangle = *iter;
				if (newTriangle.Cancels(triangle))
				{
					triangleList.erase(iter);
					addTriangle = false;
					break;
				}
			}

			if (addTriangle)
				triangleList.push_back(newTriangle);
		}
	}

	for (const Triangle& triangle : triangleList)
	{
		Polygon polygon;
		triangle.ToPolygon(polygon);
		this->polygonArray.push_back(polygon);
	}

	return true;
}

bool PolygonMesh::ReduceEdgeCount(int numEdgesToRemove)
{
	if (numEdgesToRemove <= 0)
		return true;

	Graph graph;
	if (!graph.FromPolygohMesh(*this))
		return false;

	graph.ReduceEdgeCount(numEdgesToRemove);

	if (!graph.ToPolygonMesh(*this))
		return false;

	return true;
}

void PolygonMesh::CalculateUnion(const PolygonMesh& polygonMeshA, const PolygonMesh& polygonMeshB)
{
	// TODO: Write this.
}

void PolygonMesh::CalculateIntersection(const PolygonMesh& polygonMeshA, const PolygonMesh& polygonMeshB)
{
	// TODO: Write this.
}

void PolygonMesh::CalculateDifference(const PolygonMesh& polygonMeshA, const PolygonMesh& polygonMeshB)
{
	// TODO: Write this.
}

void PolygonMesh::SimplifyFaces(bool mustBeConvex, double epsilon /*= 1e-6*/)
{
	std::vector<Imzadi::Polygon> standalonePolygonArray;
	this->ToStandalonePolygonArray(standalonePolygonArray);

	Imzadi::Polygon::Compress(standalonePolygonArray, mustBeConvex);

	this->FromStandalonePolygonArray(standalonePolygonArray, epsilon);
}

void PolygonMesh::Reduce()
{
	int i = 0;
	while (i < (signed)this->polygonArray.size())
	{
		Polygon& polygon = this->polygonArray[i];
		if (polygon.vertexArray.size() >= 3)
			i++;
		else
		{
			if (i != this->polygonArray.size() - 1)
				this->polygonArray[i] = this->polygonArray[this->polygonArray.size() - 1];
			this->polygonArray.pop_back();
		}
	}
}

void PolygonMesh::TessellateFaces(double epsilon /*= 1e-6*/)
{
	std::vector<Imzadi::Polygon> standalonePolygonArrayA;
	this->ToStandalonePolygonArray(standalonePolygonArrayA);

	std::vector<Imzadi::Polygon> standalonePolygonArrayB;
	for(Imzadi::Polygon& polygon : standalonePolygonArrayA)
		polygon.TessellateUntilTriangular(standalonePolygonArrayB);

	this->FromStandalonePolygonArray(standalonePolygonArrayB, epsilon);
}

void PolygonMesh::ToStandalonePolygonArray(std::vector<Imzadi::Polygon>& standalonePolygonArray) const
{
	for (const Polygon& polygon : this->polygonArray)
	{
		Imzadi::Polygon standalonePolygon;
		polygon.ToStandalonePolygon(standalonePolygon, this);
		standalonePolygonArray.push_back(standalonePolygon);
	}
}

void PolygonMesh::FromStandalonePolygonArray(const std::vector<Imzadi::Polygon>& standalonePolygonArray, double epsilon /*= 1e-6*/)
{
	this->Clear();

	for (const Imzadi::Polygon& standalonePolygon : standalonePolygonArray)
	{
		Polygon polygon;
		polygon.FromStandalonePolygon(standalonePolygon, this, epsilon);
		this->polygonArray.push_back(polygon);
	}
}

void PolygonMesh::Dump(std::ostream& stream) const
{
	uint32_t numVertices = (uint32_t)this->vertexArray.size();
	stream.write((char*)&numVertices, sizeof(numVertices));

	uint32_t numPolygons = (uint32_t)this->polygonArray.size();
	stream.write((char*)&numPolygons, sizeof(numPolygons));

	for (const Vector3& vertex : this->vertexArray)
		vertex.Dump(stream);

	for (const Polygon& polygon : this->polygonArray)
		polygon.Dump(stream);
}

void PolygonMesh::Restore(std::istream& stream)
{
	uint32_t numVertices = 0;
	stream.read((char*)&numVertices, sizeof(numVertices));

	uint32_t numPolygons = 0;
	stream.read((char*)&numPolygons, sizeof(numPolygons));

	this->Clear();

	for (int i = 0; i < numVertices; i++)
	{
		Vector3 vertex;
		vertex.Restore(stream);
		this->vertexArray.push_back(vertex);
	}

	for (int i = 0; i < numPolygons; i++)
	{
		Polygon polygon;
		polygon.Restore(stream);
		this->polygonArray.push_back(polygon);
	}
}

//--------------------------- PolygonMesh::Polygon ---------------------------

PolygonMesh::Polygon::Polygon()
{
}

PolygonMesh::Polygon::Polygon(const Polygon& polygon)
{
	*this = polygon;
}

/*virtual*/ PolygonMesh::Polygon::~Polygon()
{
}

void PolygonMesh::Polygon::operator=(const Polygon& polygon)
{
	this->Clear();

	for (int i : polygon.vertexArray)
		this->vertexArray.push_back(i);
}

int PolygonMesh::Polygon::operator()(int i) const
{
	i %= (signed)this->vertexArray.size();
	if (i < 0)
		i += this->vertexArray.size();
	return this->vertexArray[i];
}

void PolygonMesh::Polygon::Clear()
{
	this->vertexArray.clear();
}

void PolygonMesh::Polygon::ToStandalonePolygon(Imzadi::Polygon& polygon, const PolygonMesh* mesh) const
{
	polygon.Clear();

	for (int i : this->vertexArray)
		polygon.vertexArray.push_back(mesh->vertexArray[i]);
}

void PolygonMesh::Polygon::FromStandalonePolygon(const Imzadi::Polygon& polygon, PolygonMesh* mesh, double epsilon /*= 1e-6*/)
{
	this->Clear();

	for (const Vector3& vertex : polygon.vertexArray)
		this->vertexArray.push_back(mesh->FindOrAddVertex(vertex, epsilon));
}

int PolygonMesh::Polygon::Mod(int i) const
{
	i %= (int)this->vertexArray.size();
	if (i < 0)
		i += (int)this->vertexArray.size();
	return i;
}

void PolygonMesh::Polygon::Reverse()
{
	std::vector<int> vertexStack;
	for (int i : this->vertexArray)
		vertexStack.push_back(i);

	this->vertexArray.clear();
	while (vertexStack.size() > 0)
	{
		int i = vertexStack[vertexStack.size() - 1];
		vertexStack.pop_back();
		this->vertexArray.push_back(i);
	}
}

void PolygonMesh::Polygon::Dump(std::ostream& stream) const
{
	uint32_t numVertices = (uint32_t)this->vertexArray.size();
	stream.write((char*)&numVertices, sizeof(numVertices));

	for (int i : this->vertexArray)
		stream.write((char*)&i, sizeof(i));
}

void PolygonMesh::Polygon::Restore(std::istream& stream)
{
	uint32_t numVertices = 0;
	stream.read((char*)&numVertices, sizeof(numVertices));

	this->vertexArray.clear();
	for (int i = 0; i < numVertices; i++)
	{
		int j = 0;
		stream.read((char*)&j, sizeof(j));
		this->vertexArray.push_back(j);
	}
}

//--------------------------- PolygonMesh::Triangle ---------------------------

PolygonMesh::Triangle::Triangle()
{
	for (int i = 0; i < 3; i++)
		this->vertex[i] = 0;
}

PolygonMesh::Triangle::Triangle(int i, int j, int k)
{
	this->vertex[0] = i;
	this->vertex[1] = j;
	this->vertex[2] = k;
}

PolygonMesh::Triangle::Triangle(const Triangle& triangle)
{
	*this = triangle;
}

/*virtual*/ PolygonMesh::Triangle::~Triangle()
{
}

void PolygonMesh::Triangle::operator=(const Triangle& triangle)
{
	for (int i = 0; i < 3; i++)
		this->vertex[i] = triangle.vertex[i];
}

Plane PolygonMesh::Triangle::MakePlane(const PolygonMesh* mesh) const
{
	const Vector3& vertexA = mesh->vertexArray[this->vertex[0]];
	const Vector3& vertexB = mesh->vertexArray[this->vertex[1]];
	const Vector3& vertexC = mesh->vertexArray[this->vertex[2]];

	Vector3 normal = (vertexB - vertexA).Cross(vertexC - vertexA).Normalized();
	return Plane(vertexA, normal);
}

bool PolygonMesh::Triangle::Cancels(const Triangle& triangle) const
{
	Triangle reverse = triangle;
	reverse.Reverse();
	return reverse.SameAs(*this);
}

bool PolygonMesh::Triangle::SameAs(const Triangle& triangle) const
{
	for (int i = 0; i < 3; i++)
	{
		if (this->vertex[i] == triangle.vertex[0])
		{
			for (int j = 0; j < 3; j++)
				if (this->vertex[(i + j) % 3] != triangle.vertex[j])
					return false;

			return true;
		}
	}

	return false;
}

void PolygonMesh::Triangle::Reverse()
{
	int i = this->vertex[0];
	this->vertex[0] = this->vertex[2];
	this->vertex[2] = i;
}

PolygonMesh::Triangle PolygonMesh::Triangle::Reversed() const
{
	Triangle triangle(*this);
	triangle.Reverse();
	return triangle;
}

void PolygonMesh::Triangle::ToPolygon(Polygon& polygon) const
{
	polygon.Clear();
	for (int i = 0; i < 3; i++)
		polygon.vertexArray.push_back(this->vertex[i]);
}