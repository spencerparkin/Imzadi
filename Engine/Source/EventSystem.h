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
	class EventChannel;
	typedef std::function<void(const Event*)> EventListenerCallback;
	typedef uint64_t EventListenerHandle;
	typedef std::unordered_map<EventListenerHandle, Reference<EventListener>> EventListenerMap;
	typedef std::unordered_map<std::string, Reference<EventChannel>> EventChannelMap;

	struct EventDispatch
	{
		Reference<EventListener> listener;
		Reference<Event> event;
	};

	/**
	 * This is a simple event sending and dispatch system.  The idea here is to
	 * decouple the systems that generate events from the systems that want to
	 * respond to events.  Neither should necessarily have knowledge of the other.
	 * Further, a system shouldn't necessarily care who responds to events it
	 * generates.  Conversely, a system shouldn't necessarily care where events
	 * it processes come from.  The event system is a message broker.
	 * 
	 * Note that this class *should* be thread-safe, but you probably don't want
	 * any event listener to be dispatched on any thread other than the main thread.
	 */
	class IMZADI_API EventSystem
	{
	public:
		EventSystem();
		virtual ~EventSystem();

		/**
		 * This will send the given event asynchronously.  That is,
		 * the event is queued, and will be dispatched to all applicable
		 * listeners at an appropriate time in the frame.  We don't dispatch
		 * here so as to prevent re-entrancy/recursion into the event system.
		 * 
		 * @param[in] channelName Listeners subscribed to this channel will receive the event.
		 * @param[in] event This is the event to send, allocated on the heap.  The system takes ownership of the memory.
		 * @return True is returned on success; false, otherwise.  We can fail here if a channel by the given name does not exist.
		 */
		bool SendEvent(const std::string& channelName, Event* event);

		/**
		 * This will send the given event synchronously.  That is, the event
		 * will have been sent and processed by all listeners before this call
		 * returns.  Care must be taken to prevent infinite recursion into the
		 * event system.
		 * 
		 * Note that thread-safety can't be guarenteed here as far as the
		 * event listeners are concerned, because that is clearly out of the
		 * hands of this system.  It's probably best to only call this from
		 * the main thread.
		 */
		bool SendEventNow(const std::string& channelName, Event* event);

		/**
		 * Register an event listener with the system.
		 * 
		 * @param[in] channelName The given listener will receive events from this channel.  If the channel doesn't exist, it is created.
		 * @param[in] eventListener This is a derivative of the EventListener class which will process events on the given channel.
		 * @return A handle is returned that the user can pass to the UnregisterEventListener method.  Note that zero is an invalid handle value.
		 */
		EventListenerHandle RegisterEventListener(const std::string& channelName, Reference<EventListener> eventListener);

		/**
		 * Unregister a previously registered event listener.
		 * 
		 * @param[in] eventListenerHandle This is the handle returned from the RegisterEventListener method.
		 * @return True is returned on success; false, otherwise.
		 */
		bool UnregisterEventListener(EventListenerHandle eventListenerHandle);

		/**
		 * This should get called once per frame to send all queued events.
		 * This should probably only get called from the main thread, because
		 * event listeners are called here and they can't be expected to be
		 * thread-safe.
		 */
		void DispatchAllPendingEvents();

		/**
		 * Unregister all event listeners and delete any pending events.
		 */
		void Clear();

	private:

		std::recursive_mutex mutex;

		EventChannel* GetOrCreateChannel(const std::string& channelName, bool canCreateIfNoneExistent);

		EventChannelMap eventChannelMap;
		EventListenerHandle nextHandle;
	};

	/**
	 * These are used internally by the EventSystem class to manage channels.
	 */
	class EventChannel : public ReferenceCounted
	{
	public:
		EventChannel();
		virtual ~EventChannel();

		bool AddSubscriber(EventListenerHandle eventListenerHandle, Reference<EventListener>& eventListener);
		bool RemoveSubscriber(EventListenerHandle eventListenerHandle);
		void EnqueueEvent(Event* event);
		void GenerateDispatches(std::vector<EventDispatch>& eventDispatchArray);
		void DispatchEventNow(Event* event);
		void Clear();

	private:
		EventListenerMap eventListenerMap;
		std::list<Reference<Event>> eventQueue;
	};

	/**
	 * Events are what flow through the event system from where
	 * they're generated to where they're processed.  They're
	 * all just derivatives of this class.  Processors are expected
	 * to know how to cast them or to use a dynamic cast.
	 */
	class IMZADI_API Event : public ReferenceCounted
	{
	public:
		Event();
		Event(const std::string& name);
		virtual ~Event();

		void SetName(const std::string& name) { this->name = name; }
		const std::string& GetName() const { return this->name; }

	protected:
		std::string name;
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
		EventListener();
		virtual ~EventListener();

		virtual void ProcessEvent(Event* event) = 0;
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
		LambdaEventListener(EventListenerCallback callback);
		virtual ~LambdaEventListener();

		virtual void ProcessEvent(Event* event) override;

	private:
		EventListenerCallback callback;
	};
}