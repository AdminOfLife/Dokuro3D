#include "Timer.hpp"

Timer::Timer() {
	// Get the frequency (It can't change when the computer is running)
	QueryPerformanceFrequency(&frequency);

	// Set last and this time
	QueryPerformanceCounter(&thisTime);
	QueryPerformanceCounter(&lastTime);
}

void Timer::Tick() {
	// Query the counter
	lastTime = thisTime;
	QueryPerformanceCounter(&thisTime);
}

double Timer::GetTime() {
	return (thisTime.QuadPart - lastTime.QuadPart) / (double)(frequency.QuadPart);
}