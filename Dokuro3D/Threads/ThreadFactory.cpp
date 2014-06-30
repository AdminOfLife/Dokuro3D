#include "ThreadFactory.hpp"
using namespace dkr;

/* The factory function */
IThread* (*ThreadFactory::threadFunction)(IRunnable*) = 0;
IMutex*  (*ThreadFactory::mutexFunction)() = 0;
IEvent* (*ThreadFactory::eventFunction)() = 0;

IMutex* ThreadFactory::GetMutex() {
	if (!mutexFunction) {
		throw FactoryProductionException("ThreadFactory: There is no mutex function set!");
	}

	return mutexFunction();
}

IThread* ThreadFactory::GetThread(IRunnable* _runnable) {
	if (!threadFunction) {
		throw FactoryProductionException("ThreadFactory: There is no factory function set!");
	}

	return threadFunction(_runnable);
}

IEvent* ThreadFactory::GetEvent() {
	if (!eventFunction) {
		throw FactoryProductionException("ThreadFactory: There is no event function set!");
	}

	return eventFunction();
}

void ThreadFactory::SetMutexFunction(IMutex* (*_function)()) {
	mutexFunction = _function;
}

void ThreadFactory::SetThreadFunction(IThread* (*_function)(IRunnable*)) {
	threadFunction = _function;
}

void ThreadFactory::SetEventFunction(IEvent* (*_function)()) {
	eventFunction = _function;
}