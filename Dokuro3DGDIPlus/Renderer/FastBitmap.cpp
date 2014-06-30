#include "FastBitmap.hpp"
#include <Exceptions/DokuroException.hpp>
using namespace dkr;

FastBitmap::FastBitmap() {
	bitmap = NULL;
	width = 0;
	height = 0;

	// Get a hdc
	HDC hdc = GetDC(0);
	
	// Calculate the size.
	long size = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);

	// Release that dc
	ReleaseDC(0, hdc);

	// Create the font
	font = CreateFont(size, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Arial");
}

FastBitmap::FastBitmap(int _width, int _height, HDC _hdc) {
	bitmap = NULL;
	Resize(_width, _height, _hdc);
}

FastBitmap::~FastBitmap() {
	/* Delete the bitmap */
	DeleteObject(bitmap);

	/* Delete the font */
	DeleteObject(font);
}

/* Resize */
void FastBitmap::Resize(int _width, int _height, HDC _hdc) {
	// Make sure negative size bitmaps aren't created.
	if (_width <= 0 || _height <= 0)
		return;

	/* Create the fast bitmap */
	BITMAPINFO info;
	info.bmiHeader.biSize = sizeof(BITMAPINFO);
	info.bmiHeader.biWidth = _width;
	info.bmiHeader.biHeight =  -_height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	info.bmiHeader.biSizeImage = 0;
	info.bmiHeader.biXPelsPerMeter = 0;
	info.bmiHeader.biYPelsPerMeter = 0;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
	info.bmiColors[0].rgbBlue = 0;
	info.bmiColors[0].rgbGreen = 0;
	info.bmiColors[0].rgbRed = 0;
	info.bmiColors[0].rgbReserved = 0;

	/* Create the bitmap using CreateDIBSection to give us access to the raw pixels */
	HBITMAP newBitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
	if (newBitmap == NULL)
		throw dkr::DokuroException("Couldn't create fast bitmap!");

	/* Blit the old bitmap onto this new bitmap */
	/* Create a memory dc to draw with */
	HDC hdc = CreateCompatibleDC(_hdc);
	HDC oldHdc = CreateCompatibleDC(_hdc);

	/* Select our bitmap */
	HBITMAP old = (HBITMAP)SelectObject(hdc, newBitmap);
	HBITMAP oldOld = (HBITMAP)SelectObject(oldHdc, bitmap);

	/* Blit to hdc */
	BitBlt(hdc, 0, 0, width, height, oldHdc, 0, 0, SRCCOPY);

	/* Select the old bitmap again */
	SelectObject(hdc, old);
	DeleteDC(hdc);
	SelectObject(oldHdc, oldOld);
	DeleteDC(oldHdc);

	/* Delete the bitmap */
	DeleteObject(bitmap);
	
	/* Set the new bitmap */
	bitmap = newBitmap;

	width = _width;
	height = _height;
}

/* Get the size of the bitmap */
int FastBitmap::GetWidth() {
	return width;
}

int FastBitmap::GetHeight() {
	return height;
}

/* Clear the bitmap */
void FastBitmap::Clear(dkr::Color _color, HDC _hdc) {
	/*int size = width * height;
	for (int i = 0; i < size; i++) {
		pixels[i].r = _color.colors[0];
		pixels[i].g = _color.colors[1];
		pixels[i].b = _color.colors[2];
		pixels[i].a = _color.colors[3];
	}*/

	/* Create a memory dc to draw with */
	HDC hdc = CreateCompatibleDC(_hdc);

	/* Select our bitmap */
	HBITMAP old = (HBITMAP)SelectObject(hdc, bitmap);

	/* Fill rect */
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = height;
	rect.right = width;
	HBRUSH brush = CreateSolidBrush(RGB(_color.colors[0], _color.colors[1], _color.colors[2]));
	FillRect(hdc, &rect, brush);
	DeleteObject(brush);

	/* Select the old bitmap again */
	SelectObject(hdc, old);
	DeleteDC(hdc);

	//Pixel pixel;
	//pixel.r = _color.colors[0];
	//pixel.g = _color.colors[1];
	//pixel.b = _color.colors[2];
	//std::fill_n(pixels, height * width, pixel);
}

/* Clear to an unsigned int */
void FastBitmap::Clear(unsigned int _color, HDC _hdc) {
	/* Create a memory dc to draw with */
	HDC hdc = CreateCompatibleDC(_hdc);

	/* Select our bitmap */
	HBITMAP old = (HBITMAP)SelectObject(hdc, bitmap);

	/* Fill rect */
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.bottom = height;
	rect.right = width;
	HBRUSH brush = CreateSolidBrush(_color);
	FillRect(hdc, &rect, brush);
	DeleteObject(brush);

	/* Select the old bitmap again */
	SelectObject(hdc, old);
	DeleteDC(hdc);
}

/* Get the pixel data */
FastBitmap::Pixel* FastBitmap::GetPixels() {
	return pixels;
}

/* Draw a pixel */
void FastBitmap::SetPixel(int _x, int _y, dkr::Color _color) {
	if (_x >= width || _x < 0 || _y >= height || _y < 0)
		return;

	int i = _y * width + _x;
	pixels[i].r = _color.colors[0];
	pixels[i].g = _color.colors[1];
	pixels[i].b = _color.colors[2];
	pixels[i].a = _color.colors[3];
}

/* Draw text to the screen */
void FastBitmap::PrintText(HDC _hdc, const std::string& _text, int _x, int _y, dkr::Color _color) {
	/* Create a memory dc to draw with */
	HDC hdc = CreateCompatibleDC(_hdc);

	/* Select our bitmap */
	HBITMAP old = (HBITMAP)SelectObject(hdc, bitmap);

	// Select our font
	HFONT oldFont = (HFONT)SelectObject(hdc, font);
	SetBkMode(hdc, TRANSPARENT); // Transparent background.
	SetTextColor(hdc, RGB(_color.colors[0], _color.colors[1], _color.colors[2]));

	RECT rect;
	rect.left = _x;
	rect.top = _y;
	DrawTextA(hdc, _text.c_str(), -1, &rect, DT_NOCLIP);

	// Reselect the font
	SelectObject(hdc, oldFont);

	/* Select the old bitmap again */
	SelectObject(hdc, old);
	DeleteDC(hdc);
}

/* Blit the bitmap to a hdc */
void FastBitmap::Blit(HDC _hdc) {
	/* Create a memory dc to draw with */
	HDC hdc = CreateCompatibleDC(_hdc);

	/* Select our bitmap */
	HBITMAP old = (HBITMAP)SelectObject(hdc, bitmap);

	/* Blit to hdc */
	BitBlt(_hdc, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

	/* Select the old bitmap again */
	SelectObject(hdc, old);
	DeleteDC(hdc);
}