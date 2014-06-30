#ifndef GAME_DEF
#define GAME_DEF

#include <Threads/IRunnable.hpp>
#include <Windows.h>

#include "../Renderer/WindowsRenderer.hpp"
#include <Threads/ThreadFactory.hpp>
#include "../Threads/Thread.hpp"
#include <Transformer/Transformer.hpp>

class Game : public dkr::IRunnable {
public:
	Game(HWND _hWnd, int _width, int _height, int _depth);
	virtual ~Game();

	/* Resize the game */
	virtual void Resize();

	/* Run the game */
	virtual int Run();
protected:
	/* Whether or not the game is running */
	bool running;

	/* The renderer */
	WindowsRenderer renderer;

	/* Graphics memory */	
	dkr::GraphicsMemory memory;

	/* The transformer */
	dkr::Transformer transformer;

	/* The window handle */
	HWND hWnd;

	/* The thread object */
	dkr::IThread* thread;
};

#endif