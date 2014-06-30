#include "DokuroException.hpp"
using namespace dkr;

#include <cstdio>

DokuroException::DokuroException() {
}

DokuroException::DokuroException(const std::string& _msg) {
	msg = _msg;
}

DokuroException::~DokuroException() {
}

/* Return the message using the standard exception function... */
const char* DokuroException::what() const throw() {
	return msg.c_str();
}

/* Return the message as a string */
const std::string& DokuroException::GetMsg() {
	return msg;
}