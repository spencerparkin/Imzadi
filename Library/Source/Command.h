#pragma once

#include "Task.h"
#include "Shape.h"

namespace Collision
{
	class Thread;
	class Shape;

	/**
	 * This is the base class for all collision system comands
	 * that can be issued to manipulate the collision world.
	 */
	class COLLISION_LIB_API Command : public Task
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
	class COLLISION_LIB_API ExitThreadCommand : public Command
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
	 * This command is used to add a collision shape to the system.
	 */
	class COLLISION_LIB_API AddShapeCommand : public Command
	{
	public:
		AddShapeCommand();
		virtual ~AddShapeCommand();

		/**
		 * Set the shape that is to be added to the collision system.
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
		 * Allocate an instance of the AddShapeCommand.
		 */
		static AddShapeCommand* Create();

	private:
		Shape* shape;
	};

	/**
	 * This command is used to remove a shape from the system.
	 */
	class COLLISION_LIB_API RemoveShapeCommand : public Command
	{
	public:
		RemoveShapeCommand();
		virtual ~RemoveShapeCommand();

		/**
		 * Set the shape ID of the shape to be removed from the collision system.
		 */
		void SetShapeID(ShapeID shapeID) { this->shapeID = shapeID; }

		/**
		 * Get the shape ID of the shape tob e removed from the collision system.
		 */
		ShapeID GetShapeID() const { return this->shapeID; }

		/**
		 * Perform the removal of the set shape from the collision system.
		 */
		virtual void Execute(Thread* thread) override;

		/**
		 * Allocate an instance of the RemoveShapeCommand.
		 */
		static RemoveShapeCommand* Create();

	private:
		ShapeID shapeID;
	};

	/**
	 * This command is used to remove all shapes from the collision world.
	 */
	class COLLISION_LIB_API RemoveAllShapesCommand : public Command
	{
	public:
		RemoveAllShapesCommand();
		virtual ~RemoveAllShapesCommand();

		virtual void Execute(Thread* thread) override;

		static RemoveAllShapesCommand* Create();
	};
}