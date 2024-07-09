#pragma once

#include "Defines.h"
#include "Reference.h"
#include <list>
#include <string>
#include <unordered_map>
#include <functional>

namespace Imzadi
{
	class Event;
	class EventListener;
	typedef std::function<void(const Event*)> EventListenerCallback;
	typedef uint32_t EventListenerHandle;

	/**
	 * This is a simple event sending and dispatch system.  The idea here is to
	 * decouple the systems that generate events from the systems that want to
	 * respond to events.  Neither should necessarily have knowledge of the other.
	 * Further, a system shouldn't necessarily care who responds to events it
	 * generates.  Conversely, a system shouldn't necessarily care where events
	 * it processes come from.
	 */
	class IMZADI_API EventSystem
	{
	public:
		EventSystem();
		virtual ~EventSystem();

		/**
		 * This will send the given event asynchronously.  That is,
		 * the event is queued, and will be dispatched to all listeners
		 * at an appropriate time in the frame.  We don't dispatch here
		 * so as to prevent re-entrancy/recursion into the event system.
		 * 
		 * @param[in] channelMask All registered listeners having channel flags not masked out by the given mask will be recipients of the event.
		 * @param[in] event This is the event to send, allocated on the heap.  The system takes ownership of the memory.
		 */
		void SendEvent(uint64_t channelMask, Event* event);

		/**
		 * Register an event listener with the system.
		 * 
		 * @return A handle is returned that the user can pass to the UnregisterEventListener method.
		 */
		EventListenerHandle RegisterEventListener(Reference<EventListener> eventListener);

		/**
		 * Unregister a previously registered event listener.
		 * 
		 * @param[in] eventListenerHandle This is the handle returned from the RegisterEventListener method.
		 */
		void UnregisterEventListener(EventListenerHandle eventListenerHandle);

		/**
		 * This should get called once per frame to send all queued events.
		 */
		void DispatchAllPendingEvents();

		/**
		 * Unregister all event listeners and delete any pending events.
		 */
		void Clear();

	private:

		typedef std::unordered_map<EventListenerHandle, Reference<EventListener>> EventListenerMap;
		EventListenerMap eventListenerMap;
		EventListenerHandle nextHandle;
		std::list<Event*> eventQueue;
	};

	/**
	 * Events are what flow through the event system from where
	 * they're generated to where they're processed.  They're
	 * all just derivatives of this class.  Processors are expected
	 * to know how to cast them or to use a dynamic cast.
	 */
	class IMZADI_API Event
	{
	public:
		Event();
		virtual ~Event();

		void SetName(const std::string& name) { this->name = name; }
		const std::string& GetName() const { return this->name; }

		void SetChannelMask(uint64_t channelMask) { this->channelMask = channelMask; }
		uint64_t GetChannelMask() const { return this->channelMask; }

	protected:
		std::string name;
		uint64_t channelMask;
	};

	/**
	 * Any class that wishes to be able to process events should
	 * derive from this class.  To avoid multiple inheritance,
	 * you may wish to use the LambdaEventListener class, or
	 * simply own an instance of a class that derives from this class.
	 * In any case, be sure you don't derive from ReferenceCounted
	 * more than once.  Virtual inheritance is not being used here,
	 * nor would that help anyway.
	 */
	class IMZADI_API EventListener : public ReferenceCounted
	{
	public:
		EventListener(uint64_t channelFlags);
		virtual ~EventListener();

		virtual void ProcessEvent(Event* event) = 0;

		void SetChannelFlags(uint64_t channelFlags) { this->channelFlags = channelFlags; }

		uint64_t GetChannelFlags() const { return this->channelFlags; }

	protected:
		uint64_t channelFlags;
	};

	/**
	 * This is a general-purpose event-listener that the application can
	 * use instead of making their own derivative of the EventListener class.
	 * It is convenient, but the user should be cautious to make sure that
	 * any variables captured by the lambda don't go stale while the listener
	 * is registered with the system.  That is, the listener should get unregistered
	 * before any such variables are deleted.
	 */
	class IMZADI_API LambdaEventListener : public EventListener
	{
	public:
		LambdaEventListener(uint64_t channelFlags, EventListenerCallback callback);
		virtual ~LambdaEventListener();

		virtual void ProcessEvent(Event* event) override;

	private:
		EventListenerCallback callback;
	};
}