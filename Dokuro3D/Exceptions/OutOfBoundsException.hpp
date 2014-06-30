#ifndef OUTOFBOUNDSEXCEPTION_DEF
#define OUTOFBOUNDSEXCEPTION_DEF

#include "DokuroException.hpp"

namespace dkr {

	class OutOfBoundsException : public DokuroException {
	public:
		OutOfBoundsException();
		OutOfBoundsException(const std::string& _msg);
		virtual ~OutOfBoundsException();
	};

}

#endif