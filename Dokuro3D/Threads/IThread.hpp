#ifndef ITHREAD_DEF
#define ITHREAD_DEF

#ifndef THREADS_MAX
#define THREADS_MAX 100
#endif

#include <map>
#include <stack>

#include "IRunnable.hpp"
#include "../Exceptions/FactoryProductionException.hpp"

#include "Event/IEvent.hpp"

namespace dkr {
	class IThread {
	public:
		IThread();
		IThread(IRunnable* _runnable);
		virtual ~IThread();
		void Init();

		/* Whether or not the thread is running */
		virtual bool Running();

		/* Run the thread */
		virtual void Start();

		/* Stop the thread */
		virtual void Stop();

		/* Pause the thread */
		virtual void Pause() = 0;

		/* Resume the thread */
		virtual void Resume() = 0;

		/* Wait for this thread to end */
		virtual void Wait() = 0;
		virtual void WaitForThreads(IThread** _threads, int _size) = 0;
		virtual void WaitForEvents(IEvent** _events, int _size) = 0;

		/* The function to run when the thread runs */
		virtual int Run();

	private:
		IThread(const IThread& _thread);
		IThread& operator= (const IThread& _thread);

		/* The runnable instance */
		IRunnable* runnable;

		/* Whether or not the thread is running */
		bool running;
	};
}

#endif