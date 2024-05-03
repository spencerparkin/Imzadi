#include "Loader.h"

using namespace Collision;

//------------------------------ ShapeLoader ------------------------------

ShapeLoader::ShapeLoader()
{
}

/*virtual*/ ShapeLoader::~ShapeLoader()
{
}

//------------------------------ OBJ_ShapeLoader ------------------------------

OBJ_ShapeLoader::OBJ_ShapeLoader()
{
}

/*virtual*/ OBJ_ShapeLoader::~OBJ_ShapeLoader()
{
}

/*virtual*/ bool OBJ_ShapeLoader::LoadShapes(const std::string& filePath, std::vector<Shape*>& shapeArray)
{
	// TODO: Write this.
	return false;
}