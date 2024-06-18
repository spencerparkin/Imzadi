#include "Error.h"

using namespace Imzadi;

//----------------------------------------- Error -----------------------------------------

Error::Error()
{
}

/*virtual*/ Error::~Error()
{
}

void Error::SendErrorMessage(const std::string& errorMessage)
{
	std::lock_guard<std::mutex> guard(this->mutex);
	for (auto pair : this->errorCaptureMap)
	{
		ErrorCapture* capture = pair.second;
		capture->Error(errorMessage);
	}
}

void Error::RegisterErrorCapture(const std::string& key, ErrorCapture* errorCapture)
{
	std::lock_guard<std::mutex> guard(this->mutex);
	this->errorCaptureMap.insert(std::pair<std::string, ErrorCapture*>(key, errorCapture));
}

void Error::UnregisterErrorCapture(const std::string& key)
{
	std::lock_guard<std::mutex> guard(this->mutex);
	this->errorCaptureMap.erase(key);
}

/*static*/ Error* Error::Get()
{
	static Error error;
	return &error;
}

//----------------------------------------- ErrorCapture -----------------------------------------

ErrorCapture::ErrorCapture()
{
}

/*virtual*/ ErrorCapture::~ErrorCapture()
{
}