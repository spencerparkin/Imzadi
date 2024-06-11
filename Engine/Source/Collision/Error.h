#pragma once

#include "Defines.h"
#include <string>
#include <vector>
#include <mutex>

namespace Collision
{
	/**
	 * An instance of this class contains all error information provided by the collision library.
	 * Whenever an API call fails, error information can be retrieved from this class.  See the
	 * GetError function.  This class is used to both retrieve and report error information.
	 */
	class COLLISION_LIB_API Error
	{
	public:
		Error();
		virtual ~Error();

		/**
		 * Remove all error messages that have been accumulated.  This is a thread-safe call.
		 */
		void Clear();

		/**
		 * Accumulate an error message.  This is typically done by the collision system
		 * internals as the call-stack is popped.  This is a thread-safe call.
		 * 
		 * @param[in] errorMessage This message will be added to the list of such.
		 */
		void AddErrorMessage(const std::string& errorMessage);

		/**
		 * Tell the caller how many error messages have accumulated.  A typical API call
		 * that can fail will return a boolean indicating success or failure.  Only when
		 * failure is returned from such a call does the caller then need to consider
		 * looking at error information.  This call is thread-safe.
		 * 
		 * @return The number of error messages is returned.  If it's zero, then the caller knows that no error has occurred.
		 */
		int GetCount() const;

		/**
		 * Return all accumulated error messages as a single, formatted message.
		 * This is a thread-safe call.
		 * 
		 * @return The formatted error message is returned.
		 */
		std::string GetAllErrorMessages() const;

	private:
		std::vector<std::string>* errorMessageArray;
		std::mutex* mutex;
	};

	/**
	 * Whenever an API call fails, this function can be used to get the error object which,
	 * in turn, can be used to get information about why the call failed.  Note that errors
	 * accumulate.  It is up to the user of the collision library to clear errors, if any,
	 * in preparation for other calls.
	 */
	COLLISION_LIB_API Error* GetError();
}