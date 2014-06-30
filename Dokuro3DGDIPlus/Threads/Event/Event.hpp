#ifndef EVENT_DEF
#define EVENT_DEF

#include <Threads/Event/IEvent.hpp>
#include "../Thread.hpp"
#include <Windows.h>

class Event : public dkr::IEvent {
public:
	Event();
	virtual ~Event();

	// Wait for the event to be signaled.
	virtual void Wait();

	// Signal the event.
	virtual void Signal();
		
	// Reset the event.
	virtual void Reset();

	friend class Thread;
private:
	HANDLE event;
};


#endif