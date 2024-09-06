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

bool EventSystem::SendEvent(const std::string& channelName, Event* event)
{
	std::scoped_lock lock(this->mutex);

	EventChannel* channel = this->GetOrCreateChannel(channelName, false);
	if (!channel)
		return false;

	channel->EnqueueEvent(event);
	return true;
}

bool EventSystem::SendEventNow(const std::string& channelName, Event* event)
{
	std::scoped_lock lock(this->mutex);

	EventChannel* channel = this->GetOrCreateChannel(channelName, false);
	if (!channel)
		return false;

	channel->DispatchEventNow(event);
	delete event;
	return true;
}

EventListenerHandle EventSystem::RegisterEventListener(const std::string& channelName, EventListenerType eventListenerType, Reference<EventListener> eventListener)
{
	std::scoped_lock lock(this->mutex);

	eventListener->eventListenerType = eventListenerType;
	EventListenerHandle handle = this->nextHandle++;
	EventChannel* channel = this->GetOrCreateChannel(channelName, true);
	IMZADI_ASSERT(channel);
	if (!channel->AddSubscriber(handle, eventListener))
		handle = 0;
	return handle;
}

bool EventSystem::UnregisterEventListener(EventListenerHandle eventListenerHandle)
{
	std::scoped_lock lock(this->mutex);

	for (auto pair : this->eventChannelMap)
	{
		EventChannel* channel = pair.second;
		if (channel->RemoveSubscriber(eventListenerHandle))
			return true;
	}

	return false;
}

void EventSystem::DispatchAllPendingEvents()
{
	std::scoped_lock lock(this->mutex);

	std::vector<EventDispatch> eventDispatchArray;
	for (auto pair : this->eventChannelMap)
	{
		EventChannel* channel = pair.second;
		channel->GenerateDispatches(eventDispatchArray);
	}

	// We call the listeners like this, because it should be
	// fine for an event handler to mutate the event system,
	// because we can't let the event system be mutated while
	// we're trying to iterate over its data-structures.
	for (auto& eventDispatch : eventDispatchArray)
		eventDispatch.listener->ProcessEvent(eventDispatch.event);
}

void EventSystem::Clear()
{
	std::scoped_lock lock(this->mutex);
	this->eventChannelMap.clear();
}

void EventSystem::ResetForNextLevel()
{
	std::scoped_lock lock(this->mutex);

	for (auto pair : this->eventChannelMap)
	{
		EventChannel* channel = pair.second;
		channel->ResetForNextLevel();
	}
}

EventChannel* EventSystem::GetOrCreateChannel(const std::string& channelName, bool canCreateIfNoneExistent)
{
	EventChannel* channel = nullptr;

	EventChannelMap::iterator iter = this->eventChannelMap.find(channelName);
	if (iter != this->eventChannelMap.end())
		channel = iter->second;
	else if (canCreateIfNoneExistent)
	{
		channel = new EventChannel();
		this->eventChannelMap.insert(std::pair<std::string, Reference<EventChannel>>(channelName, channel));
	}

	return channel;
}

//-------------------------------------- EventChannel --------------------------------------

EventChannel::EventChannel()
{
}

/*virtual*/ EventChannel::~EventChannel()
{
	this->Clear();
}

bool EventChannel::AddSubscriber(EventListenerHandle eventListenerHandle, Reference<EventListener>& eventListener)
{
	EventListenerMap::iterator iter = this->eventListenerMap.find(eventListenerHandle);
	if (iter != this->eventListenerMap.end())
		return false;

	this->eventListenerMap.insert(std::pair<EventListenerHandle, Reference<EventListener>>(eventListenerHandle, eventListener));
	return true;
}

bool EventChannel::RemoveSubscriber(EventListenerHandle eventListenerHandle)
{
	EventListenerMap::iterator iter = this->eventListenerMap.find(eventListenerHandle);
	if (iter == this->eventListenerMap.end())
		return false;

	this->eventListenerMap.erase(iter);
	return true;
}

void EventChannel::EnqueueEvent(Event* event)
{
	this->eventQueue.push_back(event);
}

void EventChannel::GenerateDispatches(std::vector<EventDispatch>& eventDispatchArray)
{
	while (this->eventQueue.size() > 0)
	{
		std::list<Reference<Event>>::iterator iter = this->eventQueue.begin();
		Event* event = *iter;

		for (auto pair : this->eventListenerMap)
		{
			EventListener* eventListener = pair.second;
			eventDispatchArray.push_back({ eventListener, event });
		}

		this->eventQueue.erase(iter);
	}
}

void EventChannel::DispatchEventNow(Event* event)
{
	for (auto pair : this->eventListenerMap)
	{
		EventListener* eventListener = pair.second;
		eventListener->ProcessEvent(event);
	}
}

void EventChannel::Clear()
{
	for (Event* event : this->eventQueue)
		delete event;

	this->eventQueue.clear();
}

void EventChannel::ResetForNextLevel()
{
	this->Clear();

	std::vector<EventListenerHandle> doomedListenersArray;

	for (auto pair : this->eventListenerMap)
	{
		EventListener* eventListener = pair.second;
		if (eventListener->eventListenerType == EventListenerType::TRANSITORY)
			doomedListenersArray.push_back(pair.first);
	}

	for (auto handle : doomedListenersArray)
		this->eventListenerMap.erase(handle);
}

//-------------------------------------- Event --------------------------------------

Event::Event()
{
	this->name = "?";
}

Event::Event(const std::string& name)
{
	this->name = name;
}

/*virtual*/ Event::~Event()
{
}

//-------------------------------------- EventListener --------------------------------------

EventListener::EventListener()
{
	this->eventListenerType = EventListenerType::TRANSITORY;
}

/*virtual*/ EventListener::~EventListener()
{
}

//-------------------------------------- LambdaEventListener --------------------------------------

LambdaEventListener::LambdaEventListener(EventListenerCallback callback)
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