#include "Reference.h"

ReferenceCounted::ReferenceCounted()
{
	this->refCount = 0;
}

/*virtual*/ ReferenceCounted::~ReferenceCounted()
{
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