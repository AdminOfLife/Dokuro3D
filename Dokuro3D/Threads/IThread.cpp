#include "IThread.hpp"
using namespace dkr;

#include <iostream>

IThread::IThread() {
	Init();
}

IThread::IThread(IRunnable* _runnable) {
	Init();
	runnable = _runnable;
}

IThread::IThread(const IThread& _thread) {
}

IThread::~IThread() {
}

void IThread::Init() {
	running = false;
	runnable = 0;
}

IThread& IThread::operator= (const IThread& _thread) {
	return *(IThread*)0;
}

bool IThread::Running() {
	return running;
}

void IThread::Start() {
	/* If we are already running, there is no point in doing anything */
	if (running)
		return;

	/* We are now running */
	running = true;
}

void IThread::Stop() {
	/* If we are already stopped then there is no point in doing anything */
	if (!running)
		return;
	/* We have now stopped */
	running = false;
}

int IThread::Run() {
	if (runnable)
		return runnable->Run();

	return 0;
}