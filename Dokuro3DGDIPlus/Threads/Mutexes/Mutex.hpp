#ifndef MUTEX_DEF
#define MUTEX_DEF

#include <Threads\Mutexes\IMutex.hpp>
#include <Windows.h>

class Mutex : public dkr::IMutex {
public:
	/* Construct the mutex */
	Mutex();
	virtual ~Mutex();

	/* Lock and unlock */
	virtual void Lock();
	virtual void Unlock();

private:
	/* The mutex handle */
	HANDLE mutex;
};

#endif