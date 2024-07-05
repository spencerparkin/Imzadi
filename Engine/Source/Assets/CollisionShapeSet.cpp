#include "CollisionShapeSet.h"
#include "Collision/Shapes/Polygon.h"
#include "Log.h"

using namespace Imzadi;

CollisionShapeSet::CollisionShapeSet()
{
}

/*virtual*/ CollisionShapeSet::~CollisionShapeSet()
{
	this->Clear(true);
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
			this->collisionShapeArray.push_back(polygon);

			if (!shapeValue.HasMember("vertex_array") || !shapeValue["vertex_array"].IsArray())
			{
				IMZADI_LOG_ERROR("No \"vertex_array\" member found or it is not an array.");
				return false;
			}

			const rapidjson::Value& vertexArrayValue = shapeValue["vertex_array"];
			for (int j = 0; j < vertexArrayValue.Size(); j++)
			{
				const rapidjson::Value& vertexValue = vertexArrayValue[j];
				
				Vector3 vertex;
				if (!LoadVector(vertexValue, vertex))
				{
					IMZADI_LOG_ERROR("Failed to load vertex for collision shape.");
					return false;
				}

				polygon->AddVertex(vertex);
			}
		}
		else
		{
			IMZADI_LOG_ERROR(std::format("The shape type \"{}\" is not yet supported.", shapeType.c_str()));
			return false;
		}
	}

	Transform objectToWorld;
	objectToWorld.SetIdentity();
	if (jsonDoc.HasMember("object_to_world"))
	{
		if (!Asset::LoadTransform(jsonDoc["object_to_world"], objectToWorld))
		{
			IMZADI_LOG_ERROR("Failed to load object-to-world transform from collision file.");
			return false;
		}
	}

	for (Imzadi::Shape* shape : this->collisionShapeArray)
		shape->SetObjectToWorldTransform(objectToWorld);

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
		for (Shape* shape : this->collisionShapeArray)
			Shape::Free(shape);

	this->collisionShapeArray.clear();
}

bool CollisionShapeSet::GetBoundingBox(AxisAlignedBoundingBox& boundingBox) const
{
	if (this->collisionShapeArray.size() == 0)
		return false;

	Shape* shape = this->collisionShapeArray[0];
	boundingBox = shape->GetBoundingBox();

	for (int i = 1; i < (signed)this->collisionShapeArray.size(); i++)
	{
		shape = this->collisionShapeArray[i];
		boundingBox.Expand(shape->GetBoundingBox());
	}

	return true;
}