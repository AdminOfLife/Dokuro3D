#include "Event.hpp"

Event::Event() {
	// Create an event.
	event = CreateEvent(NULL, true, false, NULL);
}

Event::~Event() {
	// Close the event.
	CloseHandle(event);
}


// Wait for the event to be signaled.
void Event::Wait() {
	WaitForSingleObject(event, INFINITE);
}

// Signal the event.
void Event::Signal() {
	SetEvent(event);
}
		
// Reset the event.
void Event::Reset() {
	ResetEvent(event);
}
