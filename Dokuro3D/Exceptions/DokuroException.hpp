#ifndef DOKUROEXCEPTION_DEF
#define DOKUROEXCEPTION_DEF

#include <string>
#include <exception>
#include <cstdarg>

namespace dkr {
	/* Base exception class used by all exceptions in Dokuro3D */
	class DokuroException : public std::exception {
	public:
		DokuroException();
		DokuroException(const std::string& _msg);
		virtual ~DokuroException();

		/* Override this method and return the message for the exception */
		virtual const char* what() const throw();

		/* Get the message */
		virtual const std::string& GetMsg();

	protected:
		/* The message */
		std::string msg;
	};
}

#endif