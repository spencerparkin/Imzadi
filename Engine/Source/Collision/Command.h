#pragma once

#include "Task.h"
#include "Shape.h"

namespace Imzadi {
namespace Collision {

class Thread;
class Shape;

/**
 * This is the base class for all collision system comands
 * that can be issued to manipulate the collision world.
 */
class IMZADI_API Command : public Task
{
public:
	Command();
	virtual ~Command();
};

/**
 * This command is used to signal the collision thread to exit.
 * Users of the collision system never need to use it directly.
 * They can simply call the Shutdown function of the System class.
 */
class IMZADI_API ExitThreadCommand : public Command
{
public:
	ExitThreadCommand();
	virtual ~ExitThreadCommand();

	/**
	 * Signal the collision thread to shutdown.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Allocate and return an instance of the ExitThreadCommand.
	 */
	static ExitThreadCommand* Create();
};

/**
 * Not meant to be used directly, this class is the base class for any
 * shape-oriented command.  That is, any command that operates on a
 * particular shape.
 */
class IMZADI_API ShapeCommand : public Command
{
public:
	ShapeCommand();
	virtual ~ShapeCommand();

	/**
	 * Set the shape ID of the shape to be operated upon.
	 */
	void SetShapeID(ShapeID shapeID) { this->shapeID = shapeID; }

	/**
	 * Get the shape ID of the shape to be operated upon.
	 */
	ShapeID GetShapeID() const { return this->shapeID; }

protected:
	ShapeID shapeID;
};

/**
 * This command is used to add a collision shape to the system.
 */
class IMZADI_API AddShapeCommand : public Command
{
public:
	AddShapeCommand();
	virtual ~AddShapeCommand();

	/**
	 * Set the shape that is to be added to the collision system.
	 * Use the Create method of the System class with the desired shape class
	 * to create the argument.
	 */
	void SetShape(Shape* shape) { this->shape = shape; }

	/**
	 * Get the shape that is to be added to the collision system.
	 */
	Shape* GetShape() { return this->shape; }

	/**
	 * Perform the addition of the set shape to the collision system.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Set the insertion flags to be used in the add operation.
	 * These are flags of the form IMZADI_ADD_FLAG_*.
	 */
	void SetFlags(uint32_t flags) { this->flags = flags; }

	/**
	 * Get the insertion flags to be used in the add operation.
	 */
	uint32_t GetFlags() const { return this->flags; }

	/**
	 * Allocate an instance of the AddShapeCommand.
	 */
	static AddShapeCommand* Create();

private:
	Shape* shape;
	uint32_t flags;
};

/**
 * This command is used to remove a shape from the system.
 */
class IMZADI_API RemoveShapeCommand : public ShapeCommand
{
public:
	RemoveShapeCommand();
	virtual ~RemoveShapeCommand();

	/**
	 * Perform the removal of the set shape from the collision system.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Allocate an instance of the RemoveShapeCommand.
	 */
	static RemoveShapeCommand* Create();
};

/**
 * This command is used to remove all shapes from the collision world.
 */
class IMZADI_API RemoveAllShapesCommand : public Command
{
public:
	RemoveAllShapesCommand();
	virtual ~RemoveAllShapesCommand();

	/**
	 * Perform the removal of all collision shapes from the collision world.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Create a new instance of the RemoveAllShapesCommand class.
	 */
	static RemoveAllShapesCommand* Create();
};

/**
 * This command can be used to change the debug render color of a collision shape.
 * It is not meant to be used in production; just debug.
 */
class IMZADI_API SetDebugRenderColorCommand : public ShapeCommand
{
public:
	SetDebugRenderColorCommand();
	virtual ~SetDebugRenderColorCommand();

	/**
	 * Perform the debug render color assignment.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Set the color to be applied to the desired shape.
	 */
	void SetColor(const Vector3& color) { this->color = color; }

	/**
	 * Get the color to be applied to the desired shape.
	 */
	const Vector3& GetColor() const { return this->color; }

	/**
	 * Create a new instance of the SetDebugRenderColorCommand class.
	 */
	static SetDebugRenderColorCommand* Create();

private:
	Vector3 color;
};

/**
 * Use this command to change the object-to-world transform of a collision shape.
 */
class IMZADI_API ObjectToWorldCommand : public ShapeCommand
{
public:
	ObjectToWorldCommand();
	virtual ~ObjectToWorldCommand();

	/**
	 * Alter the object-to-world transform of the target shape.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Allocate and return a new instance of the ObjectToWorldCommand class.
	 */
	static ObjectToWorldCommand* Create();

public:
	Transform objectToWorld;		///< This transform is what's assigned to the target shape's object-to-world transform.
};

/**
 * This would be typically called once per frame at the top of the frame
 * before any other command or query to the collision system.
 */
class IMZADI_API ResetProfileDataCommand : public Command
{
public:
	ResetProfileDataCommand();
	virtual ~ResetProfileDataCommand();

	virtual void Execute(Thread* thread) override;

	static ResetProfileDataCommand* Create();
};

/**
 * Use this command to dump or restore the collision world to or from disk, respectively.
 * It's mainly used for debugging purposes.  It's not meant to be used in a production case.
 */
class IMZADI_API FileCommand : public Command
{
public:
	FileCommand();
	virtual ~FileCommand();

	/**
	 * Perform the dump or restore operation of the collision world.
	 */
	virtual void Execute(Thread* thread) override;

	/**
	 * Create a new instance of the FileComand class.
	 */
	static FileCommand* Create();

	enum Action
	{
		DUMP,
		RESTORE
	};

	/**
	 * Configure this command to either write the collision world shapes to file,
	 * or read them from file.
	 */
	void SetAction(Action action) { this->action = action; }

	/**
	 * Get the action this command is configured to perform.
	 */
	Action GetAction() const { return this->action; }

	/**
	 * Configure the file that is to be read from or written to.
	 */
	void SetFilePath(const std::string& filePath) { *this->filePath = filePath; }

	/**
	 * Get the file this command is configured to read from or write to.
	 */
	const std::string& GetFilePath() const { return *this->filePath; }

private:
	std::string* filePath;
	Action action;
};

} // namespace Collision {
} // namespace Imzadi {