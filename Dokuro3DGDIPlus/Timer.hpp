#ifndef TIMER_DEF
#define TIMER_DEF

#include <Windows.h>

class Timer {
public:
	// Create a new timer.
	Timer();

	// Query the timer and update the time.
	void Tick();

	// Get how much time has passed in seconds.
	double GetTime();

private:
	LARGE_INTEGER frequency;
	LARGE_INTEGER lastTime;
	LARGE_INTEGER thisTime;
};

#endif