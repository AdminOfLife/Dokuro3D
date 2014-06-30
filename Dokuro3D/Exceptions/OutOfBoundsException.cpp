#include "OutOfBoundsException.hpp"
using namespace dkr;

OutOfBoundsException::OutOfBoundsException() : 
	DokuroException("Out Of Bounds Exception!") {
}

OutOfBoundsException::OutOfBoundsException(const std::string& _msg)
	: DokuroException(_msg) {
	msg = "Out Of Bounds Exception: " + msg;
}

OutOfBoundsException::~OutOfBoundsException() {
}