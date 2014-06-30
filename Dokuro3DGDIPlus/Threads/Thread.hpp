#ifndef THREAD_DEF
#define THREAD_DEF

#include <Threads/IThread.hpp>
#include <Threads/IRunnable.hpp>
#include <Threads/Event/IEvent.hpp>

#include <Windows.h>

class Thread : public dkr::IThread {
public:
	Thread();
	Thread(dkr::IRunnable* _runnable);
	virtual ~Thread();

	/* Starting and stopping the thread */
	virtual void Start();
	virtual void Stop();

	/* Pausing and resuming threads */
	virtual void Pause();
	virtual void Resume();

	/* Wait for this thread to end */
	virtual void Wait();
	virtual void WaitForThreads(dkr::IThread** _threads, int _size);
	virtual void WaitForEvents(dkr::IEvent** _events, int _size);

private:
	/* This function will pass the callback function onto the correct thread instance */
	static int RunThread(void* _this);

	/* The thread handle */
	HANDLE thread;

	/* A work around struct to allow for inheritance of this class */
	struct ThisContainer {
		IThread* thread;
	};
};

#endif