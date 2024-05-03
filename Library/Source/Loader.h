#pragma once

#include "Defines.h"
#include <string>
#include <vector>

namespace Collision
{
	class Shape;

	/**
	 * This is the base class for any derivative that knows how to load shapes
	 * from a file on disk.
	 */
	class COLLISION_LIB_API ShapeLoader
	{
	public:
		ShapeLoader();
		virtual ~ShapeLoader();

		/**
		 * Overrides should impliment this method by populating the given array with
		 * a bunch of shapes from the given file.  This is done on the main thread and
		 * then the shapes can be handed over to the collision system or used for any
		 * other purpose.
		 * 
		 * @param[in] filePath This should be a fully-qualified path to the file containing the shapes.  The format depends on the ShapeLoader class derivative.
		 * @param[out] shapeArray This will get populated with shapes if the load operation succeeds.  It should be empty, otherwise.
		 * @return True is returned on success; false, otherwise.
		 */
		virtual bool LoadShapes(const std::string& filePath, std::vector<Shape*>& shapeArray) = 0;
	};

	/**
	 * This class knows how to load polygon shapes from .OBJ files.
	 */
	class COLLISION_LIB_API OBJ_ShapeLoader : public ShapeLoader
	{
	public:
		OBJ_ShapeLoader();
		virtual ~OBJ_ShapeLoader();

		/**
		 * The given OBJ file is parsed for polygon data to generate polygon shapes which
		 * will then be put into the given array of such.
		 */
		virtual bool LoadShapes(const std::string& filePath, std::vector<Shape*>& shapeArray) override;
	};
}