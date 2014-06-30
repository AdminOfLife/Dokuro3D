#ifndef IRUNNABLE_DEF
#define IRUNNABLE_DEF

namespace dkr {
	class IRunnable {
	public:
		IRunnable() {}
		virtual ~IRunnable() {}

		/* Run when the thread is run */
		virtual int Run() = 0;

	private:
		IRunnable(const IRunnable& _runnable) {}
		IRunnable& operator= (const IRunnable& _runnable) {}
	};
}

#endif