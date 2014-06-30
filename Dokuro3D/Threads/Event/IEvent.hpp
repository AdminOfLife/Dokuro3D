#ifndef IEVENT_DEF
#define IEVENT_DEF

namespace dkr {
	class IEvent {
	public:
		IEvent();
		virtual ~IEvent();

		// Release and wait.
		virtual void Wait() = 0;
		virtual void Signal() = 0; // Signal the event.
		virtual void Reset() = 0; // Reset the event ("Unsignals" the event)

	private:
	};
}

#endif