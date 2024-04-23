#include "Error.h"
#include <format>

using namespace Collision;

Error::Error()
{
	this->errorMessageArray = new std::vector<std::string>();
	this->mutex = new std::mutex();
}

/*virtual*/ Error::~Error()
{
	delete this->errorMessageArray;
	delete this->mutex;
}

void Error::Clear()
{
	std::lock_guard<std::mutex> guard(*this->mutex);
	this->errorMessageArray->clear();
}

void Error::AddErrorMessage(const std::string& errorMessage)
{
	std::lock_guard<std::mutex> guard(*this->mutex);
	this->errorMessageArray->push_back(errorMessage);
}

int Error::GetCount() const
{
	return this->errorMessageArray->size();
}

std::string Error::GetAllErrorMessages() const
{
	std::lock_guard<std::mutex> guard(*this->mutex);

	std::string formattedMessage = std::format("Totals errors: {}\n\n", this->GetCount());

	int i = 0;
	for (const std::string& errorMessage : *this->errorMessageArray)
		formattedMessage += std::format("{}: {}\n", i++, errorMessage);

	return formattedMessage;
}

namespace Collision
{
	Error* GetError()
	{
		static Error error;
		return &error;
	}
}