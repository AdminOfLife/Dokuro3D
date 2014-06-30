#include "WindowsRenderer.hpp"
using namespace dkr;

WindowsRenderer::WindowsRenderer(HWND _hWnd) {
	InitGDI();
	hWnd = _hWnd;
	bufferMutex = ThreadFactory::GetMutex();

	/* Create a device context */
	hdc = GetDC(hWnd);
}

WindowsRenderer::~WindowsRenderer() {
	// Delete the mutex
	delete bufferMutex;

	// Stop painting
	ReleaseDC(hWnd, hdc);
}

void WindowsRenderer::InitGDI() {
}

/* Create the screen (resizes) */
void WindowsRenderer::Create(int _width, int _height, int _depth) {
	/* Resize the back buffer */
	buffer.Resize(_width, _height, hdc);
	zbuffer.Resize(_width, _height, hdc);

	width = _width;
	height = _height;
	depth = _depth;
}

void WindowsRenderer::Clear(Color _color) {
	/* Clear buffer */
	buffer.Clear(_color, hdc);

	/* Clear the zbuffer */
	float convert = 0.0f;
	zbuffer.Clear(*(unsigned int*)&convert, hdc);
}

void WindowsRenderer::DrawPixel(int _x, int _y, Color _color) {
	// Draw a single pixel
	buffer.SetPixel(_x, _y, _color);
}

void* WindowsRenderer::GetPixels() {
	return (void*)buffer.GetPixels();
}

WindowsRenderer::Pixel WindowsRenderer::ConvertColorToPixel(dkr::Color _color) {
	WindowsRenderer::Pixel pixel;
	pixel.r = _color.colors[0];
	pixel.g = _color.colors[1];
	pixel.b = _color.colors[2];
	pixel.a = _color.colors[3];

	return pixel;
}

dkr::Color WindowsRenderer::ConvertPixelToColor(WindowsRenderer::Pixel _pixel) {
	dkr::Color color((char)_pixel.r, _pixel.g, _pixel.b, _pixel.a);
	return color;
}

void WindowsRenderer::PrintText(const std::string& _text, int _x, int _y, dkr::Color _color) {
	buffer.PrintText(hdc, _text, _x, _y, _color);
}

float* WindowsRenderer::GetZBuffer() {
	return (float*)zbuffer.GetPixels();
}

void WindowsRenderer::Flip() {
	/* Blit to the hdc */
	buffer.Blit(hdc);
}