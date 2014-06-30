#ifndef IMUTEX_DEF
#define IMUTEX_DEF

namespace dkr {

	class IMutex {
	public:
		/* Virtual destructor */
		virtual ~IMutex() {}

		/* Unlock and lock the mutex */
		virtual void Lock() = 0;
		virtual void Unlock() = 0;

	private:
	};

}

#endif