#pragma once

#include "Defines.h"
#include <string>
#include <vector>

namespace Imzadi {
namespace Collision {

class Shape;

/**
 * This is the base class for any derivative that knows how to load shapes
 * from a file on disk.
 */
class IMZADI_API ShapeLoader
{
public:
	ShapeLoader();
	virtual ~ShapeLoader();

	/**
	 * Overrides should impliment this method by populating the given array with
	 * a bunch of shapes from the given file.  This is done on the main thread and
	 * then the shapes can be handed over to the collision system or used for any
	 * other purpose.  The override should append to whatever is already in the
	 * given array.
	 *
	 * @param[in] filePath This should be a fully-qualified path to the file containing the shapes.  The format depends on the ShapeLoader class derivative.
	 * @param[out] shapeArray This will get populated with shapes if the load operation succeeds.  It should be empty, otherwise.
	 * @return True is returned on success; false, otherwise.
	 */
	virtual bool LoadShapes(const std::string& filePath, std::vector<Shape*>& shapeArray) = 0;

	/**
	 * Overrides should return true if the derived class can load files
	 * of the type specified by the given file extension.
	 *
	 * @param[in] fileExtension This will be something of the form, for example, ".xyz".
	 * @return True is returned if the loader can handle the given file type; false, otherwise.
	 */
	virtual bool CanLoadFileType(const std::string& fileExtension) = 0;

	/**
	 * Call this to create a ShapeLoader class derivative that can handle the given file.
	 * Be sure to call the Free function when you're done with it.
	 */
	static ShapeLoader* Create(const std::string& filePath);

	/**
	 * Reclaim the memory associated with the given shape loader.
	 */
	static void Free(ShapeLoader* shapeLoader);
};

/**
 * This class defines the interface to a class that represents a ShapeLoader class.
 */
class ShapeLoaderClassInterface
{
public:
	virtual ShapeLoader* Create() = 0;
};

/**
 * This class represents a ShapeLoader class derivative.
 */
template<typename Type>
class ShapeLoaderClass : public ShapeLoaderClassInterface
{
public:
	virtual ShapeLoader* Create() override
	{
		return new Type;
	}
};

/**
 * This class knows how to load polygon shapes from .OBJ files.
 */
class IMZADI_API OBJ_ShapeLoader : public ShapeLoader
{
public:
	OBJ_ShapeLoader();
	virtual ~OBJ_ShapeLoader();

	/**
	 * The given OBJ file is parsed for polygon data to generate polygon shapes which
	 * will then be put into the given array of such.
	 */
	virtual bool LoadShapes(const std::string& filePath, std::vector<Shape*>& shapeArray) override;

	/**
	 * We return true here if and only if the given extension is, ignoring case, ".obj".
	 */
	virtual bool CanLoadFileType(const std::string& fileExtension) override;

private:
	bool Tokenize(const std::string& line, std::vector<std::string>& tokenArray);
};

} // namespace Collision {
} // namespace Imzadi {