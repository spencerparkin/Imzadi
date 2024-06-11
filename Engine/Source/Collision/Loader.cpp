#include "Loader.h"
#include "Shapes/Polygon.h"
#include <fstream>
#include <format>
#include <filesystem>

using namespace Imzadi;

//------------------------------ ShapeLoader ------------------------------

ShapeLoader::ShapeLoader()
{
}

/*virtual*/ ShapeLoader::~ShapeLoader()
{
}

/*static*/ ShapeLoader* ShapeLoader::Create(const std::string& filePath)
{
	std::string fileExt = std::filesystem::path(filePath).extension().string();

	std::vector<ShapeLoaderClassInterface*> shapeLoaderClassArray;
	shapeLoaderClassArray.push_back(new ShapeLoaderClass<OBJ_ShapeLoader>());
	// TODO: Add more shape loader class derivatives here.

	ShapeLoader* chosenShapeLoader = nullptr;

	for (ShapeLoaderClassInterface* shapeLoaderClass : shapeLoaderClassArray)
	{
		ShapeLoader* shapeLoader = shapeLoaderClass->Create();
		if (shapeLoader->CanLoadFileType(fileExt))
		{
			chosenShapeLoader = shapeLoader;
			break;
		}

		Free(shapeLoader);
	}

	for (ShapeLoaderClassInterface* shapeLoaderClass : shapeLoaderClassArray)
		delete shapeLoaderClass;

	return chosenShapeLoader;
}

/*static*/ void ShapeLoader::Free(ShapeLoader* shapeLoader)
{
	delete shapeLoader;
}

//------------------------------ OBJ_ShapeLoader ------------------------------

OBJ_ShapeLoader::OBJ_ShapeLoader()
{
}

/*virtual*/ OBJ_ShapeLoader::~OBJ_ShapeLoader()
{
}

/*virtual*/ bool OBJ_ShapeLoader::CanLoadFileType(const std::string& fileExtension)
{
	return 0 == ::strcmpi(".obj", fileExtension.c_str());
}

/*virtual*/ bool OBJ_ShapeLoader::LoadShapes(const std::string& filePath, std::vector<Shape*>& shapeArray)
{
	std::ifstream stream;
	stream.open(filePath);
	if (!stream.is_open())
		return false;

	std::vector<Vector3> vertexArray;
	// TODO: Do we need to periodically reset this array as we go down the file?

	std::string line;
	while (std::getline(stream, line))
	{
		std::vector<std::string> tokenArray;
		if (this->Tokenize(line, tokenArray))
		{
			if (tokenArray[0] == "v" && tokenArray.size() == 4)
			{
				Vector3 vertex;
				vertex.x = ::atof(tokenArray[1].c_str());
				vertex.y = ::atof(tokenArray[2].c_str());
				vertex.z = ::atof(tokenArray[3].c_str());
				vertexArray.push_back(vertex);
			}
			else if (tokenArray[0] == "f")
			{
				auto polygon = new PolygonShape(false);
				shapeArray.push_back(polygon);

				for (int i = 1; i < (signed)tokenArray.size(); i++)
				{
					const std::string& token = tokenArray[i];
					int j = ::atoi(token.c_str()) - 1;
					if (0 <= j && j < (signed)vertexArray.size())
						polygon->AddVertex(vertexArray[j]);
					else
						return false;
				}

				if (!polygon->IsValid())
					return false;
			}
		}
	}

	stream.close();
	return true;
}

bool OBJ_ShapeLoader::Tokenize(const std::string& line, std::vector<std::string>& tokenArray)
{
	tokenArray.clear();
	std::istringstream stream(line);
	std::string token;
	while (std::getline(stream, token, ' '))
		if (token.length() > 0)
			tokenArray.push_back(token);
	return tokenArray.size() > 0;
}