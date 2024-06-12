#include "CollisionShapeSet.h"
#include "Collision/Shapes/Polygon.h"

using namespace Imzadi;

CollisionShapeSet::CollisionShapeSet()
{
	this->collisionShapeArray = new std::vector<Imzadi::Shape*>();
}

/*virtual*/ CollisionShapeSet::~CollisionShapeSet()
{
	this->Clear(true);

	delete this->collisionShapeArray;
}

/*virtual*/ bool CollisionShapeSet::Load(const rapidjson::Document& jsonDoc, std::string& error, AssetCache* assetCache)
{
	this->Clear(true);

	if (!jsonDoc.IsObject() || !jsonDoc.HasMember("shape_set"))
		return false;

	const rapidjson::Value& shapeSetValue = jsonDoc["shape_set"];
	if (!shapeSetValue.IsArray())
		return false;

	for (int i = 0; i < shapeSetValue.Size(); i++)
	{
		const rapidjson::Value& shapeValue = shapeSetValue[i];

		if (!shapeValue.IsObject() || !shapeValue.HasMember("type") || !shapeValue["type"].IsString())
			return false;

		std::string shapeType = shapeValue["type"].GetString();

		if (shapeType == "polygon")
		{
			auto polygon = PolygonShape::Create();
			this->collisionShapeArray->push_back(polygon);

			if (!shapeValue.HasMember("vertex_array") || !shapeValue["vertex_array"].IsArray())
				return false;

			const rapidjson::Value& vertexArrayValue = shapeValue["vertex_array"];
			for (int j = 0; j < vertexArrayValue.Size(); j++)
			{
				const rapidjson::Value& vertexValue = vertexArrayValue[j];
				if (!vertexValue.IsArray() || vertexValue.Size() != 3)
					return false;

				Vector3 vertex;
				vertex.x = vertexValue[0].GetFloat();
				vertex.y = vertexValue[1].GetFloat();
				vertex.z = vertexValue[2].GetFloat();
				polygon->AddVertex(vertex);
			}
		}
		else
		{
			// TODO: Add support for other shape types.
			return false;
		}
	}

	return true;
}

/*virtual*/ bool CollisionShapeSet::Unload()
{
	this->Clear(true);

	return true;
}

void CollisionShapeSet::Clear(bool deleteShapes)
{
	if (deleteShapes)
		for (Shape* shape : *this->collisionShapeArray)
			Shape::Free(shape);

	this->collisionShapeArray->clear();
}

bool CollisionShapeSet::GetBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	if (this->collisionShapeArray->size() == 0)
		return false;

	Shape* shape = (*this->collisionShapeArray)[0];
	boundingBox = shape->GetBoundingBox();

	for (int i = 1; i < (signed)this->collisionShapeArray->size(); i++)
	{
		shape = (*this->collisionShapeArray)[i];
		boundingBox.Expand(shape->GetBoundingBox());
	}

	return true;
}