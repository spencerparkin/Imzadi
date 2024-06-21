#include "CollisionShapeSet.h"
#include "Collision/Shapes/Polygon.h"
#include "Log.h"

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

/*virtual*/ bool CollisionShapeSet::Load(const rapidjson::Document& jsonDoc, AssetCache* assetCache)
{
	this->Clear(true);

	if (!jsonDoc.IsObject() || !jsonDoc.HasMember("shape_set"))
	{
		IMZADI_LOG_ERROR("JSON data has no \"shape_set\" member");
		return false;
	}

	const rapidjson::Value& shapeSetValue = jsonDoc["shape_set"];
	if (!shapeSetValue.IsArray())
	{
		IMZADI_LOG_ERROR("The \"shape_set\" member is not an array.");
		return false;
	}

	for (int i = 0; i < shapeSetValue.Size(); i++)
	{
		const rapidjson::Value& shapeValue = shapeSetValue[i];

		if (!shapeValue.IsObject() || !shapeValue.HasMember("type") || !shapeValue["type"].IsString())
		{
			IMZADI_LOG_ERROR("No \"type\" field found in shape set entry.");
			return false;
		}

		std::string shapeType = shapeValue["type"].GetString();

		if (shapeType == "polygon")
		{
			auto polygon = PolygonShape::Create();
			this->collisionShapeArray->push_back(polygon);

			if (!shapeValue.HasMember("vertex_array") || !shapeValue["vertex_array"].IsArray())
			{
				IMZADI_LOG_ERROR("No \"vertex_array\" member found or it is not an array.");
				return false;
			}

			const rapidjson::Value& vertexArrayValue = shapeValue["vertex_array"];
			for (int j = 0; j < vertexArrayValue.Size(); j++)
			{
				const rapidjson::Value& vertexValue = vertexArrayValue[j];
				if (!vertexValue.IsArray() || vertexValue.Size() != 3)
				{
					IMZADI_LOG_ERROR("Expected vertex to be an array of 3.");
					return false;
				}

				Vector3 vertex;
				vertex.x = vertexValue[0].GetFloat();
				vertex.y = vertexValue[1].GetFloat();
				vertex.z = vertexValue[2].GetFloat();
				polygon->AddVertex(vertex);
			}
		}
		else
		{
			IMZADI_LOG_ERROR(std::format("The shape type \"{}\" is not yet supported.", shapeType.c_str()));
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