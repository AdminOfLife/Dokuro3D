#ifndef THREADFACTORY_DEF
#define THREADFACTORY_DEF

#include "IRunnable.hpp"
#include "IThread.hpp"
#include "Mutexes/IMutex.hpp"
#include "Event/IEvent.hpp"

#include "../Exceptions/FactoryProductionException.hpp"

namespace dkr {
	class ThreadFactory {
	public:
		/* Get's a mutex from the thread factory */
		static IMutex* GetMutex();

		/* Get's a thread from the thread factory */
		static IThread* GetThread(IRunnable* _runnable);

		/* Get's an event from the thread factory */
		static IEvent* GetEvent();

		/* Set's the function to use to get the thread */
		static void SetThreadFunction(IThread* (*_function)(IRunnable*));

		/* Set's the function to get mutexes from */
		static void SetMutexFunction(IMutex* (*_function)());

		/* Set's the function to get events from */
		static void SetEventFunction(IEvent* (*_function)());

	private:

		/* The factory function */
		static IThread* (*threadFunction)(IRunnable*);
		static IMutex* (*mutexFunction)();
		static IEvent* (*eventFunction)();

		/* Functions we shouldn't call... */
		ThreadFactory() {}
		ThreadFactory(const ThreadFactory& _factory) {}
		ThreadFactory& operator= (const ThreadFactory& _factory) {}
	};
}

#endif