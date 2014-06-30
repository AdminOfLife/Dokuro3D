#ifndef DEMO_DEF
#define DEMO_DEF

#include "../Game/Game.hpp"

class Demo {
public:
	Demo();
	virtual ~Demo();

	// Initialization and unloading
	virtual void Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) = 0;
	virtual void Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) = 0;

	// Update and draw the demo.
	virtual void Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) = 0;
	virtual void Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) = 0;

	// Get the how long it takes to expire the demo
	virtual float Expires() = 0;

private:
};

#endif