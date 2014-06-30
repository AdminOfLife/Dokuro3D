#ifndef WINDOWSRENDERER_DEF
#define WINDOWSRENDERER_DEF

#include <Renderer/IRenderTarget.hpp>

#include <Threads/Mutexes/IMutex.hpp>
#include <Threads/ThreadFactory.hpp>
#include <GraphicsMemory/VertexBuffer.hpp>

#include "FastBitmap.hpp"

#include <SDKDDKVer.h>
#include <windows.h>
#include <GdiPlus.h>

/* Windows api implementation of the IRenderTarget */
class WindowsRenderer : public dkr::IRenderTarget {
public:
	WindowsRenderer();
	WindowsRenderer(HWND _hWnd);
	virtual ~WindowsRenderer();

	/* Create the screen */
	virtual void Create(int _width, int _height, int _depth);

	/* Clear the screen */
	virtual void Clear(dkr::Color _color);

	/* Draws a pixel */
	virtual void DrawPixel(int _x, int _y, dkr::Color _color);
	virtual void* GetPixels();
	virtual Pixel ConvertColorToPixel(dkr::Color _color);
	virtual dkr::Color ConvertPixelToColor(Pixel _pixel);

	/* Draws text */
	virtual void PrintText(const std::string& _text, int _x, int _y, dkr::Color _color);

	/* Get the zbuffer */
	virtual float* GetZBuffer();

	/* Flip the back buffer */
	virtual void Flip();

protected:
	/* Intialize GDI */
	virtual void InitGDI();

private:
	/* The handle to the window */
	HWND hWnd;

	/* The hdc */
	HDC hdc;

	/* A paint struct */ 
	PAINTSTRUCT ps;

	/* GDI token to shut it down later */
	ULONG_PTR gdiToken;

	/* The back buffer */
	FastBitmap buffer;
	BITMAP bufferInfo;

	/* The zbuffer */
	FastBitmap zbuffer;

	/* The mutex to lock the buffer with */	
	dkr::IMutex* bufferMutex;
};

#endif