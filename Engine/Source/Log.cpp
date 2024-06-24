#include "Log.h"
#include <ctime>
#include <stdarg.h>
#include <windows.h>

using namespace Imzadi;

//------------------------------- LoggingSystem -------------------------------

LoggingSystem::LoggingSystem()
{
}

/*virtual*/ LoggingSystem::~LoggingSystem()
{
	this->ClearAllRoutes();
}

void LoggingSystem::PrintLogMessage(uint32_t messageFlags, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char logMessageBuffer[1024];
	vsprintf_s(logMessageBuffer, sizeof(logMessageBuffer), format, args);

	std::string logMessage(logMessageBuffer);
	this->PrintLogMessage(messageFlags, logMessage);

	va_end(args);
}

void LoggingSystem::PrintLogMessage(uint32_t messageFlags, const std::string& logMessage)
{
	std::time_t time = std::time(nullptr);
	char timeBuffer[128];
	std::strftime(timeBuffer, sizeof(timeBuffer), "%T", std::localtime(&time));
	std::string timeStampedLogMessage = std::format("{}: {}\n", timeBuffer, logMessage.c_str());

	std::lock_guard guard(this->mutex);

	for (auto pair : this->logRouteMap)
	{
		LogRoute* logRoute = pair.second;
		if ((logRoute->GetFilterFlags() & messageFlags) != 0)
			logRoute->PrintLogMessage(messageFlags, timeStampedLogMessage);
	}

	if ((messageFlags & IMZADI_LOG_FATAL_ERROR_FLAG) != 0)
	{
		// We've encountered an error so bad that the program can't continue.
		// Just hang out here until we're terminated.

		if (IsDebuggerPresent())
		{
			while (true)
			{
				DebugBreak();
			}
		}
		else
		{
			while (true)
			{
				IMZADI_ASSERT(false);
			}
		}
	}
}

bool LoggingSystem::AddRoute(LogRoute* logRoute)
{
	std::lock_guard guard(this->mutex);
	
	LogRouteMap::iterator iter = this->logRouteMap.find(logRoute->GetName());
	if (iter != this->logRouteMap.end())
		return false;

	this->logRouteMap.insert(std::pair<std::string, LogRoute*>(logRoute->GetName(), logRoute));
	logRoute->RouteRegistered();
	return true;
}

bool LoggingSystem::RemoveRoute(const std::string& logRouteName)
{
	std::lock_guard guard(this->mutex);

	LogRouteMap::iterator iter = this->logRouteMap.find(logRouteName);
	if (iter == this->logRouteMap.end())
		return false;

	LogRoute* logRoute = iter->second;
	logRoute->RouteUnregistered();
	this->logRouteMap.erase(iter);
	return true;
}

bool LoggingSystem::RouteExists(const std::string& logRouteName)
{
	std::lock_guard guard(this->mutex);

	return this->logRouteMap.find(logRouteName) != this->logRouteMap.end();
}

void LoggingSystem::ClearAllRoutes()
{
	std::lock_guard guard(this->mutex);

	for (auto pair : this->logRouteMap)
	{
		LogRoute* logRoute = pair.second;
		logRoute->RouteUnregistered();
	}

	this->logRouteMap.clear();
}

void LoggingSystem::SetRouteFilter(const std::string& routeName, uint32_t filterFlags)
{
	std::lock_guard guard(this->mutex);

	if (routeName.length() == 0)
	{
		for (auto pair : this->logRouteMap)
		{
			LogRoute* logRoute = pair.second;
			logRoute->SetFilterFlags(filterFlags);
		}
	}
	else
	{
		LogRouteMap::iterator iter = this->logRouteMap.find(routeName);
		if (iter != this->logRouteMap.end())
		{
			LogRoute* logRoute = iter->second;
			logRoute->SetFilterFlags(filterFlags);
		}
	}
}

uint32_t LoggingSystem::GetRouteFilter(const std::string& routeName)
{
	std::lock_guard guard(this->mutex);

	LogRouteMap::iterator iter = this->logRouteMap.find(routeName);
	if (iter == this->logRouteMap.end())
		return 0;

	LogRoute* logRoute = iter->second;
	return logRoute->GetFilterFlags();
}

/*static*/ LoggingSystem* LoggingSystem::Get()
{
	static LoggingSystem system;
	return &system;
}

//------------------------------- LogRoute -------------------------------

LogRoute::LogRoute()
{
	this->filterFlags = 0;
}

/*virtual*/ LogRoute::~LogRoute()
{
}

/*virtual*/ void LogRoute::RouteRegistered()
{
}

/*virtual*/ void LogRoute::RouteUnregistered()
{
}

//------------------------------- LogFileRoute -------------------------------

LogFileRoute::LogFileRoute()
{
	this->SetName("log_file");
	this->SetFilterFlags(IMZADI_LOG_ALL_FLAGS);
}

/*virtual*/ LogFileRoute::~LogFileRoute()
{
}

/*virtual*/ void LogFileRoute::PrintLogMessage(uint32_t messageFlags, const std::string& logMessage)
{
	if (this->fileStream.is_open())
	{
		this->fileStream << logMessage;
		this->fileStream.flush();
	}
}

/*virtual*/ void LogFileRoute::RouteRegistered()
{
	if (std::filesystem::exists(this->logFilePath))
		std::filesystem::remove(this->logFilePath);

	this->fileStream.open(this->logFilePath, std::ios::out);
}

/*virtual*/ void LogFileRoute::RouteUnregistered()
{
	this->fileStream.close();
}

//------------------------------- LogFileRoute -------------------------------

LogConsoleRoute::LogConsoleRoute()
{
	this->SetName("log_console");
	this->SetFilterFlags(IMZADI_LOG_ALL_FLAGS);
}

/*virtual*/ LogConsoleRoute::~LogConsoleRoute()
{
}

/*virtual*/ void LogConsoleRoute::PrintLogMessage(uint32_t messageFlags, const std::string& logMessage)
{
	OutputDebugStringA(logMessage.c_str());
}