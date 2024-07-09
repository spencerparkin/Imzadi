#include "EventSystem.h"

using namespace Imzadi;

//-------------------------------------- EventSystem --------------------------------------

EventSystem::EventSystem()
{
	this->nextHandle = 1;
}

/*virtual*/ EventSystem::~EventSystem()
{
	this->Clear();
}

void EventSystem::SendEvent(uint64_t channelMask, Event* event)
{
	event->SetChannelMask(channelMask);
	this->eventQueue.push_back(event);
}

EventListenerHandle EventSystem::RegisterEventListener(Reference<EventListener> eventListener)
{
	EventListenerHandle handle = this->nextHandle++;
	this->eventListenerMap.insert(std::pair<EventListenerHandle, Reference<EventListener>>(handle, eventListener));
	return handle;
}

void EventSystem::UnregisterEventListener(EventListenerHandle eventListenerHandle)
{
	this->eventListenerMap.erase(eventListenerHandle);
}

void EventSystem::DispatchAllPendingEvents()
{
	while (this->eventQueue.size() > 0)
	{
		std::list<Event*>::iterator iter = this->eventQueue.begin();
		Event* event = *iter;
		this->eventQueue.erase(iter);

		uint64_t channelMask = event->GetChannelMask();

		for (auto pair : this->eventListenerMap)
		{
			Reference<EventListener>& eventListener = pair.second;
			uint64_t channelFlags = eventListener->GetChannelFlags();

			if ((channelFlags & channelMask) != 0)
				eventListener->ProcessEvent(event);
		}

		delete event;
	}
}

void EventSystem::Clear()
{
	this->eventListenerMap.clear();

	for (Event* event : this->eventQueue)
		delete event;

	this->eventQueue.clear();
}

//-------------------------------------- Event --------------------------------------

Event::Event()
{
	this->name = "?";
	this->channelMask = 0;
}

/*virtual*/ Event::~Event()
{
}

//-------------------------------------- EventListener --------------------------------------

EventListener::EventListener(uint64_t channelFlags)
{
	this->channelFlags = channelFlags;
}

/*virtual*/ EventListener::~EventListener()
{
}

//-------------------------------------- LambdaEventListener --------------------------------------

LambdaEventListener::LambdaEventListener(uint64_t channelFlags, EventListenerCallback callback) : EventListener(channelFlags)
{
	this->callback = callback;
}

/*virtual*/ LambdaEventListener::~LambdaEventListener()
{
}

/*virtual*/ void LambdaEventListener::ProcessEvent(Event* event)
{
	this->callback(event);
}