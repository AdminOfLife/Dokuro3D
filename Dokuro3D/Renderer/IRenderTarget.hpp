#ifndef IRenderTarget_DEF
#define IRenderTarget_DEF

#include "Color.hpp"
#include "../Maths/Matrix4.hpp"
#include "../GraphicsMemory/GraphicsMemory.hpp"
#include "../Lights/LightInfo.hpp"

namespace dkr {
	class PixelShader;

	/* An interface for platforms to implement. All it needs to be able to do is to draw pixels */
	class IRenderTarget {
	public:
		/* Default constructor */
		IRenderTarget();
		virtual ~IRenderTarget();

		/* Create the screen */
		virtual void Create(int _width, int _height, int _depth) = 0;

		/* Set the graphics memory */
		virtual void SetGraphicsMemory(GraphicsMemory* _memory);

		/* Clear the screen */
		virtual void Clear(Color _color) = 0;

		/* Get the zbuffer data */
		virtual float* GetZBuffer() = 0;

		/* Draw a pixel onto the screen */
		virtual void DrawPixel(int _x, int _y, Color _color) = 0;
		virtual void* GetPixels() = 0;

		/* A Pixel */
		struct Pixel {
			Pixel() {
				b = 0;
				g = 0;
				r = 0;
				a = 0;
			}
			Pixel(int _r, int _g, int _b, int _a) {
				b = _b;
				g = _g;
				r = _r;				
				a = _a;
			}

			unsigned char b, g, r, a;
		};

		virtual Pixel ConvertColorToPixel(dkr::Color _color) = 0;
		virtual Color ConvertPixelToColor(Pixel _pixel) = 0;

		/* Draw a line onto the screen */
		virtual void DrawLine(int _x1, int _y1, int _x2, int _y2, Color _color);
		virtual void DrawLines(int _bufferId);

		/* Draw a triangle onto the screen */
		virtual void DrawSolidTriangle(const DrawBuffer::Triangle& _triangle);
		virtual void DrawInterpolatedTriangle(DrawBuffer::Triangle& _triangle);
		virtual void DrawInterpolatedTrianglePhong(DrawBuffer::Triangle& _triangle);
		virtual void DrawInterpolatedTrianglePhongTextured(DrawBuffer::Triangle& _triangle);
		virtual void DrawInterpolatedTriangleTextured(DrawBuffer::Triangle& _triangle);
		virtual void DrawInterpolatedTriangleTexturedLighted(DrawBuffer::Triangle& _triangle);
		virtual void DrawInterpolatedTrianglePhongNormalMap(DrawBuffer::Triangle& _triangle);
		virtual void DrawSolidTriangles(int _bufferId);

		/* Draw a triangle using the scanlines algorithm */
		virtual void DrawInterpolatedTriangleScanLines(DrawBuffer::Triangle& _triangle);

		/* Draw text to the screen */
		virtual void PrintText(const std::string& _text, int _x, int _y, dkr::Color _color);

		/* Flip the double buffer (if there is one) */
		virtual void Flip() = 0;

		/* Get the width, height and depth */
		int GetWidth();
		int GetHeight();
		int GetDepth();
	
	protected:
		int width;
		int height;
		int depth;
			
	private:
		/* The graphics memory object */
		GraphicsMemory* memory;

		/* The zbuffer */
		float* zbuffer;

		/* This class can't be copied */
		IRenderTarget(const IRenderTarget& _renderer) {};
		IRenderTarget& operator= (const IRenderTarget& _renderer) {};
	};
}

#endif