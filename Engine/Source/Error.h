#pragma once

#include "Defines.h"
#include <map>
#include <mutex>

// We could no-op this in a release build, but I'm hesitant to do that.
#define IMZADI_ERROR(errorMsg)		Error::Get()->SendErrorMessage(errorMsg)

namespace Imzadi
{
	class ErrorCapture;

	/**
	 * A singleton instance of this class is used for reporting errors on any thread.
	 * The Imzadi Game Engine tries to continue on no matter the error, but will report
	 * errors if they're encountered.  By default, the engine is silent on errors as
	 * there is no default error capture registered.  It is up to the user to register
	 * such a capture if they wish to see errors.
	 */
	class IMZADI_API Error
	{
	public:
		Error();
		virtual ~Error();

		/**
		 * Issue an error message.  This is a thread-safe call.
		 *
		 * @param[in] errorMessage This should be a concise and reasonably descriptive/useful message about what went wrong.
		 */
		void SendErrorMessage(const std::string& errorMessage);

		/**
		 * Register an instance of the ErrorCapture class with the error reporting system.
		 * This instance's @ref Error method is called to report an error.
		 *
		 * @param[in] key The ErrorCapture instance is registered under this name.
		 * @param[in] errorCapture This instance receives errors messages.  We do not take ownership of the memory.
		 */
		void RegisterErrorCapture(const std::string& key, ErrorCapture* errorCapture);

		/**
		 * Unregister what ever instance of the ErrorCapture class is registered, if any,
		 * under the given name.
		 *
		 * @param[in] key This is the name underwhich registration is considered.
		 */
		void UnregisterErrorCapture(const std::string& key);

		/**
		 * Get at the error singleton instance to report an error or register an error capture.
		 */
		static Error* Get();

	private:
		std::mutex mutex;
		std::map<std::string, ErrorCapture*> errorCaptureMap;
	};

	/**
	 * This class forms the basis for Imzadi's error reporting system.
	 * Rather than throw exceptions or try to return error messages from
	 * API calls, etc., whenever an error is encountered, it is reported
	 * to one of these, if any is registered, and then the engine tries
	 * to continue or halts if necessary.
	 */
	class IMZADI_API ErrorCapture
	{
	public:
		ErrorCapture();
		virtual ~ErrorCapture();

		/**
		 * This method is called when the engine wants to report an error.
		 * Note that it can be called from any thread!  This means that any
		 * implimentation of this method needs to be thread-safe.
		 *
		 * This method also provides a way to trap errors at the place where
		 * they occur within the engine.
		 *
		 * @param[in] errorMessage This should be a detailed message about what went wrong.
		 */
		virtual void Error(const std::string& errorMessage) = 0;
	};
}