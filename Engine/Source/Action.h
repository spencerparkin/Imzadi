#pragma once

#include "Reference.h"
#include <unordered_map>

namespace Imzadi
{
	/**
	 * These are actions a player can perform, if they so choose.
	 */
	class IMZADI_API Action : public ReferenceCounted
	{
	public:
		Action();
		virtual ~Action();

		/**
		 * Here you might show give the player an indication that the action can be taken.
		 */
		virtual void Init();

		/**
		 * Clean-up anything you did in Init().
		 */
		virtual void Deinit();

		/**
		 * Here you might provide any needed animation.  No need
		 * to override this at all if you don't need it.
		 */
		virtual void Tick(double deltaTime);

		/**
		 * Override this to actually perform the action.  Doing so might,
		 * for example, be as simple as firing an event, but it could be
		 * anything.
		 * 
		 * @return The override should return true if and only if the action should be retained in the system.
		 */
		virtual bool Perform() = 0;
	};

	/**
	 * Manage a collection of actions, each bound to an input.
	 * A player character/biped might own one of these.
	 */
	class IMZADI_API ActionManager
	{
	public:
		ActionManager();
		virtual ~ActionManager();

		/**
		 * Bind the given action to the given controller button.
		 * Anything that was already bound to that key is unbound.
		 */
		void BindAction(uint32_t inputKey, Action* action);

		/**
		 * Unbind anything bound to the given controller button.
		 */
		void UnbindAction(uint32_t inputKey);

		/**
		 * This doesn't just tick the currently bound actions.  It also
		 * handles input from the controller to perform the bound actions,
		 * if any.
		 */
		void Tick(double deltaTime);

		/**
		 * Remove all currently bound actions.
		 */
		void Clear();

		void SetControllerUser(const std::string& controllerUser) { this->controllerUser = controllerUser; }
		const std::string& GetControllerUser() const { return this->controllerUser; }

	private:
		std::string controllerUser;

		typedef std::unordered_map<uint32_t, Reference<Action>> ActionMap;
		ActionMap actionMap;
	};
}