#include "Reference.h"

uint32_t ReferenceCounted::nextHandle = 1;
ReferenceCounted::ObjectMap ReferenceCounted::objectMap;

ReferenceCounted::ReferenceCounted()
{
	this->refCount = 0;
	this->handle = nextHandle++;
	this->objectMap.insert(std::pair<uint32_t, ReferenceCounted*>(this->handle, this));
}

/*virtual*/ ReferenceCounted::~ReferenceCounted()
{
	this->objectMap.erase(this->handle);
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

/*static*/ ReferenceCounted* ReferenceCounted::GetObjectFromHandle(uint32_t handle)
{
	ObjectMap::iterator iter = objectMap.find(handle);
	if (iter == objectMap.end())
		return nullptr;

	return iter->second;
}