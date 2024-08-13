#include "Clock.h"
#include <Windows.h>
#include <sysinfoapi.h>

using namespace Imzadi;

Clock::Clock()
{
	this->timeBase = 0;
}

/*virtual*/ Clock::~Clock()
{
}

void Clock::Reset()
{
	this->timeBase = this->GetCurrentSystemTime();
}

bool Clock::NeverBeenReset() const
{
	return this->timeBase == 0;
}

uint64_t Clock::GetCurrentSystemTime() const
{
	FILETIME fileTime{};
	::GetSystemTimePreciseAsFileTime(&fileTime);

	ULARGE_INTEGER largeInteger{};
	largeInteger.LowPart = fileTime.dwLowDateTime;
	largeInteger.HighPart = fileTime.dwHighDateTime;

	return largeInteger.QuadPart;
}

uint64_t Clock::GetElapsedTime() const
{
	uint64_t timeCurrent = this->GetCurrentSystemTime();
	uint64_t timeElapsed = timeCurrent - this->timeBase;
	return timeElapsed;
}

double Clock::GetCurrentTimeMilliseconds() const
{
	long double oneHundredNanoSecondTicks = (long double)this->GetElapsedTime();
	long double milliseconds = oneHundredNanoSecondTicks / 10000.0;
	return double(milliseconds);
}

double Clock::GetCurrentTimeSeconds() const
{
	long double oneHundredNanoSecondTicks = (long double)this->GetElapsedTime();
	long double seconds = oneHundredNanoSecondTicks / 10000000.0;
	return double(seconds);
}