#include "Thread.hpp"
#include "Event/Event.hpp"
using namespace dkr;

Thread::Thread() {
	thread = NULL;
}

Thread::Thread(IRunnable* _runnable)
	: IThread(_runnable) {
	thread = NULL;
}

Thread::~Thread() {
	Stop();
}

void Thread::Start() {
	IThread::Start();

	/* Create the thread */
	if (thread)
		return;
		
	/* This container will cast back into container after being casted to void* */
	ThisContainer* container = new ThisContainer();
	container->thread = this;

	/* This is a hacky way of making sure that the pointer we pass through can be reconverted back to Thread from void* */
	thread = CreateThread(NULL, 
						  0, 
						  (unsigned long (__stdcall*)(void*))this->RunThread,
						  (void*)container,
						  0,
						  NULL);
	if (thread == NULL)
		throw DokuroException("Couldn't create win32 thread!");
							  
}

void Thread::Stop() {
	IThread::Stop();

	/* Stop the thread */
	if (thread != NULL) {
		/* Forcefully kill the thread */
		TerminateThread(thread, 0);
		CloseHandle(thread);
	}
}

void Thread::Pause() {
}

void Thread::Resume() {
}

/* Wait for this thread to end */
void Thread::Wait() {
	// Wait for this thread to finish.
	WaitForSingleObject(thread, INFINITE);
}
void Thread::WaitForThreads(dkr::IThread** _threads, int _size) {
	// Wait for multiple threads to finish.
	// First extract the handles from the threads
	HANDLE* threads = new HANDLE[_size];
	for (int i = 0; i < _size; ++i) {
		threads[i] = dynamic_cast<Thread*>(_threads[i])->thread;
	}

	// Now wait...
	WaitForMultipleObjects(_size, threads, TRUE, INFINITE);

	// Deallocate the memory we allocate before
	delete [] threads;
}

void Thread::WaitForEvents(dkr::IEvent** _events, int _size) {
	// Wait for multiple threads to finish.
	// First extract the handles from the threads
	HANDLE* events = new HANDLE[_size];
	for (int i = 0; i < _size; ++i) {
		events[i] = dynamic_cast<Event*>(_events[i])->event;
	}

	// Now wait...
	WaitForMultipleObjects(_size, events, TRUE, INFINITE);

	// Deallocate the memory we allocate before
	delete [] events;
}

/* Run the thread */
int Thread::RunThread(void* _this) {
	/* Get the this pointer out of the container and use it as the this pointer */
	ThisContainer* container = (ThisContainer*)_this;
	IThread* t = (container->thread);
	
	/* Delete the container */
	delete container;

	/* Call the run function */
	return t->Run();
}