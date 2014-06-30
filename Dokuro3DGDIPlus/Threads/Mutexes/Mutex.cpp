#include "Mutex.hpp"
using namespace dkr;

#include <Exceptions\DokuroException.hpp>

Mutex::Mutex() {
	mutex = NULL;
	mutex = CreateMutex(NULL, false, NULL);
	if (mutex == NULL) {
		throw DokuroException("Mutex couldn't be created!");
	}
}

Mutex::~Mutex() {
	if (mutex != NULL)
		CloseHandle(mutex);
}

void Mutex::Lock() {
	/* Lock this mutex */
	WaitForSingleObject(mutex, INFINITE);
}

void Mutex::Unlock() {
	/* Unlock the mutex */
	ReleaseMutex(mutex);
}