#pragma once

#include "Defines.h"
#include "Reference.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <filesystem>
#include <fstream>

#define IMZADI_LOG_FATAL_ERROR_FLAG			0x00000001
#define IMZADI_LOG_ERROR_FLAG				0x00000002
#define IMZADI_LOG_WARNING_FLAG				0x00000004
#define IMZADI_LOG_INFO_FLAG				0x00000008
#define IMZADI_LOG_ALL_FLAGS				0xFFFFFFFF

#if defined _DEBUG
#	define IMZADI_LOG_FATAL_ERROR(format, ...)		Imzadi::LoggingSystem::Get()->PrintLogMessage(IMZADI_LOG_FATAL_ERROR_FLAG, format, __VA_ARGS__)
#	define IMZADI_LOG_ERROR(format, ...)			Imzadi::LoggingSystem::Get()->PrintLogMessage(IMZADI_LOG_ERROR_FLAG, format, __VA_ARGS__)
#	define IMZADI_LOG_WARNING(format, ...)			Imzadi::LoggingSystem::Get()->PrintLogMessage(IMZADI_LOG_WARNING_FLAG, format, __VA_ARGS__)
#	define IMZADI_LOG_INFO(format, ...)				Imzadi::LoggingSystem::Get()->PrintLogMessage(IMZADI_LOG_INFO_FLAG, format, __VA_ARGS__)
#else
#	define IMZADI_LOG_FATAL_ERROR(format, ...)
#	define IMZADI_LOG_ERROR(format, ...)
#	define IMZADI_LOG_WARNING(format, ...)
#	define IMZADI_LOG_INFO(format, ...)
#endif

namespace Imzadi
{
	class LogRoute;

	/**
	 * When the Imzadi Game Engine encounters errors, it always tries to
	 * recover and continue if possible, or else it just halts.  In any case,
	 * logging is used to report what went wrong and why (as far as I haven't
	 * been too lazy to include logging in the code.)  In a release build,
	 * logging will completely compile out.
	 * 
	 * The logging system will be thread-safe so that logs can be generated
	 * from any thread.  Also, the issuance of errors and warnings in the
	 * logs can be a way of trapping faults when debugging.
	 * 
	 * Different types of log messages will be supported here so that the logs
	 * can be filtered on the receiving end.  Also, logs can be routed wherever
	 * the user wants them to go.
	 * 
	 * Logging is an important part of the engine, and so some effort has been
	 * made here to provide a reasonably robust and extendable logging system.
	 * I also favor it greatly over other methods of error management, exception-
	 * throwing in particular.  In my world, the program exits cleanly and
	 * memory-leak free under any circumstance, even errors, or just halts
	 * outright if it can't continue.
	 */
	class IMZADI_API LoggingSystem
	{
	public:
		LoggingSystem();
		virtual ~LoggingSystem();

		/**
		 * The is the main entry-point for all logging of messages.  Rather than
		 * calling it directly, prefer to use the C-preprocessor macros so that
		 * all logging and compile out of a release build.
		 * 
		 * Note that there are plenty of free bits in the message flags parameter
		 * for you to create your own flags for your own purposes.
		 * 
		 * @param[in] messageFlags This is an OR-ing of IMZADI_LOG_*_FLAG defines.  Only routes with filters overlapping these flags will receive the message.
		 * @param[in] format This is a format string similar to that used in the printf() function.
		 */
		void PrintLogMessage(uint32_t messageFlags, const char* format, ...);

		/**
		 * This variant takes std::string instead of a C-string in case std::format() is preferred.
		 */
		void PrintLogMessage(uint32_t messageFlags, const std::string& logMessage);

		/**
		 * Register the given logging route with the system.
		 * 
		 * @param[in] logRoute This is a pointer to the heap-allocated route.  The logging system takes ownership of the memory here whether we succeed or fail!
		 * @return True is returned on success; false, otherwise.  We can fail here if a route already exists by the given route's name.
		 */
		bool AddRoute(LogRoute* logRoute);

		/**
		 * Unregister a route with the system by the given name.
		 * 
		 * @param[in] logRouteName This is the name of the route to remove.
		 * @return True is returned on successful removeal; false, otherwise.
		 */
		bool RemoveRoute(const std::string& logRouteName);

		/**
		 * Tell the caller if a route is registered under the given name.
		 */
		bool RouteExists(const std::string& logRouteName);

		/**
		 * Unregister/delete all registered routes.
		 */
		void ClearAllRoutes();

		/**
		 * Set filter flags on the route registered under the given name.
		 * 
		 * @param[in] routeName The route with this name has its flags updated.  Pass an empty string here to update flags on all registered routes.
		 * @param[in] filterFlags These are the new flags to replace the old ones on the desired route.
		 */
		void SetRouteFilter(const std::string& routeName, uint32_t filterFlags);

		/**
		 * Get the filter flags on the route registered under the given name.
		 * 
		 * @param[in] routeName The filter flags of the registered route with this name is returned.
		 * @return The said filter flags are returned here; zero, if the route doesn't exists.
		 */
		uint32_t GetRouteFilter(const std::string& routeName);

		/**
		 * Get a pointer to the logging system singleton.
		 */
		static LoggingSystem* Get();

	private:
		std::mutex mutex;
		typedef std::unordered_map<std::string, Reference<LogRoute>> LogRouteMap;
		LogRouteMap logRouteMap;
	};

	/**
	 * These are recepticals of logging messages.
	 */
	class IMZADI_API LogRoute : public ReferenceCounted
	{
	public:
		LogRoute();
		virtual ~LogRoute();

		/**
		 * A derived class must override this to consume a log message.
		 * Note that while no two calls to this method will ever happen
		 * simultaneously on seperate threads, this call itself must
		 * be thread-safe.  If in doubt, enqueue the given message on
		 * a thread-safe queue, then consume that queue on your own thread.
		 * 
		 * @param[in] messageFlags These are the same flags that were sent to the logging system before being routed here.  Use them however you like (e.g., to color-code the message.)
		 * @param[in] logMessage This is the message to be routed by the router.
		 */
		virtual void PrintLogMessage(uint32_t messageFlags, const std::string& logMessage) = 0;

		/**
		 * Override this to do something on registration of the route with the system.
		 */
		virtual void RouteRegistered();

		/**
		 * Override this to do something on unregistration of the route with the system.
		 */
		virtual void RouteUnregistered();

		const std::string& GetName() const { return this->name; }
		void SetName(const std::string& name) { this->name = name; }

		uint32_t GetFilterFlags() const { return this->filterFlags; }
		void SetFilterFlags(uint32_t flags) { this->filterFlags = flags; }

	protected:
		std::string name;
		uint32_t filterFlags;
	};

	/**
	 * Provide a way to route log messages to disk.  Messages
	 * get flushed immediately so that the log is complete up
	 * to a program crash.
	 */
	class IMZADI_API LogFileRoute : public LogRoute
	{
	public:
		LogFileRoute();
		virtual ~LogFileRoute();

		virtual void PrintLogMessage(uint32_t messageFlags, const std::string& logMessage) override;
		virtual void RouteRegistered() override;
		virtual void RouteUnregistered() override;

		const std::filesystem::path& GetLogFilePath() const { return this->logFilePath; }
		void SetLogFilePath(const std::filesystem::path& path) { this->logFilePath = path; }

	protected:
		std::filesystem::path logFilePath;
		std::fstream fileStream;
	};

	/**
	 * Provide a way to route log messages as output to the stdout or stderr.
	 */
	class IMZADI_API LogConsoleRoute : public LogRoute
	{
	public:
		LogConsoleRoute();
		virtual ~LogConsoleRoute();

		virtual void PrintLogMessage(uint32_t messageFlags, const std::string& logMessage) override;
	};
}