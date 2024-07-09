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
	EventChannel* channel = this->GetOrCreateChannel(channelName, false);
	if (!channel)
		return false;

	channel->EnqueueEvent(event);
	return true;
}

EventListenerHandle EventSystem::RegisterEventListener(const std::string& channelName, Reference<EventListener> eventListener)
{
	EventListenerHandle handle = this->nextHandle++;
	EventChannel* channel = this->GetOrCreateChannel(channelName, true);
	IMZADI_ASSERT(channel);
	if (!channel->AddSubscriber(handle, eventListener))
		handle = 0;
	return handle;
}

bool EventSystem::UnregisterEventListener(EventListenerHandle eventListenerHandle)
{
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
	for (auto pair : this->eventChannelMap)
	{
		EventChannel* channel = pair.second;
		channel->DispatchQueue();
	}
}

void EventSystem::Clear()
{
	this->eventChannelMap.clear();
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

void EventChannel::DispatchQueue()
{
	while (this->eventQueue.size() > 0)
	{
		std::list<Event*>::iterator iter = this->eventQueue.begin();
		Event* event = *iter;
		this->eventQueue.erase(iter);

		for (auto pair : this->eventListenerMap)
		{
			EventListener* eventListener = pair.second;
			eventListener->ProcessEvent(event);
		}

		delete event;
	}
}

void EventChannel::Clear()
{
	for (Event* event : this->eventQueue)
		delete event;

	this->eventQueue.clear();
}

//-------------------------------------- Event --------------------------------------

Event::Event()
{
	this->name = "?";
}

/*virtual*/ Event::~Event()
{
}

//-------------------------------------- EventListener --------------------------------------

EventListener::EventListener()
{
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