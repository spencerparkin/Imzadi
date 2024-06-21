#include "Reference.h"

using namespace Imzadi;

std::atomic<uint32_t> ReferenceCounted::nextHandle(1);

//---------------------------- ReferenceCounted ----------------------------

ReferenceCounted::ReferenceCounted()
{
	this->refCount = 0;
	this->handle = nextHandle++;
	HandleManager::Get()->Register(this);	// TODO: May need to optimize this to be O(1).
}

/*virtual*/ ReferenceCounted::~ReferenceCounted()
{
	HandleManager::Get()->Unregister(this);	// TODO: May need to optimize this to be O(1).
}

void ReferenceCounted::IncRef() const
{
	this->refCount++;
}

void ReferenceCounted::DecRef() const
{
	if (this->refCount > 0)
	{
		this->refCount--;

		if (this->refCount == 0)
			delete this;
	}
}

//---------------------------- HandleManager ----------------------------

HandleManager::HandleManager()
{
}

/*virtual*/ HandleManager::~HandleManager()
{
}

void HandleManager::Register(ReferenceCounted* refCounted)
{
	std::lock_guard guard(this->mutex);
	this->objectMap.insert(std::pair<uint32_t, ReferenceCounted*>(refCounted->GetHandle(), refCounted));
}

void HandleManager::Unregister(ReferenceCounted* refCounted)
{
	std::lock_guard guard(this->mutex);
	this->objectMap.erase(refCounted->GetHandle());
}

bool HandleManager::GetObjectFromHandle(uint32_t handle, Reference<ReferenceCounted>& ref)
{
	std::lock_guard guard(this->mutex);
	ObjectMap::iterator iter = this->objectMap.find(handle);
	if (iter != this->objectMap.end() && iter->second->GetRefCount() > 0)
	{
		ref.Set(iter->second);
		return true;
	}

	return false;
}

/*static*/ HandleManager* HandleManager::Get()
{
	static HandleManager manager;
	return &manager;
}