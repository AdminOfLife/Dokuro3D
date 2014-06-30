#ifndef FASTBITMAP_DEF
#define FASTBITMAP_DEF

#include <Renderer/Color.hpp>
#include <string>
#include <Windows.h>

class FastBitmap {
public:
	FastBitmap();
	FastBitmap(int _width, int _height, HDC _hdc);
	~FastBitmap();

	/* Resize the bitmap */
	void Resize(int _width, int _height, HDC _hdc);

	/* Get the size of the bitmap */
	int GetWidth();
	int GetHeight();

	/* Structure to store a pixel */
	struct Pixel {
		Pixel() {
			b = 0;
			g = 0;
			r = 0;
			a = 0;
		}

		unsigned char b, g, r, a;
	};

	/* Get the pixels */
	Pixel* GetPixels();

	/* Modify the bitmap */
	void Clear(dkr::Color _color, HDC _hdc);
	void Clear(unsigned int _color, HDC _hdc);
	void SetPixel(int _x, int _y, dkr::Color _color);

	/* Draw text to the bitmap */
	void PrintText(HDC _hdc, const std::string& _text, int _x, int _y, dkr::Color _color);

	/* Blitting to another hdc */
	void Blit(HDC _hdc);

private:
	/* Can't copy this */
	FastBitmap& operator=(const FastBitmap& _bitmap) {}
	FastBitmap(const FastBitmap& _bitmap) {}

	/* The pixel data for the bitmap */
	Pixel* pixels;

	/* The handle to the bitmap */
	HBITMAP bitmap;

	/* The width and height of the bitmap */
	int width, height;

	/* The font */
	HFONT font;
};

#endif