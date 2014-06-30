#include "IRenderTarget.hpp"
using namespace dkr;

#include <algorithm>
#include <malloc.h>
#include <cmath>

// SSE 
#include <xmmintrin.h>
#include <emmintrin.h>
#include "../Maths/SSEVector.hpp"
#include "../Maths/SSEVectorInteger.hpp"

IRenderTarget::IRenderTarget() {
	memory = 0;
}

IRenderTarget::~IRenderTarget() {
}

void IRenderTarget::SetGraphicsMemory(GraphicsMemory* _memory) {
	memory = _memory;
}

/* Override this method to put in your own line drawing algorithm if you want */
void IRenderTarget::DrawLine(int _x1, int _y1, int _x2, int _y2, Color _color) {
	// Bresenhams line algorithm (as shown on wikipedia page)
    int dx = abs(_x2 - _x1);
	int dy = abs(_y2 - _y1);

	int sx = -1;
	int sy = -1;
	if (_x1 < _x2) sx = 1;
	if (_y1 < _y2) sy = 1;

	int err = dx - dy;
	int e2;

	// Get the pixel array
	Pixel* pixels = (Pixel*)GetPixels();

	while (true) {
		if (_y1 >= 0 && _y1 < GetHeight() && _x1 >= 0 && _x1 < GetWidth())
			pixels[_y1 * GetWidth() + _x1] = ConvertColorToPixel(_color);
		else
			break;

		if (_x1 == _x2 && _y1 == _y2) break;
		e2 = 2*err;
		if (e2 > -dy) {
			err = err - dy;
			_x1 = _x1 + sx;
		}
		if (e2 < dx) {
			err = err + dx;
			_y1 = _y1 + sy;
		}
	}
}

/* Draw a vertex buffer in wireframe */
void IRenderTarget::DrawLines(int _bufferId) {
	// Loop through each triangle on screen and draw it.
	for (unsigned int i = 0; i < memory->GetTempBuffer(_bufferId).GetTriangles().size(); ++i) {
		DrawBuffer::Triangle triangle = memory->GetTempBuffer(_bufferId).GetTriangles()[i];
		DrawLine((int)triangle.vertices[0].x, (int)triangle.vertices[0].y,
					(int)triangle.vertices[1].x, (int)triangle.vertices[1].y,
							triangle.faceColor);
		DrawLine((int)triangle.vertices[1].x, (int)triangle.vertices[1].y,
					(int)triangle.vertices[2].x, (int)triangle.vertices[2].y,
							triangle.faceColor);
		DrawLine((int)triangle.vertices[2].x, (int)triangle.vertices[2].y,
					(int)triangle.vertices[0].x, (int)triangle.vertices[0].y,
							triangle.faceColor);
	}
}

/* Draw a triangle */
void IRenderTarget::DrawSolidTriangle(const DrawBuffer::Triangle& _triangle) {        
	// Get the color as a pixel
	Pixel pixel = ConvertColorToPixel(_triangle.faceColor);

	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	// Get the pixels pointer to draw to
	Pixel* pixels = (Pixel*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			Pixel* tempBuffer = pixels;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					for (int blockX = x; blockX < highX; ++blockX) {
						tempBuffer[blockX] = pixel;
					}
					tempBuffer += width;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this blog.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					int currentX1 = currentY1;
					int currentX2 = currentY2;
					int currentX3 = currentY3;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX1 > 0 && currentX2 > 0 && currentX3 > 0) {
							tempBuffer[blockX] = pixel;
						}

						currentX1 -= fixedDeltaY1;
						currentX2 -= fixedDeltaY2;
						currentX3 -= fixedDeltaY3;
					}
					currentY1 += fixedDeltaX1;
					currentY2 += fixedDeltaX2;
					currentY3 += fixedDeltaX3;

					// Increment the buffer
					tempBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
	}
}

void IRenderTarget::DrawInterpolatedTriangle(DrawBuffer::Triangle& _triangle) {
	// Get the color as a pixel
	Pixel pixel = ConvertColorToPixel(_triangle.vertexColors[0]);

	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	/* ------------------------ SETUP INTERPOLANTS ------------------------ */
	// Barycentric delta's
	float alphaDen = (_triangle.vertices[1].y - _triangle.vertices[2].y) * _triangle.vertices[0].x +
					(_triangle.vertices[2].x - _triangle.vertices[1].x) * _triangle.vertices[0].y +
					 ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y));
	float betaDen = (_triangle.vertices[2].y - _triangle.vertices[0].y) * _triangle.vertices[1].x +
					(_triangle.vertices[0].x - _triangle.vertices[2].x) * _triangle.vertices[1].y +
					 ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y));
	float gammaDen = (_triangle.vertices[0].y - _triangle.vertices[1].y) * _triangle.vertices[2].x +
					(_triangle.vertices[1].x - _triangle.vertices[0].x) * _triangle.vertices[2].y +
					 ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y));
	float alphaStartBary = ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y)) / alphaDen;
	float betaStartBary = ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y)) / betaDen;
	float gammaStartBary = ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y)) / gammaDen;

	// X delta's
	float alphaXDelta = (_triangle.vertices[1].y - _triangle.vertices[2].y) / alphaDen;
	float betaXDelta = (_triangle.vertices[2].y - _triangle.vertices[0].y) / betaDen;
	float gammaXDelta = (_triangle.vertices[0].y - _triangle.vertices[1].y) / gammaDen;

	// Y delta's
	float alphaYDelta = (_triangle.vertices[2].x - _triangle.vertices[1].x) / alphaDen;
	float betaYDelta = (_triangle.vertices[0].x - _triangle.vertices[2].x) / betaDen;
	float gammaYDelta = (_triangle.vertices[1].x - _triangle.vertices[0].x) / gammaDen;

	//////////////////////// Color interpolation ///////////////////////////
	// Multiply the color by 255 so that we can convert to integers without using multiplication.
	for (int i = 0; i < 3; ++i) {
		_triangle.vertexColors[i] *= 255;
	}

	Vector4 colorStart = _triangle.vertexColors[0] * alphaStartBary + _triangle.vertexColors[1] * betaStartBary + _triangle.vertexColors[2] * gammaStartBary;
	Vector4 colorXDelta = _triangle.vertexColors[0] * alphaXDelta + _triangle.vertexColors[1] * betaXDelta + _triangle.vertexColors[2] * gammaXDelta;
	Vector4 colorYDelta = _triangle.vertexColors[0] * alphaYDelta + _triangle.vertexColors[1] * betaYDelta + _triangle.vertexColors[2] * gammaYDelta;
	SSEVector colorStartSSE(colorStart.x, colorStart.y, colorStart.z, 0);
	SSEVector colorXDeltaSSE(colorXDelta.x, colorXDelta.y, colorXDelta.z, 0);
	SSEVector colorYDeltaSSE(colorYDelta.x, colorYDelta.y, colorYDelta.z, 0);

	//////////////////////// Z interpolation ///////////////////////////////
	// We can probably pack the z into the same vector as the colors...
	float zStart = (_triangle.vertices[0].z - 1.0f) * alphaStartBary + (_triangle.vertices[1].z - 1.0f) * betaStartBary + (_triangle.vertices[2].z - 1.0f) * gammaStartBary;
	float zXDelta = (_triangle.vertices[0].z - 1.0f) * alphaXDelta + (_triangle.vertices[1].z - 1.0f) * betaXDelta + (_triangle.vertices[2].z - 1.0f) * gammaXDelta;
	float zYDelta = (_triangle.vertices[0].z - 1.0f) * alphaYDelta + (_triangle.vertices[1].z - 1.0f) * betaYDelta + (_triangle.vertices[2].z - 1.0f) * gammaYDelta;
	colorStartSSE.w = zStart;
	colorXDeltaSSE.w = zXDelta;
	colorYDeltaSSE.w = zYDelta;

	// Get the pixels pointer to draw to
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Get the zbuffer pointer to access
	float* zBuffer = GetZBuffer();
	zBuffer += ymin * width;

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// Calculate the start of our interpolants # colorYBary = colorStart + colorXDelta * x + colorYDelta * y; (in other words)
			SSEVector colorYBarySSE = colorStartSSE;
			SSEVector temp = colorXDeltaSSE;
			temp *= (float)x;
			colorYBarySSE += temp;
			temp = colorYDeltaSSE;
			temp *= (float)y;
			colorYBarySSE += temp;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			unsigned int* tempBuffer = pixels;
			float* tempZBuffer = zBuffer;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					// Set the interpolated colors back to the start of a line.
					SSEVector colorXBarySSE = colorYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						
						// Put the pixel if the z coord is bigger than the one in the z buffer
						if (colorXBarySSE.w < tempZBuffer[blockX]) {
							// Convert the color to an integer
							__m128i pixel = _mm_cvtps_epi32(colorXBarySSE.data);

							// Set the pixel
							unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
							tempBuffer[blockX] = tempPixel;

							// Mark this on the z buffer
							tempZBuffer[blockX] = colorXBarySSE.w;
						}

						// Increment the color.
						colorXBarySSE += colorXDeltaSSE;
					}
					tempBuffer += width;
					tempZBuffer += width;

					// Increment the color.
					colorYBarySSE += colorYDeltaSSE;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this block.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					int currentX1 = currentY1;
					int currentX2 = currentY2;
					int currentX3 = currentY3;

					// Set the interpolated colors back to the start of a line.
					SSEVector colorXBarySSE = colorYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX1 > 0 && currentX2 > 0 && currentX3 > 0) {
							// Put the pixel if the z coord is bigger than the one in the z buffer
							if (colorXBarySSE.w < tempZBuffer[blockX]) {
								// Convert the color to an integer
								__m128i pixel = _mm_cvtps_epi32(colorXBarySSE.data);

								// Set the pixel
								unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
								tempBuffer[blockX] = tempPixel;

								// Mark this on the z buffer
								tempZBuffer[blockX] = colorXBarySSE.w;
							}
						}

						// Increment the color.
						colorXBarySSE += colorXDeltaSSE;

						// Increment the half space
						currentX1 -= fixedDeltaY1;
						currentX2 -= fixedDeltaY2;
						currentX3 -= fixedDeltaY3;
					}
					// Increment the half space
					currentY1 += fixedDeltaX1;
					currentY2 += fixedDeltaX2;
					currentY3 += fixedDeltaX3;

					// Increment the color.
					colorYBarySSE += colorYDeltaSSE;

					// Increment the buffer
					tempBuffer += width;
					tempZBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
		zBuffer += blockSize * width;
	}
}

void IRenderTarget::DrawInterpolatedTriangleTextured(DrawBuffer::Triangle& _triangle) {
	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;
	const SSEVectorInteger fixedDeltaX(fixedDeltaX1, fixedDeltaX2, fixedDeltaX3, 0);

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;
	const SSEVectorInteger fixedDeltaY(fixedDeltaY1, fixedDeltaY2, fixedDeltaY3, 0);

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	/* ------------------------ SETUP INTERPOLANTS ------------------------ */
	// Barycentric delta's
	float alphaDen = (_triangle.vertices[1].y - _triangle.vertices[2].y) * _triangle.vertices[0].x +
					(_triangle.vertices[2].x - _triangle.vertices[1].x) * _triangle.vertices[0].y +
					 ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y));
	float betaDen = (_triangle.vertices[2].y - _triangle.vertices[0].y) * _triangle.vertices[1].x +
					(_triangle.vertices[0].x - _triangle.vertices[2].x) * _triangle.vertices[1].y +
					 ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y));
	float gammaDen = (_triangle.vertices[0].y - _triangle.vertices[1].y) * _triangle.vertices[2].x +
					(_triangle.vertices[1].x - _triangle.vertices[0].x) * _triangle.vertices[2].y +
					 ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y));
	// If any of the denominators are 0, then we can through out the triangle since it is degenerate (on a line)
	if (alphaDen == 0 || betaDen == 0 || gammaDen == 0) return;

	float alphaStartBary = ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y)) / alphaDen;
	float betaStartBary = ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y)) / betaDen;
	float gammaStartBary = ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y)) / gammaDen;

	// X delta's
	float alphaXDelta = (_triangle.vertices[1].y - _triangle.vertices[2].y) / alphaDen;
	float betaXDelta = (_triangle.vertices[2].y - _triangle.vertices[0].y) / betaDen;
	float gammaXDelta = (_triangle.vertices[0].y - _triangle.vertices[1].y) / gammaDen;

	// Y delta's
	float alphaYDelta = (_triangle.vertices[2].x - _triangle.vertices[1].x) / alphaDen;
	float betaYDelta = (_triangle.vertices[0].x - _triangle.vertices[2].x) / betaDen;
	float gammaYDelta = (_triangle.vertices[1].x - _triangle.vertices[0].x) / gammaDen;

	////////////////////////// TEXTURE COORD intepolation /////////////////////
	// Calculate the differences so that we can get a value that we can increment the pixel buffer for the texture.
	Vector4 textureCoordStart = (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaStartBary + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaStartBary + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaStartBary;
	Vector4 textureCoordXDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaXDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaXDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaXDelta;
	Vector4 textureCoordYDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaYDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaYDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaYDelta;

	float oneOverZStart = ((1/_triangle.originalZ[0]) * alphaStartBary + (1/_triangle.originalZ[1]) * betaStartBary + (1/_triangle.originalZ[2]) * gammaStartBary);
	float oneOverZXDelta = ((1/_triangle.originalZ[0]) * alphaXDelta + (1/_triangle.originalZ[1]) * betaXDelta + (1/_triangle.originalZ[2]) * gammaXDelta);
	float oneOverZYDelta = ((1/_triangle.originalZ[0]) * alphaYDelta + (1/_triangle.originalZ[1]) * betaYDelta + (1/_triangle.originalZ[2]) * gammaYDelta);

	// Get the texture data pixels
	unsigned int* textureData = (unsigned int*)_triangle.texture->GetPixels();
	int textureWidth = _triangle.texture->GetWidth();
	int textureHeight = _triangle.texture->GetHeight();
	
	// Put the u/z, v/z and 1/z values into one SSE 
	SSEVector textureCoordStartSSE(textureCoordStart.x, textureCoordStart.y, oneOverZStart, 0);
	SSEVector textureCoordXDeltaSSE(textureCoordXDelta.x, textureCoordXDelta.y, oneOverZXDelta, 0);
	SSEVector textureCoordYDeltaSSE(textureCoordYDelta.x, textureCoordYDelta.y, oneOverZYDelta, 0);

	//////////////////////// Z interpolation ///////////////////////////////
	// We can probably pack the z into the same vector as the texture coords...
	float zStart = (_triangle.vertices[0].z - 1.0f) * alphaStartBary + (_triangle.vertices[1].z - 1.0f) * betaStartBary + (_triangle.vertices[2].z - 1.0f) * gammaStartBary;
	float zXDelta = (_triangle.vertices[0].z - 1.0f) * alphaXDelta + (_triangle.vertices[1].z - 1.0f) * betaXDelta + (_triangle.vertices[2].z - 1.0f) * gammaXDelta;
	float zYDelta = (_triangle.vertices[0].z - 1.0f) * alphaYDelta + (_triangle.vertices[1].z - 1.0f) * betaYDelta + (_triangle.vertices[2].z - 1.0f) * gammaYDelta;
	textureCoordStartSSE.w = zStart;
	textureCoordXDeltaSSE.w = zXDelta;
	textureCoordYDeltaSSE.w = zYDelta;

	// Get the pixels pointer to draw to
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Get the zbuffer pointer to access
	float* zBuffer = GetZBuffer();
	zBuffer += ymin * width;

	// Get the face color as an SSEVector
	SSEVector faceColor(_triangle.faceColor.x, _triangle.faceColor.y, _triangle.faceColor.z, 0);

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// Calculate the start of our texture coordinates.
			SSEVector textureCoordYBarySSE = textureCoordStartSSE;
			SSEVector temp = textureCoordXDeltaSSE;
			temp *= (float)x;
			textureCoordYBarySSE += temp;
			temp = textureCoordYDeltaSSE;
			temp *= (float)y;
			textureCoordYBarySSE += temp;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			unsigned int* tempBuffer = pixels;
			float* tempZBuffer = zBuffer;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					// Set the interpolated colors back to the start of a line.
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						
						// Put the pixel if the z coord is bigger than the one in the z buffer
						if (textureCoordXBarySSE.w < tempZBuffer[blockX]) {

							// Calculate the actual uv coordinates (perspective correct)
							float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
							SSEVector tempUV = textureCoordXBarySSE;
							tempUV *= z; // Correct the uv coordinates								
								
							// Get the uv coordinates
							tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
							__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
							// Sample the colors
							int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

							// Mix the texture color with the lighting color
							SSEVector color(((unsigned char*)&texColor)[2] / (float)255, 
											((unsigned char*)&texColor)[1] / (float)255, 
											((unsigned char*)&texColor)[0] / (float)255,
											0);
							color *= faceColor;
							color *= SSEVector(255);

							// Get the color
							__m128i plotColor = _mm_cvttps_epi32(color.data);
							unsigned int tempPixel = plotColor.m128i_i32[1] << 0 | plotColor.m128i_i32[2] << 8 | plotColor.m128i_i32[3] << 16;

							// Set the pixel
							tempBuffer[blockX] = tempPixel;

							// Mark this on the z buffer
							tempZBuffer[blockX] = textureCoordXBarySSE.w;
						}

						// Increment the color.
						textureCoordXBarySSE += textureCoordXDeltaSSE;
					}
					tempBuffer += width;
					tempZBuffer += width;

					// Increment the color.
					textureCoordYBarySSE += textureCoordYDeltaSSE;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this block.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				SSEVectorInteger currentY(currentY1, currentY2, currentY3, 0);
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					SSEVectorInteger currentX = currentY;

					// Set the interpolated colors back to the start of a line.
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX.x > 0 && currentX.y > 0 && currentX.z > 0) {
							// Put the pixel if the z coord is bigger than the one in the z buffer
							if (textureCoordXBarySSE.w < tempZBuffer[blockX]) {
								// Calculate the actual uv coordinates (perspective correct)
								float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
								SSEVector tempUV = textureCoordXBarySSE;
								tempUV *= z; // Correct the uv coordinates								
								
								// Get the uv coordinates
								tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
								__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
								// Sample the colors
								int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

								// Mix the texture color with the lighting color
								SSEVector color(((unsigned char*)&texColor)[2] / (float)255, 
												((unsigned char*)&texColor)[1] / (float)255, 
												((unsigned char*)&texColor)[0] / (float)255,
												0);
								color *= faceColor;
								color *= SSEVector(255);

								// Get the color
								__m128i plotColor = _mm_cvttps_epi32(color.data);
								unsigned int tempPixel = plotColor.m128i_i32[1] << 0 | plotColor.m128i_i32[2] << 8 | plotColor.m128i_i32[3] << 16;

								// Set the pixel
								tempBuffer[blockX] = tempPixel;

								// Mark this on the z buffer
								tempZBuffer[blockX] = textureCoordXBarySSE.w;
							}
						}

						// Increment the color.
						textureCoordXBarySSE += textureCoordXDeltaSSE;

						// Increment the half space
						currentX -= fixedDeltaY;
					}
					// Increment the half space
					currentY += fixedDeltaX;

					// Increment the color.
					textureCoordYBarySSE += textureCoordYDeltaSSE;

					// Increment the buffer
					tempBuffer += width;
					tempZBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
		zBuffer += blockSize * width;
	}
}

void IRenderTarget::DrawInterpolatedTriangleTexturedLighted(DrawBuffer::Triangle& _triangle) {
	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;
	const SSEVectorInteger fixedDeltaX(fixedDeltaX1, fixedDeltaX2, fixedDeltaX3, 0);

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;
	const SSEVectorInteger fixedDeltaY(fixedDeltaY1, fixedDeltaY2, fixedDeltaY3, 0);

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	/* ------------------------ SETUP INTERPOLANTS ------------------------ */
	// Barycentric delta's
	float alphaDen = (_triangle.vertices[1].y - _triangle.vertices[2].y) * _triangle.vertices[0].x +
					(_triangle.vertices[2].x - _triangle.vertices[1].x) * _triangle.vertices[0].y +
					 ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y));
	float betaDen = (_triangle.vertices[2].y - _triangle.vertices[0].y) * _triangle.vertices[1].x +
					(_triangle.vertices[0].x - _triangle.vertices[2].x) * _triangle.vertices[1].y +
					 ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y));
	float gammaDen = (_triangle.vertices[0].y - _triangle.vertices[1].y) * _triangle.vertices[2].x +
					(_triangle.vertices[1].x - _triangle.vertices[0].x) * _triangle.vertices[2].y +
					 ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y));
	// If any of the denominators are 0, then we can through out the triangle since it is degenerate (on a line)
	if (alphaDen == 0 || betaDen == 0 || gammaDen == 0) return;

	float alphaStartBary = ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y)) / alphaDen;
	float betaStartBary = ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y)) / betaDen;
	float gammaStartBary = ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y)) / gammaDen;

	// X delta's
	float alphaXDelta = (_triangle.vertices[1].y - _triangle.vertices[2].y) / alphaDen;
	float betaXDelta = (_triangle.vertices[2].y - _triangle.vertices[0].y) / betaDen;
	float gammaXDelta = (_triangle.vertices[0].y - _triangle.vertices[1].y) / gammaDen;

	// Y delta's
	float alphaYDelta = (_triangle.vertices[2].x - _triangle.vertices[1].x) / alphaDen;
	float betaYDelta = (_triangle.vertices[0].x - _triangle.vertices[2].x) / betaDen;
	float gammaYDelta = (_triangle.vertices[1].x - _triangle.vertices[0].x) / gammaDen;

	//////////////////////// Color interpolation ///////////////////////////
	Vector4 colorStart = _triangle.vertexColors[0] * alphaStartBary + _triangle.vertexColors[1] * betaStartBary + _triangle.vertexColors[2] * gammaStartBary;
	Vector4 colorXDelta = _triangle.vertexColors[0] * alphaXDelta + _triangle.vertexColors[1] * betaXDelta + _triangle.vertexColors[2] * gammaXDelta;
	Vector4 colorYDelta = _triangle.vertexColors[0] * alphaYDelta + _triangle.vertexColors[1] * betaYDelta + _triangle.vertexColors[2] * gammaYDelta;
	SSEVector colorStartSSE(colorStart.x, colorStart.y, colorStart.z, 0);
	SSEVector colorXDeltaSSE(colorXDelta.x, colorXDelta.y, colorXDelta.z, 0);
	SSEVector colorYDeltaSSE(colorYDelta.x, colorYDelta.y, colorYDelta.z, 0);

	//////////////////////// Z interpolation ///////////////////////////////
	// We can probably pack the z into the same vector as the colors...
	float zStart = (_triangle.vertices[0].z - 1.0f) * alphaStartBary + (_triangle.vertices[1].z - 1.0f) * betaStartBary + (_triangle.vertices[2].z - 1.0f) * gammaStartBary;
	float zXDelta = (_triangle.vertices[0].z - 1.0f) * alphaXDelta + (_triangle.vertices[1].z - 1.0f) * betaXDelta + (_triangle.vertices[2].z - 1.0f) * gammaXDelta;
	float zYDelta = (_triangle.vertices[0].z - 1.0f) * alphaYDelta + (_triangle.vertices[1].z - 1.0f) * betaYDelta + (_triangle.vertices[2].z - 1.0f) * gammaYDelta;
	colorStartSSE.w = zStart;
	colorXDeltaSSE.w = zXDelta;
	colorYDeltaSSE.w = zYDelta;

	////////////////////////// TEXTURE COORD intepolation /////////////////////
	// Calculate the differences so that we can get a value that we can increment the pixel buffer for the texture.
	Vector4 textureCoordStart = (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaStartBary + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaStartBary + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaStartBary;
	Vector4 textureCoordXDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaXDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaXDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaXDelta;
	Vector4 textureCoordYDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaYDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaYDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaYDelta;

	float oneOverZStart = ((1/_triangle.originalZ[0]) * alphaStartBary + (1/_triangle.originalZ[1]) * betaStartBary + (1/_triangle.originalZ[2]) * gammaStartBary);
	float oneOverZXDelta = ((1/_triangle.originalZ[0]) * alphaXDelta + (1/_triangle.originalZ[1]) * betaXDelta + (1/_triangle.originalZ[2]) * gammaXDelta);
	float oneOverZYDelta = ((1/_triangle.originalZ[0]) * alphaYDelta + (1/_triangle.originalZ[1]) * betaYDelta + (1/_triangle.originalZ[2]) * gammaYDelta);

	// Get the texture data pixels
	unsigned int* textureData = (unsigned int*)_triangle.texture->GetPixels();
	int textureWidth = _triangle.texture->GetWidth();
	int textureHeight = _triangle.texture->GetHeight();
	
	// Put the u/z, v/z and 1/z values into one SSE 
	SSEVector textureCoordStartSSE(textureCoordStart.x, textureCoordStart.y, oneOverZStart, 0);
	SSEVector textureCoordXDeltaSSE(textureCoordXDelta.x, textureCoordXDelta.y, oneOverZXDelta, 0);
	SSEVector textureCoordYDeltaSSE(textureCoordYDelta.x, textureCoordYDelta.y, oneOverZYDelta, 0);

	// Get the pixels pointer to draw to
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Get the zbuffer pointer to access
	float* zBuffer = GetZBuffer();
	zBuffer += ymin * width;

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// Calculate the start of our interpolants # colorYBary = colorStart + colorXDelta * x + colorYDelta * y; (in other words)
			SSEVector colorYBarySSE = colorStartSSE;
			SSEVector temp = colorXDeltaSSE;
			temp *= (float)x;
			colorYBarySSE += temp;
			temp = colorYDeltaSSE;
			temp *= (float)y;
			colorYBarySSE += temp;

			// Calculate the start of our texture coordinates.
			SSEVector textureCoordYBarySSE = textureCoordStartSSE;
			temp = textureCoordXDeltaSSE;
			temp *= (float)x;
			textureCoordYBarySSE += temp;
			temp = textureCoordYDeltaSSE;
			temp *= (float)y;
			textureCoordYBarySSE += temp;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			unsigned int* tempBuffer = pixels;
			float* tempZBuffer = zBuffer;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					// Set the interpolated colors back to the start of a line.
					SSEVector colorXBarySSE = colorYBarySSE;
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						
						// Put the pixel if the z coord is bigger than the one in the z buffer
						if (colorXBarySSE.w < tempZBuffer[blockX]) {

							// Calculate the actual uv coordinates (perspective correct)
							float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
							SSEVector tempUV = textureCoordXBarySSE;
							tempUV *= z; // Correct the uv coordinates								
								
							// Get the uv coordinates
							tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
							__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
							// Sample the colors
							int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

							// Mix the texture color with the lighting color
							SSEVector color(((unsigned char*)&texColor)[2] / (float)255, 
											((unsigned char*)&texColor)[1] / (float)255, 
											((unsigned char*)&texColor)[0] / (float)255,
											0);
							color *= colorXBarySSE;
							color *= SSEVector(255);

							// Get the color
							__m128i plotColor = _mm_cvttps_epi32(color.data);
							unsigned int tempPixel = plotColor.m128i_i32[1] << 0 | plotColor.m128i_i32[2] << 8 | plotColor.m128i_i32[3] << 16;

							// Set the pixel
							tempBuffer[blockX] = tempPixel;

							// Mark this on the z buffer
							tempZBuffer[blockX] = colorXBarySSE.w;
						}

						// Increment the color.
						colorXBarySSE += colorXDeltaSSE;
						textureCoordXBarySSE += textureCoordXDeltaSSE;
					}
					tempBuffer += width;
					tempZBuffer += width;

					// Increment the color.
					colorYBarySSE += colorYDeltaSSE;
					textureCoordYBarySSE += textureCoordYDeltaSSE;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this block.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				SSEVectorInteger currentY(currentY1, currentY2, currentY3, 0);
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					SSEVectorInteger currentX = currentY;

					// Set the interpolated colors back to the start of a line.
					SSEVector colorXBarySSE = colorYBarySSE;
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX.x > 0 && currentX.y > 0 && currentX.z > 0) {
							// Put the pixel if the z coord is bigger than the one in the z buffer
							if (colorXBarySSE.w < tempZBuffer[blockX]) {
								// Calculate the actual uv coordinates (perspective correct)
								float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
								SSEVector tempUV = textureCoordXBarySSE;
								tempUV *= z; // Correct the uv coordinates								
								
								// Get the uv coordinates
								tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
								__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
								// Sample the colors
								int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

								// Mix the texture color with the lighting color
								SSEVector color(((unsigned char*)&texColor)[2] / (float)255, 
												((unsigned char*)&texColor)[1] / (float)255, 
												((unsigned char*)&texColor)[0] / (float)255,
												0);
								color *= colorXBarySSE;
								color *= SSEVector(255);

								// Get the color
								__m128i plotColor = _mm_cvttps_epi32(color.data);
								unsigned int tempPixel = plotColor.m128i_i32[1] << 0 | plotColor.m128i_i32[2] << 8 | plotColor.m128i_i32[3] << 16;

								// Set the pixel
								tempBuffer[blockX] = tempPixel;

								// Mark this on the z buffer
								tempZBuffer[blockX] = colorXBarySSE.w;
							}
						}

						// Increment the color.
						colorXBarySSE += colorXDeltaSSE;
						textureCoordXBarySSE += textureCoordXDeltaSSE;

						// Increment the half space
						currentX -= fixedDeltaY;
					}
					// Increment the half space
					currentY += fixedDeltaX;

					// Increment the color.
					colorYBarySSE += colorYDeltaSSE;
					textureCoordYBarySSE += textureCoordYDeltaSSE;

					// Increment the buffer
					tempBuffer += width;
					tempZBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
		zBuffer += blockSize * width;
	}
}

// Draw a triangle using phong shading.
void IRenderTarget::DrawInterpolatedTrianglePhong(DrawBuffer::Triangle& _triangle) {
	// Get the color as a pixel
	Pixel pixel = ConvertColorToPixel(_triangle.vertexColors[0]);

	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	/* ------------------------ SETUP INTERPOLANTS ------------------------ */
	// Barycentric delta's
	float alphaDen = (_triangle.vertices[1].y - _triangle.vertices[2].y) * _triangle.vertices[0].x +
					(_triangle.vertices[2].x - _triangle.vertices[1].x) * _triangle.vertices[0].y +
					 ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y));
	float betaDen = (_triangle.vertices[2].y - _triangle.vertices[0].y) * _triangle.vertices[1].x +
					(_triangle.vertices[0].x - _triangle.vertices[2].x) * _triangle.vertices[1].y +
					 ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y));
	float gammaDen = (_triangle.vertices[0].y - _triangle.vertices[1].y) * _triangle.vertices[2].x +
					(_triangle.vertices[1].x - _triangle.vertices[0].x) * _triangle.vertices[2].y +
					 ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y));
	float alphaStartBary = ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y)) / alphaDen;
	float betaStartBary = ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y)) / betaDen;
	float gammaStartBary = ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y)) / gammaDen;

	// X delta's
	float alphaXDelta = (_triangle.vertices[1].y - _triangle.vertices[2].y) / alphaDen;
	float betaXDelta = (_triangle.vertices[2].y - _triangle.vertices[0].y) / betaDen;
	float gammaXDelta = (_triangle.vertices[0].y - _triangle.vertices[1].y) / gammaDen;

	// Y delta's
	float alphaYDelta = (_triangle.vertices[2].x - _triangle.vertices[1].x) / alphaDen;
	float betaYDelta = (_triangle.vertices[0].x - _triangle.vertices[2].x) / betaDen;
	float gammaYDelta = (_triangle.vertices[1].x - _triangle.vertices[0].x) / gammaDen;

	//////////////////////// Normal interpolation ///////////////////////////
	Vector4 normalStart = _triangle.vertexNormals[0] * alphaStartBary + _triangle.vertexNormals[1] * betaStartBary + _triangle.vertexNormals[2] * gammaStartBary;
	Vector4 normalXDelta = _triangle.vertexNormals[0] * alphaXDelta + _triangle.vertexNormals[1] * betaXDelta + _triangle.vertexNormals[2] * gammaXDelta;
	Vector4 normalYDelta = _triangle.vertexNormals[0] * alphaYDelta + _triangle.vertexNormals[1] * betaYDelta + _triangle.vertexNormals[2] * gammaYDelta;
	SSEVector normalStartSSE(normalStart.x, normalStart.y, normalStart.z, 0);
	SSEVector normalXDeltaSSE(normalXDelta.x, normalXDelta.y, normalXDelta.z, 0);
	SSEVector normalYDeltaSSE(normalYDelta.x, normalYDelta.y, normalYDelta.z, 0);

	//////////////////////// Z interpolation ///////////////////////////////
	// We can probably pack the z into the same vector as the colors...
	float zStart = (_triangle.vertices[0].z - 1.0f) * alphaStartBary + (_triangle.vertices[1].z - 1.0f) * betaStartBary + (_triangle.vertices[2].z - 1.0f) * gammaStartBary;
	float zXDelta = (_triangle.vertices[0].z - 1.0f) * alphaXDelta + (_triangle.vertices[1].z - 1.0f) * betaXDelta + (_triangle.vertices[2].z - 1.0f) * gammaXDelta;
	float zYDelta = (_triangle.vertices[0].z - 1.0f) * alphaYDelta + (_triangle.vertices[1].z - 1.0f) * betaYDelta + (_triangle.vertices[2].z - 1.0f) * gammaYDelta;
	normalStartSSE.w = zStart;
	normalXDeltaSSE.w = zXDelta;
	normalYDeltaSSE.w = zYDelta;

	//////////////////////// CameraSpace positions /////////////////////////
	Vector4 cameraSpaceStart = _triangle.cameraSpace[0] * alphaStartBary + _triangle.cameraSpace[1] * betaStartBary + _triangle.cameraSpace[2] * gammaStartBary;
	Vector4 cameraSpaceXDelta = _triangle.cameraSpace[0] * alphaXDelta + _triangle.cameraSpace[1] * betaXDelta + _triangle.cameraSpace[2] * gammaXDelta;
	Vector4 cameraSpaceYDelta = _triangle.cameraSpace[0] * alphaYDelta + _triangle.cameraSpace[1] * betaYDelta + _triangle.cameraSpace[2] * gammaYDelta;
	SSEVector cameraSpaceStartSSE(cameraSpaceStart.x, cameraSpaceStart.y, cameraSpaceStart.z, 0);
	SSEVector cameraSpaceXDeltaSSE(cameraSpaceXDelta.x, cameraSpaceXDelta.y, cameraSpaceXDelta.z, 0);
	SSEVector cameraSpaceYDeltaSSE(cameraSpaceYDelta.x, cameraSpaceYDelta.y, cameraSpaceYDelta.z, 0);

	//////////////////////// WorldSpace positions //////////////////////////
	Vector4 worldSpaceStart = _triangle.worldSpace[0] * alphaStartBary + _triangle.worldSpace[1] * betaStartBary + _triangle.worldSpace[2] * gammaStartBary;
	Vector4 worldSpaceXDelta = _triangle.worldSpace[0] * alphaXDelta + _triangle.worldSpace[1] * betaXDelta + _triangle.worldSpace[2] * gammaXDelta;
	Vector4 worldSpaceYDelta = _triangle.worldSpace[0] * alphaYDelta + _triangle.worldSpace[1] * betaYDelta + _triangle.worldSpace[2] * gammaYDelta;
	SSEVector worldSpaceStartSSE(worldSpaceStart.x, worldSpaceStart.y, worldSpaceStart.z, 0);
	SSEVector worldSpaceXDeltaSSE(worldSpaceXDelta.x, worldSpaceXDelta.y, worldSpaceXDelta.z, 0);
	SSEVector worldSpaceYDeltaSSE(worldSpaceYDelta.x, worldSpaceYDelta.y, worldSpaceYDelta.z, 0);

	// Get the ambient, diffuse and specular values as sse vectors
	SSEVector ambient(_triangle.ambient.x, _triangle.ambient.y, _triangle.ambient.z, 0);
	SSEVector diffuse(_triangle.diffuse.x, _triangle.diffuse.y, _triangle.diffuse.z, 0);
	SSEVector specular(_triangle.specular.x, _triangle.specular.y, _triangle.specular.z, 0);

	// Get the pixels pointer to draw to
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Get the zbuffer pointer to access
	float* zBuffer = GetZBuffer();
	zBuffer += ymin * width;

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// Calculate the start of our interpolants # colorYBary = normalStart + normalXDelta * x + normalYDelta * y; (in other words)
			SSEVector normalYBarySSE = normalStartSSE;
			SSEVector temp = normalXDeltaSSE;
			temp *= (float)x;
			normalYBarySSE += temp;
			temp = normalYDeltaSSE;
			temp *= (float)y;
			normalYBarySSE += temp;

			// Calculate the start of our interpolants # cameraSpaceYBary = cameraSpaceStart + cameraSpaceXDelta * x + cameraSpaceYDelta * y; (in other words)
			SSEVector cameraSpaceYBarySSE = cameraSpaceStartSSE;
			temp = cameraSpaceXDeltaSSE;
			temp *= (float)x;
			cameraSpaceYBarySSE += temp;
			temp = cameraSpaceYDeltaSSE;
			temp *= (float)y;
			cameraSpaceYBarySSE += temp;

			// Calculate the start of our interpolants # worldSpaceYBary = worldSpaceStart + worldSpaceXDelta * x + worldSpaceYDelta * y; (in other words)
			SSEVector worldSpaceYBarySSE = worldSpaceStartSSE;
			temp = worldSpaceXDeltaSSE;
			temp *= (float)x;
			worldSpaceYBarySSE += temp;
			temp = worldSpaceYDeltaSSE;
			temp *= (float)y;
			worldSpaceYBarySSE += temp;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			unsigned int* tempBuffer = pixels;
			float* tempZBuffer = zBuffer;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					// Set the interpolated colors back to the start of a line.
					SSEVector normalXBarySSE = normalYBarySSE;
					SSEVector worldSpaceXBarySSE = worldSpaceYBarySSE;
					SSEVector cameraSpaceXBarySSE = cameraSpaceYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						
						// Put the pixel if the z coord is bigger than the one in the z buffer
						if (normalXBarySSE.w < tempZBuffer[blockX]) {
							// Calculate the color we should show.
							SSEVector color;
							
							// Get a normalized version of the normal
							SSEVector normal = normalXBarySSE;
							normal.Normalize();

							// Ambient light
							_triangle.info->GetAmbientLightOnPoint(color);

							// Directional light
							for (unsigned int i = 0; i < _triangle.info->GetDirectionalLights().size(); ++i) {
								_triangle.info->GetDirectionalLightOnPoint(cameraSpaceXBarySSE, 
																	 normal,
																	 i,
																	 diffuse,
																	 specular,
																	 color);
							}

							// Point lights
							for (unsigned int i = 0; i < _triangle.info->GetPointLights().size(); ++i) {
								_triangle.info->GetPointLightOnPoint(cameraSpaceXBarySSE,
																worldSpaceXBarySSE,
																normal,
																i,
																diffuse,
																specular,
																color);
							}

							// Clamp the color and mutliply by 255
							color.Clamp();
							color *= 255;

							__m128i pixel = _mm_cvtps_epi32(color.data);

							// Set the pixel
							unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
							tempBuffer[blockX] = tempPixel;

							// Mark this on the z buffer
							tempZBuffer[blockX] = normalXBarySSE.w;
						}

						// Increment the color.
						normalXBarySSE += normalXDeltaSSE;
						cameraSpaceXBarySSE += cameraSpaceXDeltaSSE;
						worldSpaceXBarySSE += worldSpaceXDeltaSSE;
					}
					tempBuffer += width;
					tempZBuffer += width;

					// Increment the color.
					normalYBarySSE += normalYDeltaSSE;
					cameraSpaceYBarySSE += cameraSpaceYDeltaSSE;
					worldSpaceYBarySSE += worldSpaceYDeltaSSE;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this block.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					int currentX1 = currentY1;
					int currentX2 = currentY2;
					int currentX3 = currentY3;

					// Set the interpolated colors back to the start of a line.
					SSEVector normalXBarySSE = normalYBarySSE;
					SSEVector worldSpaceXBarySSE = worldSpaceYBarySSE;
					SSEVector cameraSpaceXBarySSE = cameraSpaceYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX1 > 0 && currentX2 > 0 && currentX3 > 0) {
							// Put the pixel if the z coord is bigger than the one in the z buffer
							if (normalXBarySSE.w < tempZBuffer[blockX]) {
								// Put the pixel if the z coord is bigger than the one in the z buffer
								// Calculate the color we should show.
								SSEVector color;
								
								// Get a normalized version of the normal
								SSEVector normal = normalXBarySSE;
								normal.Normalize();

								// Ambient light
								_triangle.info->GetAmbientLightOnPoint(color);

								// Directional light
								for (unsigned int i = 0; i < _triangle.info->GetDirectionalLights().size(); ++i) {
									_triangle.info->GetDirectionalLightOnPoint(cameraSpaceXBarySSE, 
																			normal,
																			i,
																			diffuse,
																			specular,
																			color);
								}

								// Point lights
								for (unsigned int i = 0; i < _triangle.info->GetPointLights().size(); ++i) {
									_triangle.info->GetPointLightOnPoint(cameraSpaceXBarySSE,
																	worldSpaceXBarySSE,
																	normal,
																	i,
																	diffuse,
																	specular,
																	color);
								}

								// Clamp the color and mutliply by 255
								color.Clamp();
								color *= 255;

								__m128i pixel = _mm_cvtps_epi32(color.data);

								// Set the pixel
								unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
								tempBuffer[blockX] = tempPixel;

								// Mark this on the z buffer
								tempZBuffer[blockX] = normalXBarySSE.w;
							}
						}

						// Increment the color.
						normalXBarySSE += normalXDeltaSSE;
						cameraSpaceXBarySSE += cameraSpaceXDeltaSSE;
						worldSpaceXBarySSE += worldSpaceXDeltaSSE;

						// Increment the half space
						currentX1 -= fixedDeltaY1;
						currentX2 -= fixedDeltaY2;
						currentX3 -= fixedDeltaY3;
					}
					// Increment the half space
					currentY1 += fixedDeltaX1;
					currentY2 += fixedDeltaX2;
					currentY3 += fixedDeltaX3;

					// Increment the color.
					normalYBarySSE += normalYDeltaSSE;
					cameraSpaceYBarySSE += cameraSpaceYDeltaSSE;
					worldSpaceYBarySSE += worldSpaceYDeltaSSE;

					// Increment the buffer
					tempBuffer += width;
					tempZBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
		zBuffer += blockSize * width;
	}
}

// Draw a triangle using phong shading with texturing.
void IRenderTarget::DrawInterpolatedTrianglePhongTextured(DrawBuffer::Triangle& _triangle) {
	// Get the color as a pixel
	Pixel pixel = ConvertColorToPixel(_triangle.vertexColors[0]);

	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	/* ------------------------ SETUP INTERPOLANTS ------------------------ */
	// Barycentric delta's
	float alphaDen = (_triangle.vertices[1].y - _triangle.vertices[2].y) * _triangle.vertices[0].x +
					(_triangle.vertices[2].x - _triangle.vertices[1].x) * _triangle.vertices[0].y +
					 ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y));
	float betaDen = (_triangle.vertices[2].y - _triangle.vertices[0].y) * _triangle.vertices[1].x +
					(_triangle.vertices[0].x - _triangle.vertices[2].x) * _triangle.vertices[1].y +
					 ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y));
	float gammaDen = (_triangle.vertices[0].y - _triangle.vertices[1].y) * _triangle.vertices[2].x +
					(_triangle.vertices[1].x - _triangle.vertices[0].x) * _triangle.vertices[2].y +
					 ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y));
	float alphaStartBary = ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y)) / alphaDen;
	float betaStartBary = ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y)) / betaDen;
	float gammaStartBary = ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y)) / gammaDen;

	// X delta's
	float alphaXDelta = (_triangle.vertices[1].y - _triangle.vertices[2].y) / alphaDen;
	float betaXDelta = (_triangle.vertices[2].y - _triangle.vertices[0].y) / betaDen;
	float gammaXDelta = (_triangle.vertices[0].y - _triangle.vertices[1].y) / gammaDen;

	// Y delta's
	float alphaYDelta = (_triangle.vertices[2].x - _triangle.vertices[1].x) / alphaDen;
	float betaYDelta = (_triangle.vertices[0].x - _triangle.vertices[2].x) / betaDen;
	float gammaYDelta = (_triangle.vertices[1].x - _triangle.vertices[0].x) / gammaDen;

	//////////////////////// Normal interpolation ///////////////////////////
	Vector4 normalStart = _triangle.vertexNormals[0] * alphaStartBary + _triangle.vertexNormals[1] * betaStartBary + _triangle.vertexNormals[2] * gammaStartBary;
	Vector4 normalXDelta = _triangle.vertexNormals[0] * alphaXDelta + _triangle.vertexNormals[1] * betaXDelta + _triangle.vertexNormals[2] * gammaXDelta;
	Vector4 normalYDelta = _triangle.vertexNormals[0] * alphaYDelta + _triangle.vertexNormals[1] * betaYDelta + _triangle.vertexNormals[2] * gammaYDelta;
	SSEVector normalStartSSE(normalStart.x, normalStart.y, normalStart.z, 0);
	SSEVector normalXDeltaSSE(normalXDelta.x, normalXDelta.y, normalXDelta.z, 0);
	SSEVector normalYDeltaSSE(normalYDelta.x, normalYDelta.y, normalYDelta.z, 0);

	//////////////////////// Z interpolation ///////////////////////////////
	// We can probably pack the z into the same vector as the colors...
	float zStart = (_triangle.vertices[0].z - 1.0f) * alphaStartBary + (_triangle.vertices[1].z - 1.0f) * betaStartBary + (_triangle.vertices[2].z - 1.0f) * gammaStartBary;
	float zXDelta = (_triangle.vertices[0].z - 1.0f) * alphaXDelta + (_triangle.vertices[1].z - 1.0f) * betaXDelta + (_triangle.vertices[2].z - 1.0f) * gammaXDelta;
	float zYDelta = (_triangle.vertices[0].z - 1.0f) * alphaYDelta + (_triangle.vertices[1].z - 1.0f) * betaYDelta + (_triangle.vertices[2].z - 1.0f) * gammaYDelta;
	normalStartSSE.w = zStart;
	normalXDeltaSSE.w = zXDelta;
	normalYDeltaSSE.w = zYDelta;

	//////////////////////// CameraSpace positions /////////////////////////
	Vector4 cameraSpaceStart = _triangle.cameraSpace[0] * alphaStartBary + _triangle.cameraSpace[1] * betaStartBary + _triangle.cameraSpace[2] * gammaStartBary;
	Vector4 cameraSpaceXDelta = _triangle.cameraSpace[0] * alphaXDelta + _triangle.cameraSpace[1] * betaXDelta + _triangle.cameraSpace[2] * gammaXDelta;
	Vector4 cameraSpaceYDelta = _triangle.cameraSpace[0] * alphaYDelta + _triangle.cameraSpace[1] * betaYDelta + _triangle.cameraSpace[2] * gammaYDelta;
	SSEVector cameraSpaceStartSSE(cameraSpaceStart.x, cameraSpaceStart.y, cameraSpaceStart.z, 0);
	SSEVector cameraSpaceXDeltaSSE(cameraSpaceXDelta.x, cameraSpaceXDelta.y, cameraSpaceXDelta.z, 0);
	SSEVector cameraSpaceYDeltaSSE(cameraSpaceYDelta.x, cameraSpaceYDelta.y, cameraSpaceYDelta.z, 0);

	//////////////////////// WorldSpace positions //////////////////////////
	Vector4 worldSpaceStart = _triangle.worldSpace[0] * alphaStartBary + _triangle.worldSpace[1] * betaStartBary + _triangle.worldSpace[2] * gammaStartBary;
	Vector4 worldSpaceXDelta = _triangle.worldSpace[0] * alphaXDelta + _triangle.worldSpace[1] * betaXDelta + _triangle.worldSpace[2] * gammaXDelta;
	Vector4 worldSpaceYDelta = _triangle.worldSpace[0] * alphaYDelta + _triangle.worldSpace[1] * betaYDelta + _triangle.worldSpace[2] * gammaYDelta;
	SSEVector worldSpaceStartSSE(worldSpaceStart.x, worldSpaceStart.y, worldSpaceStart.z, 0);
	SSEVector worldSpaceXDeltaSSE(worldSpaceXDelta.x, worldSpaceXDelta.y, worldSpaceXDelta.z, 0);
	SSEVector worldSpaceYDeltaSSE(worldSpaceYDelta.x, worldSpaceYDelta.y, worldSpaceYDelta.z, 0);

	// Get the ambient, diffuse and specular values as sse vectors
	SSEVector ambient(_triangle.ambient.x, _triangle.ambient.y, _triangle.ambient.z, 0);
	SSEVector diffuse(_triangle.diffuse.x, _triangle.diffuse.y, _triangle.diffuse.z, 0);
	SSEVector specular(_triangle.specular.x, _triangle.specular.y, _triangle.specular.z, 0);

	////////////////////////// TEXTURE COORD intepolation /////////////////////
	// Calculate the differences so that we can get a value that we can increment the pixel buffer for the texture.
	Vector4 textureCoordStart = (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaStartBary + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaStartBary + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaStartBary;
	Vector4 textureCoordXDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaXDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaXDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaXDelta;
	Vector4 textureCoordYDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaYDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaYDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaYDelta;

	float oneOverZStart = ((1/_triangle.originalZ[0]) * alphaStartBary + (1/_triangle.originalZ[1]) * betaStartBary + (1/_triangle.originalZ[2]) * gammaStartBary);
	float oneOverZXDelta = ((1/_triangle.originalZ[0]) * alphaXDelta + (1/_triangle.originalZ[1]) * betaXDelta + (1/_triangle.originalZ[2]) * gammaXDelta);
	float oneOverZYDelta = ((1/_triangle.originalZ[0]) * alphaYDelta + (1/_triangle.originalZ[1]) * betaYDelta + (1/_triangle.originalZ[2]) * gammaYDelta);

	// Get the texture data pixels
	unsigned int* textureData = (unsigned int*)_triangle.texture->GetPixels();
	int textureWidth = _triangle.texture->GetWidth();
	int textureHeight = _triangle.texture->GetHeight();
	
	// Put the u/z, v/z and 1/z values into one SSE 
	SSEVector textureCoordStartSSE(textureCoordStart.x, textureCoordStart.y, oneOverZStart, 0);
	SSEVector textureCoordXDeltaSSE(textureCoordXDelta.x, textureCoordXDelta.y, oneOverZXDelta, 0);
	SSEVector textureCoordYDeltaSSE(textureCoordYDelta.x, textureCoordYDelta.y, oneOverZYDelta, 0);

	// Get the pixels pointer to draw to
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Get the zbuffer pointer to access
	float* zBuffer = GetZBuffer();
	zBuffer += ymin * width;

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// Calculate the start of our interpolants # colorYBary = normalStart + normalXDelta * x + normalYDelta * y; (in other words)
			SSEVector normalYBarySSE = normalStartSSE;
			SSEVector temp = normalXDeltaSSE;
			temp *= (float)x;
			normalYBarySSE += temp;
			temp = normalYDeltaSSE;
			temp *= (float)y;
			normalYBarySSE += temp;

			// Calculate the start of our interpolants # cameraSpaceYBary = cameraSpaceStart + cameraSpaceXDelta * x + cameraSpaceYDelta * y; (in other words)
			SSEVector cameraSpaceYBarySSE = cameraSpaceStartSSE;
			temp = cameraSpaceXDeltaSSE;
			temp *= (float)x;
			cameraSpaceYBarySSE += temp;
			temp = cameraSpaceYDeltaSSE;
			temp *= (float)y;
			cameraSpaceYBarySSE += temp;

			// Calculate the start of our interpolants # worldSpaceYBary = worldSpaceStart + worldSpaceXDelta * x + worldSpaceYDelta * y; (in other words)
			SSEVector worldSpaceYBarySSE = worldSpaceStartSSE;
			temp = worldSpaceXDeltaSSE;
			temp *= (float)x;
			worldSpaceYBarySSE += temp;
			temp = worldSpaceYDeltaSSE;
			temp *= (float)y;
			worldSpaceYBarySSE += temp;

			// Calculate the start of our texture coordinates.
			SSEVector textureCoordYBarySSE = textureCoordStartSSE;
			temp = textureCoordXDeltaSSE;
			temp *= (float)x;
			textureCoordYBarySSE += temp;
			temp = textureCoordYDeltaSSE;
			temp *= (float)y;
			textureCoordYBarySSE += temp;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			unsigned int* tempBuffer = pixels;
			float* tempZBuffer = zBuffer;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					// Set the interpolated colors back to the start of a line.
					SSEVector normalXBarySSE = normalYBarySSE;
					SSEVector worldSpaceXBarySSE = worldSpaceYBarySSE;
					SSEVector cameraSpaceXBarySSE = cameraSpaceYBarySSE;
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						
						// Put the pixel if the z coord is bigger than the one in the z buffer
						if (normalXBarySSE.w < tempZBuffer[blockX]) {							
							// Calculate the color we should show.
							SSEVector color;
							
							// Get a normalized version of the normal
							SSEVector normal = normalXBarySSE;
							normal.Normalize();

							// Ambient light
							_triangle.info->GetAmbientLightOnPoint(color);

							// Directional light
							for (unsigned int i = 0; i < _triangle.info->GetDirectionalLights().size(); ++i) {
								_triangle.info->GetDirectionalLightOnPoint(cameraSpaceXBarySSE, 
																	 normal,
																	 i,
																	 diffuse,
																	 specular,
																	 color);
							}

							// Point lights
							for (unsigned int i = 0; i < _triangle.info->GetPointLights().size(); ++i) {
								_triangle.info->GetPointLightOnPoint(cameraSpaceXBarySSE,
																worldSpaceXBarySSE,
																normal,
																i,
																diffuse,
																specular,
																color);
							}

							// Clamp the color and mutliply by 255
							color.Clamp();

							// Calculate the actual uv coordinates (perspective correct)
							float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
							SSEVector tempUV = textureCoordXBarySSE;
							tempUV *= z; // Correct the uv coordinates								
								
							// Get the uv coordinates
							tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
							__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
							// Sample the colors
							int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

							// Mix the texture color with the lighting color
							SSEVector finalColor(((unsigned char*)&texColor)[2] / (float)255, 
											  ((unsigned char*)&texColor)[1] / (float)255, 
											  ((unsigned char*)&texColor)[0] / (float)255,
											  0);
							finalColor *= color;
							finalColor *= SSEVector(255);

							// Get the color
							__m128i pixel = _mm_cvtps_epi32(finalColor.data);

							// Set the pixel
							unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
							tempBuffer[blockX] = tempPixel;

							// Mark this on the z buffer
							tempZBuffer[blockX] = normalXBarySSE.w;
						}

						// Increment the color.
						normalXBarySSE += normalXDeltaSSE;
						cameraSpaceXBarySSE += cameraSpaceXDeltaSSE;
						worldSpaceXBarySSE += worldSpaceXDeltaSSE;
						textureCoordXBarySSE += textureCoordXDeltaSSE;
					}
					tempBuffer += width;
					tempZBuffer += width;

					// Increment the color.
					normalYBarySSE += normalYDeltaSSE;
					cameraSpaceYBarySSE += cameraSpaceYDeltaSSE;
					worldSpaceYBarySSE += worldSpaceYDeltaSSE;
					textureCoordYBarySSE += textureCoordYDeltaSSE;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this block.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					int currentX1 = currentY1;
					int currentX2 = currentY2;
					int currentX3 = currentY3;

					// Set the interpolated colors back to the start of a line.
					SSEVector normalXBarySSE = normalYBarySSE;
					SSEVector worldSpaceXBarySSE = worldSpaceYBarySSE;
					SSEVector cameraSpaceXBarySSE = cameraSpaceYBarySSE;
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX1 > 0 && currentX2 > 0 && currentX3 > 0) {
							// Put the pixel if the z coord is bigger than the one in the z buffer
							if (normalXBarySSE.w < tempZBuffer[blockX]) {
								// Calculate the color we should show.
								SSEVector color;
							
								// Get a normalized version of the normal
								SSEVector normal = normalXBarySSE;
								normal.Normalize();

								// Ambient light
								_triangle.info->GetAmbientLightOnPoint(color);

								// Directional light
								for (unsigned int i = 0; i < _triangle.info->GetDirectionalLights().size(); ++i) {
									_triangle.info->GetDirectionalLightOnPoint(cameraSpaceXBarySSE, 
																		 normal,
																		 i,
																		 diffuse,
																		 specular,
																		 color);
								}

								// Point lights
								for (unsigned int i = 0; i < _triangle.info->GetPointLights().size(); ++i) {
									_triangle.info->GetPointLightOnPoint(cameraSpaceXBarySSE,
																	worldSpaceXBarySSE,
																	normal,
																	i,
																	diffuse,
																	specular,
																	color);
								}

								// Clamp the color and mutliply by 255
								color.Clamp();

								// Calculate the actual uv coordinates (perspective correct)
								float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
								SSEVector tempUV = textureCoordXBarySSE;
								tempUV *= z; // Correct the uv coordinates								
								
								// Get the uv coordinates
								tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
								__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
								// Sample the colors
								int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

								// Mix the texture color with the lighting color
								SSEVector finalColor(((unsigned char*)&texColor)[2] / (float)255, 
												  ((unsigned char*)&texColor)[1] / (float)255, 
												  ((unsigned char*)&texColor)[0] / (float)255,
												  0);
								finalColor *= color;
								finalColor *= SSEVector(255);

								// Get the color
								__m128i pixel = _mm_cvtps_epi32(finalColor.data);

								// Set the pixel
								unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
								tempBuffer[blockX] = tempPixel;

								// Mark this on the z buffer
								tempZBuffer[blockX] = normalXBarySSE.w;
							}
						}

						// Increment the color.
						normalXBarySSE += normalXDeltaSSE;
						cameraSpaceXBarySSE += cameraSpaceXDeltaSSE;
						worldSpaceXBarySSE += worldSpaceXDeltaSSE;
						textureCoordXBarySSE += textureCoordXDeltaSSE;

						// Increment the half space
						currentX1 -= fixedDeltaY1;
						currentX2 -= fixedDeltaY2;
						currentX3 -= fixedDeltaY3;
					}
					// Increment the half space
					currentY1 += fixedDeltaX1;
					currentY2 += fixedDeltaX2;
					currentY3 += fixedDeltaX3;

					// Increment the color.
					normalYBarySSE += normalYDeltaSSE;
					cameraSpaceYBarySSE += cameraSpaceYDeltaSSE;
					worldSpaceYBarySSE += worldSpaceYDeltaSSE;
					textureCoordYBarySSE += textureCoordYDeltaSSE;

					// Increment the buffer
					tempBuffer += width;
					tempZBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
		zBuffer += blockSize * width;
	}
}

// Draw a triangle using phong shading with texturing.
void IRenderTarget::DrawInterpolatedTrianglePhongNormalMap(DrawBuffer::Triangle& _triangle) {
	// Get the color as a pixel
	Pixel pixel = ConvertColorToPixel(_triangle.vertexColors[0]);

	// Calculate fixed point (24.8) values for the x and y
	const int fixedX1 = (int)(16.0f*_triangle.vertices[0].x + 0.5f);
	const int fixedX2 = (int)(16.0f*_triangle.vertices[1].x + 0.5f);
	const int fixedX3 = (int)(16.0f*_triangle.vertices[2].x + 0.5f);

	const int fixedY1 = (int)(16.0f*_triangle.vertices[0].y + 0.5f);
	const int fixedY2 = (int)(16.0f*_triangle.vertices[1].y + 0.5f);
	const int fixedY3 = (int)(16.0f*_triangle.vertices[2].y + 0.5f);

	// Calculate the bounding box for the triangle
	int xmin = ((std::min(std::min(fixedX1, fixedX2), fixedX3) + 0xF) >> 4); // Not sure why doing this fixes some artifacts...
	int ymin = ((std::min(std::min(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	int xmax = ((std::max(std::max(fixedX1, fixedX2), fixedX3) + 0xF) >> 4);
	int ymax = ((std::max(std::max(fixedY1, fixedY2), fixedY3) + 0xF) >> 4);
	if (xmin < 0) xmin = 0;
	if (ymin < 0) ymin = 0;
	if (xmax > GetWidth()) xmax = GetWidth();
	if (ymax > GetHeight()) ymax = GetHeight();

	// Calculate constants that don't need to be in the loop.
	// Delta's to be added each pixel
	const int deltaX1 = fixedX1 - fixedX2;
	const int deltaX2 = fixedX2 - fixedX3;
	const int deltaX3 = fixedX3 - fixedX1;

	const int deltaY1 = fixedY1 - fixedY2;
	const int deltaY2 = fixedY2 - fixedY3;
	const int deltaY3 = fixedY3 - fixedY1;

	// Fixed point versions of the delta's
	const int fixedDeltaX1 = deltaX1 << 4;
	const int fixedDeltaX2 = deltaX2 << 4;
	const int fixedDeltaX3 = deltaX3 << 4;

	const int fixedDeltaY1 = deltaY1 << 4;
	const int fixedDeltaY2 = deltaY2 << 4;
	const int fixedDeltaY3 = deltaY3 << 4;

	// The value to start at
	int start1 = deltaY1 * fixedX1 - deltaX1 * fixedY1;
	int start2 = deltaY2 * fixedX2 - deltaX2 * fixedY2;
	int start3 = deltaY3 * fixedX3 - deltaX3 * fixedY3;

	// Correct the starting values for the fill convention
	if (deltaY1 < 0 || deltaY1 == 0 && deltaX1 > 0) start1++;
	if (deltaY2 < 0 || deltaY2 == 0 && deltaX2 > 0) start2++;
	if (deltaY3 < 0 || deltaY3 == 0 && deltaX3 > 0) start3++;

	// The block size
	int blockSize = 8;

	// Snap xmin and ymin to multiples of 8
	xmin &= ~(blockSize - 1);
	ymin &= ~(blockSize - 1);
	//xmax &= ~(blockSize - 1);
	//ymax &= ~(blockSize - 1);

	/* ------------------------ SETUP INTERPOLANTS ------------------------ */
	// Barycentric delta's
	float alphaDen = (_triangle.vertices[1].y - _triangle.vertices[2].y) * _triangle.vertices[0].x +
					(_triangle.vertices[2].x - _triangle.vertices[1].x) * _triangle.vertices[0].y +
					 ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y));
	float betaDen = (_triangle.vertices[2].y - _triangle.vertices[0].y) * _triangle.vertices[1].x +
					(_triangle.vertices[0].x - _triangle.vertices[2].x) * _triangle.vertices[1].y +
					 ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y));
	float gammaDen = (_triangle.vertices[0].y - _triangle.vertices[1].y) * _triangle.vertices[2].x +
					(_triangle.vertices[1].x - _triangle.vertices[0].x) * _triangle.vertices[2].y +
					 ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y));
	float alphaStartBary = ((_triangle.vertices[1].x * _triangle.vertices[2].y) - (_triangle.vertices[2].x * _triangle.vertices[1].y)) / alphaDen;
	float betaStartBary = ((_triangle.vertices[2].x * _triangle.vertices[0].y) - (_triangle.vertices[0].x * _triangle.vertices[2].y)) / betaDen;
	float gammaStartBary = ((_triangle.vertices[0].x * _triangle.vertices[1].y) - (_triangle.vertices[1].x * _triangle.vertices[0].y)) / gammaDen;

	// X delta's
	float alphaXDelta = (_triangle.vertices[1].y - _triangle.vertices[2].y) / alphaDen;
	float betaXDelta = (_triangle.vertices[2].y - _triangle.vertices[0].y) / betaDen;
	float gammaXDelta = (_triangle.vertices[0].y - _triangle.vertices[1].y) / gammaDen;

	// Y delta's
	float alphaYDelta = (_triangle.vertices[2].x - _triangle.vertices[1].x) / alphaDen;
	float betaYDelta = (_triangle.vertices[0].x - _triangle.vertices[2].x) / betaDen;
	float gammaYDelta = (_triangle.vertices[1].x - _triangle.vertices[0].x) / gammaDen;

	//////////////////////// Normal interpolation ///////////////////////////
	Vector4 normalStart = _triangle.vertexNormals[0] * alphaStartBary + _triangle.vertexNormals[1] * betaStartBary + _triangle.vertexNormals[2] * gammaStartBary;
	Vector4 normalXDelta = _triangle.vertexNormals[0] * alphaXDelta + _triangle.vertexNormals[1] * betaXDelta + _triangle.vertexNormals[2] * gammaXDelta;
	Vector4 normalYDelta = _triangle.vertexNormals[0] * alphaYDelta + _triangle.vertexNormals[1] * betaYDelta + _triangle.vertexNormals[2] * gammaYDelta;
	SSEVector normalStartSSE(normalStart.x, normalStart.y, normalStart.z, 0);
	SSEVector normalXDeltaSSE(normalXDelta.x, normalXDelta.y, normalXDelta.z, 0);
	SSEVector normalYDeltaSSE(normalYDelta.x, normalYDelta.y, normalYDelta.z, 0);

	//////////////////////// Z interpolation ///////////////////////////////
	// We can probably pack the z into the same vector as the colors...
	float zStart = (_triangle.vertices[0].z - 1.0f) * alphaStartBary + (_triangle.vertices[1].z - 1.0f) * betaStartBary + (_triangle.vertices[2].z - 1.0f) * gammaStartBary;
	float zXDelta = (_triangle.vertices[0].z - 1.0f) * alphaXDelta + (_triangle.vertices[1].z - 1.0f) * betaXDelta + (_triangle.vertices[2].z - 1.0f) * gammaXDelta;
	float zYDelta = (_triangle.vertices[0].z - 1.0f) * alphaYDelta + (_triangle.vertices[1].z - 1.0f) * betaYDelta + (_triangle.vertices[2].z - 1.0f) * gammaYDelta;
	normalStartSSE.w = zStart;
	normalXDeltaSSE.w = zXDelta;
	normalYDeltaSSE.w = zYDelta;

	//////////////////////// CameraSpace positions /////////////////////////
	Vector4 cameraSpaceStart = _triangle.cameraSpace[0] * alphaStartBary + _triangle.cameraSpace[1] * betaStartBary + _triangle.cameraSpace[2] * gammaStartBary;
	Vector4 cameraSpaceXDelta = _triangle.cameraSpace[0] * alphaXDelta + _triangle.cameraSpace[1] * betaXDelta + _triangle.cameraSpace[2] * gammaXDelta;
	Vector4 cameraSpaceYDelta = _triangle.cameraSpace[0] * alphaYDelta + _triangle.cameraSpace[1] * betaYDelta + _triangle.cameraSpace[2] * gammaYDelta;
	SSEVector cameraSpaceStartSSE(cameraSpaceStart.x, cameraSpaceStart.y, cameraSpaceStart.z, 0);
	SSEVector cameraSpaceXDeltaSSE(cameraSpaceXDelta.x, cameraSpaceXDelta.y, cameraSpaceXDelta.z, 0);
	SSEVector cameraSpaceYDeltaSSE(cameraSpaceYDelta.x, cameraSpaceYDelta.y, cameraSpaceYDelta.z, 0);

	//////////////////////// WorldSpace positions //////////////////////////
	Vector4 worldSpaceStart = _triangle.worldSpace[0] * alphaStartBary + _triangle.worldSpace[1] * betaStartBary + _triangle.worldSpace[2] * gammaStartBary;
	Vector4 worldSpaceXDelta = _triangle.worldSpace[0] * alphaXDelta + _triangle.worldSpace[1] * betaXDelta + _triangle.worldSpace[2] * gammaXDelta;
	Vector4 worldSpaceYDelta = _triangle.worldSpace[0] * alphaYDelta + _triangle.worldSpace[1] * betaYDelta + _triangle.worldSpace[2] * gammaYDelta;
	SSEVector worldSpaceStartSSE(worldSpaceStart.x, worldSpaceStart.y, worldSpaceStart.z, 0);
	SSEVector worldSpaceXDeltaSSE(worldSpaceXDelta.x, worldSpaceXDelta.y, worldSpaceXDelta.z, 0);
	SSEVector worldSpaceYDeltaSSE(worldSpaceYDelta.x, worldSpaceYDelta.y, worldSpaceYDelta.z, 0);

	// Get the ambient, diffuse and specular values as sse vectors
	SSEVector ambient(_triangle.ambient.x, _triangle.ambient.y, _triangle.ambient.z, 0);
	SSEVector diffuse(_triangle.diffuse.x, _triangle.diffuse.y, _triangle.diffuse.z, 0);
	SSEVector specular(_triangle.specular.x, _triangle.specular.y, _triangle.specular.z, 0);

	////////////////////////// TEXTURE COORD intepolation /////////////////////
	// Calculate the differences so that we can get a value that we can increment the pixel buffer for the texture.
	Vector4 textureCoordStart = (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaStartBary + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaStartBary + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaStartBary;
	Vector4 textureCoordXDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaXDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaXDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaXDelta;
	Vector4 textureCoordYDelta =  (_triangle.vertexTextureCoord[0]/_triangle.originalZ[0]) * alphaYDelta + (_triangle.vertexTextureCoord[1]/_triangle.originalZ[1]) * betaYDelta + (_triangle.vertexTextureCoord[2]/_triangle.originalZ[2]) * gammaYDelta;

	float oneOverZStart = ((1/_triangle.originalZ[0]) * alphaStartBary + (1/_triangle.originalZ[1]) * betaStartBary + (1/_triangle.originalZ[2]) * gammaStartBary);
	float oneOverZXDelta = ((1/_triangle.originalZ[0]) * alphaXDelta + (1/_triangle.originalZ[1]) * betaXDelta + (1/_triangle.originalZ[2]) * gammaXDelta);
	float oneOverZYDelta = ((1/_triangle.originalZ[0]) * alphaYDelta + (1/_triangle.originalZ[1]) * betaYDelta + (1/_triangle.originalZ[2]) * gammaYDelta);

	// Get the texture data pixels
	unsigned int* textureData = (unsigned int*)_triangle.texture->GetPixels();
	int textureWidth = _triangle.texture->GetWidth();
	int textureHeight = _triangle.texture->GetHeight();
	
	// Put the u/z, v/z and 1/z values into one SSE 
	SSEVector textureCoordStartSSE(textureCoordStart.x, textureCoordStart.y, oneOverZStart, 0);
	SSEVector textureCoordXDeltaSSE(textureCoordXDelta.x, textureCoordXDelta.y, oneOverZXDelta, 0);
	SSEVector textureCoordYDeltaSSE(textureCoordYDelta.x, textureCoordYDelta.y, oneOverZYDelta, 0);

	// Get the pixels pointer to draw to
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += ymin * width;

	// Get the zbuffer pointer to access
	float* zBuffer = GetZBuffer();
	zBuffer += ymin * width;

	// Loop through the bounding box and evaluate each block.
	for (int y = ymin; y < ymax; y += blockSize) {          
		for (int x = xmin; x < xmax; x += blockSize) {
			// Calculate the positions of the corners of this block
			int top = y << 4;
			int bottom = (y + blockSize - 1) << 4;
			int left = x << 4;
			int right = (x + blockSize - 1) << 4;

			// Get the half space values for each of these corners
			bool alpha0 = start1 + deltaX1 * top - deltaY1 * left > 0; // The top left corner
			bool alpha1 = start1 + deltaX1 * top - deltaY1 * right > 0; // The top right corner
			bool alpha2 = start1 + deltaX1 * bottom - deltaY1 * left > 0; // The bottom left corner
			bool alpha3 = start1 + deltaX1 * bottom - deltaY1 * right > 0; // The bottom right corner
			int alphaMask = (alpha0 << 0) | (alpha1 << 1) | (alpha2 << 2) | (alpha3 << 3); // Create a mask to make it easier to compare.

			bool beta0 = start2 + deltaX2 * top - deltaY2 * left > 0; // The top left corner
			bool beta1 = start2 + deltaX2 * top - deltaY2 * right > 0; // The top right corner
			bool beta2 = start2 + deltaX2 * bottom - deltaY2 * left > 0; // The bottom left corner
			bool beta3 = start2 + deltaX2 * bottom - deltaY2 * right > 0; // The bottom right corner
			int betaMask = (beta0 << 0) | (beta1 << 1) | (beta2 << 2) | (beta3 << 3); // Create a mask to make it easier to compare.

			bool gamma0 = start3 + deltaX3 * top - deltaY3 * left > 0; // The top left corner
			bool gamma1 = start3 + deltaX3 * top - deltaY3 * right > 0; // The top right corner
			bool gamma2 = start3 + deltaX3 * bottom - deltaY3 * left > 0; // The bottom left corner
			bool gamma3 = start3 + deltaX3 * bottom - deltaY3 * right > 0; // The bottom right corner
			int gammaMask = (gamma0 << 0) | (gamma1 << 1) | (gamma2 << 2) | (gamma3 << 3); // Create a mask to make it easier to compare.

			// If all of these masks are empty then this block is completley empty. So we can just skip to the next loop
			if (alphaMask == 0 || betaMask == 0 || gammaMask == 0) continue;

			// If this block is over the edge of the screen then we need to clip it.
			int highX = x + blockSize; // How high to loop the x to.
			if (highX > width)
				highX = width;
			int highY = y + blockSize; // How high to loop the y to.
			if (highY > height)
				highY = height;

			// Calculate the start of our interpolants # colorYBary = normalStart + normalXDelta * x + normalYDelta * y; (in other words)
			SSEVector normalYBarySSE = normalStartSSE;
			SSEVector temp = normalXDeltaSSE;
			temp *= (float)x;
			normalYBarySSE += temp;
			temp = normalYDeltaSSE;
			temp *= (float)y;
			normalYBarySSE += temp;

			// Calculate the start of our interpolants # cameraSpaceYBary = cameraSpaceStart + cameraSpaceXDelta * x + cameraSpaceYDelta * y; (in other words)
			SSEVector cameraSpaceYBarySSE = cameraSpaceStartSSE;
			temp = cameraSpaceXDeltaSSE;
			temp *= (float)x;
			cameraSpaceYBarySSE += temp;
			temp = cameraSpaceYDeltaSSE;
			temp *= (float)y;
			cameraSpaceYBarySSE += temp;

			// Calculate the start of our interpolants # worldSpaceYBary = worldSpaceStart + worldSpaceXDelta * x + worldSpaceYDelta * y; (in other words)
			SSEVector worldSpaceYBarySSE = worldSpaceStartSSE;
			temp = worldSpaceXDeltaSSE;
			temp *= (float)x;
			worldSpaceYBarySSE += temp;
			temp = worldSpaceYDeltaSSE;
			temp *= (float)y;
			worldSpaceYBarySSE += temp;

			// Calculate the start of our texture coordinates.
			SSEVector textureCoordYBarySSE = textureCoordStartSSE;
			temp = textureCoordXDeltaSSE;
			temp *= (float)x;
			textureCoordYBarySSE += temp;
			temp = textureCoordYDeltaSSE;
			temp *= (float)y;
			textureCoordYBarySSE += temp;

			// If all of these masks are 0xF then the block is completley inside the triangle, we can simply fill the whole lot.
			unsigned int* tempBuffer = pixels;
			float* tempZBuffer = zBuffer;
			if (alphaMask == 0xF && betaMask == 0xF && gammaMask == 0xF) {
				for (int i = y; i < highY; ++i) {
					// Set the interpolated colors back to the start of a line.
					SSEVector normalXBarySSE = normalYBarySSE;
					SSEVector worldSpaceXBarySSE = worldSpaceYBarySSE;
					SSEVector cameraSpaceXBarySSE = cameraSpaceYBarySSE;
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						
						// Put the pixel if the z coord is bigger than the one in the z buffer
						if (normalXBarySSE.w < tempZBuffer[blockX]) {							
							// Calculate the color we should show.
							SSEVector color;
							
							// Get a normalized version of the normal
							SSEVector normal = normalXBarySSE;
							normal.Normalize();

							// Ambient light
							_triangle.info->GetAmbientLightOnPoint(color);

							// Directional light
							for (unsigned int i = 0; i < _triangle.info->GetDirectionalLights().size(); ++i) {
								_triangle.info->GetDirectionalLightOnPoint(cameraSpaceXBarySSE, 
																	 normal,
																	 i,
																	 diffuse,
																	 specular,
																	 color);
							}

							// Point lights
							for (unsigned int i = 0; i < _triangle.info->GetPointLights().size(); ++i) {
								_triangle.info->GetPointLightOnPoint(cameraSpaceXBarySSE,
																worldSpaceXBarySSE,
																normal,
																i,
																diffuse,
																specular,
																color);
							}

							// Clamp the color and mutliply by 255
							color.Clamp();

							// Calculate the actual uv coordinates (perspective correct)
							float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
							SSEVector tempUV = textureCoordXBarySSE;
							tempUV *= z; // Correct the uv coordinates								
								
							// Get the uv coordinates
							tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
							__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
							// Sample the colors
							int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

							// Mix the texture color with the lighting color
							SSEVector finalColor(((unsigned char*)&texColor)[2] / (float)255, 
											  ((unsigned char*)&texColor)[1] / (float)255, 
											  ((unsigned char*)&texColor)[0] / (float)255,
											  0);
							finalColor *= color;
							finalColor *= SSEVector(255);

							// Get the color
							__m128i pixel = _mm_cvtps_epi32(finalColor.data);

							// Set the pixel
							unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
							tempBuffer[blockX] = tempPixel;

							// Mark this on the z buffer
							tempZBuffer[blockX] = normalXBarySSE.w;
						}

						// Increment the color.
						normalXBarySSE += normalXDeltaSSE;
						cameraSpaceXBarySSE += cameraSpaceXDeltaSSE;
						worldSpaceXBarySSE += worldSpaceXDeltaSSE;
						textureCoordXBarySSE += textureCoordXDeltaSSE;
					}
					tempBuffer += width;
					tempZBuffer += width;

					// Increment the color.
					normalYBarySSE += normalYDeltaSSE;
					cameraSpaceYBarySSE += cameraSpaceYDeltaSSE;
					worldSpaceYBarySSE += worldSpaceYDeltaSSE;
					textureCoordYBarySSE += textureCoordYDeltaSSE;
				}
			} else {
				// The block is only partially covered so we need to do checking on each pixel in this block.
				// Get the half space for the top of this block.
				int currentY1 = start1 + deltaX1 * top - deltaY1 * left;
				int currentY2 = start2 + deltaX2 * top - deltaY2 * left;
				int currentY3 = start3 + deltaX3 * top - deltaY3 * left;
				for (int blockY = y; blockY < highY; ++blockY) {
					// Reset the currentX for this scanline
					int currentX1 = currentY1;
					int currentX2 = currentY2;
					int currentX3 = currentY3;

					// Set the interpolated colors back to the start of a line.
					SSEVector normalXBarySSE = normalYBarySSE;
					SSEVector worldSpaceXBarySSE = worldSpaceYBarySSE;
					SSEVector cameraSpaceXBarySSE = cameraSpaceYBarySSE;
					SSEVector textureCoordXBarySSE = textureCoordYBarySSE;

					for (int blockX = x; blockX < highX; ++blockX) {
						if (currentX1 > 0 && currentX2 > 0 && currentX3 > 0) {
							// Put the pixel if the z coord is bigger than the one in the z buffer
							if (normalXBarySSE.w < tempZBuffer[blockX]) {
								// Calculate the color we should show.
								SSEVector color;
							
								// Get a normalized version of the normal
								SSEVector normal = normalXBarySSE;
								normal.Normalize();

								// Ambient light
								_triangle.info->GetAmbientLightOnPoint(color);

								// Directional light
								for (unsigned int i = 0; i < _triangle.info->GetDirectionalLights().size(); ++i) {
									_triangle.info->GetDirectionalLightOnPoint(cameraSpaceXBarySSE, 
																		 normal,
																		 i,
																		 diffuse,
																		 specular,
																		 color);
								}

								// Point lights
								for (unsigned int i = 0; i < _triangle.info->GetPointLights().size(); ++i) {
									_triangle.info->GetPointLightOnPoint(cameraSpaceXBarySSE,
																	worldSpaceXBarySSE,
																	normal,
																	i,
																	diffuse,
																	specular,
																	color);
								}

								// Clamp the color and mutliply by 255
								color.Clamp();

								// Calculate the actual uv coordinates (perspective correct)
								float z = 1 / textureCoordXBarySSE.z; // Calculate the the z coordinate.
								SSEVector tempUV = textureCoordXBarySSE;
								tempUV *= z; // Correct the uv coordinates								
								
								// Get the uv coordinates
								tempUV *= SSEVector((float)textureWidth, (float)textureHeight, 0, 0);
								__m128i uvs = _mm_cvttps_epi32(tempUV.data);
								
								// Sample the colors
								int texColor = _triangle.texture->Sample(uvs.m128i_i32[3], uvs.m128i_i32[2]);		

								// Mix the texture color with the lighting color
								SSEVector finalColor(((unsigned char*)&texColor)[2] / (float)255, 
												  ((unsigned char*)&texColor)[1] / (float)255, 
												  ((unsigned char*)&texColor)[0] / (float)255,
												  0);
								finalColor *= color;
								finalColor *= SSEVector(255);

								// Get the color
								__m128i pixel = _mm_cvtps_epi32(finalColor.data);

								// Set the pixel
								unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;
								tempBuffer[blockX] = tempPixel;

								// Mark this on the z buffer
								tempZBuffer[blockX] = normalXBarySSE.w;
							}
						}

						// Increment the color.
						normalXBarySSE += normalXDeltaSSE;
						cameraSpaceXBarySSE += cameraSpaceXDeltaSSE;
						worldSpaceXBarySSE += worldSpaceXDeltaSSE;
						textureCoordXBarySSE += textureCoordXDeltaSSE;

						// Increment the half space
						currentX1 -= fixedDeltaY1;
						currentX2 -= fixedDeltaY2;
						currentX3 -= fixedDeltaY3;
					}
					// Increment the half space
					currentY1 += fixedDeltaX1;
					currentY2 += fixedDeltaX2;
					currentY3 += fixedDeltaX3;

					// Increment the color.
					normalYBarySSE += normalYDeltaSSE;
					cameraSpaceYBarySSE += cameraSpaceYDeltaSSE;
					worldSpaceYBarySSE += worldSpaceYDeltaSSE;
					textureCoordYBarySSE += textureCoordYDeltaSSE;

					// Increment the buffer
					tempBuffer += width;
					tempZBuffer += width;
				}
			}
		}

		// Increase the pixels by blocksize * stride to get to the next block.
		pixels += blockSize * width;
		zBuffer += blockSize * width;
	}
}

// Draw a triangle using the scan line method.
void IRenderTarget::DrawInterpolatedTriangleScanLines(DrawBuffer::Triangle& _triangle) {
	// Represents an edge.
	struct Edge {
		int v1, v2;

		Edge(int _i, int _j, DrawBuffer::Triangle& _triangle) {
			if (_triangle.vertices[_j].y < _triangle.vertices[_i].y) {
				v1 = _j;
				v2 = _i;
			} else {
				v1 = _i;
				v2 = _j;
			}
		}

		int LengthY(const DrawBuffer::Triangle& _triangle) const {
			return (int)(_triangle.vertices[v2].y - _triangle.vertices[v1].y);
		}
	};

	// The edges
	Edge edges[3] = { Edge(0, 1, _triangle), Edge(1, 2, _triangle), Edge(2, 0, _triangle) };

	// Find the longest edge
	int maxLength = 0;
	int longestEdge = 0;
	for (int i = 0; i < 3; ++i) {
		int length = edges[i].LengthY(_triangle);
		if (length > maxLength) {
			maxLength = length;
			longestEdge = i;
		}
	}

	// We know the longest edge, swap the edges around
	Edge temp = edges[longestEdge];
	edges[longestEdge] = edges[0];
	edges[0] = temp;

	// Put the others in order
	if (_triangle.vertices[edges[1].v2].y > _triangle.vertices[edges[2].v2].y) {
		Edge temp = edges[1];
		edges[1] = edges[2];
		edges[2] = temp;
	}

	// Times the colors by 255 so that they are faster to convert
	for (int i = 0; i < 3; ++i) {
		_triangle.vertexColors[i] *= 255;
	}

	// Draw the first half of the triangle
	// Calculate the delta of the long side.
	int yStart = (int)floor(_triangle.vertices[edges[0].v1].y); // The long the sides y start value.
	int yEnd = (int)floor(_triangle.vertices[edges[0].v2].y);
	float fYStart = _triangle.vertices[edges[0].v1].y;
	float fYEnd = _triangle.vertices[edges[0].v2].y;

	// Deltas for the long side.
	float longDelta = ((_triangle.vertices[edges[0].v2].x - _triangle.vertices[edges[0].v1].x) /
						(_triangle.vertices[edges[0].v2].y - _triangle.vertices[edges[0].v1].y));
	float longInter = _triangle.vertices[edges[0].v1].x; // The long sides interpolated value.

	// Get a pointer to the pixel data
	unsigned int* pixels = (unsigned int*)GetPixels();
	int width = GetWidth();
	int height = GetHeight();
	pixels += width * yStart;

	// Color interpolant
	Vector4 longColorDelta = (_triangle.vertexColors[edges[0].v2] - _triangle.vertexColors[edges[0].v1]) / (fYEnd - fYStart);
	SSEVector longColorInterSSE(_triangle.vertexColors[edges[0].v1].x, _triangle.vertexColors[edges[0].v1].y, _triangle.vertexColors[edges[0].v1].z, 0);
	SSEVector longColorDeltaSSE(longColorDelta.x, longColorDelta.y, longColorDelta.z, 0);

	// Check the shorter edge whether or not it has a 0 length.
	if (edges[1].LengthY(_triangle) > 0) {
		// Calculate the end point.
		fYEnd = _triangle.vertices[edges[1].v2].y;
		yEnd = (int)floor(fYEnd); // What scanline to end on.

		// Calculate the delta of the shorter side.
		float shortDelta = ((_triangle.vertices[edges[1].v2].x - _triangle.vertices[edges[1].v1].x) /
								(fYEnd - fYStart));
		float shortInter = _triangle.vertices[edges[1].v1].x; // The short sides interpolated value.

		// Interpolants for the colors
		Vector4 shortColorDelta = (_triangle.vertexColors[edges[1].v2] - _triangle.vertexColors[edges[1].v1]) / (fYEnd - fYStart);
		SSEVector shortColorInterSSE(_triangle.vertexColors[edges[1].v1].x, _triangle.vertexColors[edges[1].v1].y, _triangle.vertexColors[edges[1].v1].z, 0);
		SSEVector shortColorDeltaSSE(shortColorDelta.x, shortColorDelta.y, shortColorDelta.z, 0);

		// Now draw the triangle
		for (int y = yStart; y < yEnd; ++y) {
			
			if (y >= 0 && y < height) {

				// Get the end and start of the line
				int xStart;
				int xEnd;
				SSEVector xStartColor;
				SSEVector xEndColor;
				if (longInter > shortInter) {
					xEnd = (int)floor(longInter) + 1;
					xStart = (int)floor(shortInter);

					// Colors
					xStartColor = shortColorInterSSE;
					xEndColor = longColorInterSSE;
				} else {
					xEnd = (int)floor(shortInter) + 1;
					xStart = (int)floor(longInter);

					// Colors
					xStartColor = longColorInterSSE;
					xEndColor = shortColorInterSSE;
				}

				// Calculate the interpolation values for this scan line.

				// Color
				SSEVector colorXDiffSSE = xEndColor;
				colorXDiffSSE -= xStartColor;
				SSEVector colorXDeltaSSE = colorXDiffSSE;
				colorXDeltaSSE /= (float)(xEnd - xStart);
				SSEVector color = xStartColor;

				// Clip the x
				if (xStart < 0) {
					// Recalculate the correct interpolated values

					// Color
					SSEVector colorDifference = colorXDeltaSSE;
					colorDifference *= (float)(0 - xStart);
					color += colorDifference;
					
					xStart = 0;

				}
				if (xEnd >= width)
					xEnd = width;

				// Draw a line between the two points
				for (int x = xStart; x < xEnd; ++x) {
					// Calculate the color.
					__m128i pixel = _mm_cvttps_epi32(color.data);
					unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;

					pixels[x] = tempPixel;

					// Increment the color
					color += colorXDeltaSSE;
				}
			}

			// Increment the interpolated values.
			longInter += longDelta;
			shortInter += shortDelta;

			longColorInterSSE += longColorDeltaSSE;
			shortColorInterSSE += shortColorDeltaSSE;

			// Increment y
			pixels += width;
		}

		// Set the start to the end of this half.
		yStart = yEnd;
		fYStart = fYEnd;
	}

	// Draw the other half of the triangle.
	if (edges[2].LengthY(_triangle) > 0) {
		fYEnd = _triangle.vertices[edges[2].v2].y;
		yEnd = (int)floor(fYEnd); // What scanline to end on.

		// Calculate the delta of the shorter side.
		float shortDelta = ((_triangle.vertices[edges[2].v2].x - _triangle.vertices[edges[2].v1].x) /
								(fYEnd - fYStart));
		float shortInter = _triangle.vertices[edges[2].v1].x; // The short sides interpolated value.

		// Interpolants for the colors
		Vector4 shortColorDelta = (_triangle.vertexColors[edges[2].v2] - _triangle.vertexColors[edges[2].v1]) / (fYEnd - fYStart);
		SSEVector shortColorInterSSE(_triangle.vertexColors[edges[2].v1].x, _triangle.vertexColors[edges[2].v1].y, _triangle.vertexColors[edges[2].v1].z, 0);
		SSEVector shortColorDeltaSSE(shortColorDelta.x, shortColorDelta.y, shortColorDelta.z, 0);

		// Now draw the triangle
		for (int y = yStart; y < yEnd; ++y) {
			
			if (y >= 0 && y < height) {

				// Get the end and start of the line
				int xStart;
				int xEnd;
				SSEVector xStartColor;
				SSEVector xEndColor;
				if (longInter > shortInter) {
					xEnd = (int)floor(longInter) + 1;
					xStart = (int)floor(shortInter);

					// Colors
					xStartColor = shortColorInterSSE;
					xEndColor = longColorInterSSE;
				} else {
					xEnd = (int)floor(shortInter) + 1;
					xStart = (int)floor(longInter);

					// Colors
					xStartColor = longColorInterSSE;
					xEndColor = shortColorInterSSE;
				}

				// Calculate the interpolation values for this scan line.

				// Color
				SSEVector colorXDiffSSE = xEndColor;
				colorXDiffSSE -= xStartColor;
				SSEVector colorXDeltaSSE = colorXDiffSSE;
				colorXDeltaSSE /= (float)(xEnd - xStart);
				SSEVector color = xStartColor;

				// Clip the x
				if (xStart < 0) {
					// Recalculate the correct interpolated values

					// Color
					SSEVector colorDifference = colorXDeltaSSE;
					colorDifference *= (float)(0 - xStart);
					color += colorDifference;
					
					xStart = 0;

				}
				if (xEnd >= width)
					xEnd = width;

				// Draw a line between the two points
				for (int x = xStart; x < xEnd; ++x) {
					// Calculate the color.
					__m128i pixel = _mm_cvttps_epi32(color.data);
					unsigned int tempPixel = pixel.m128i_i32[1] << 0 | pixel.m128i_i32[2] << 8 | pixel.m128i_i32[3] << 16;

					pixels[x] = tempPixel;

					// Increment the color
					color += colorXDeltaSSE;
				}
			}

			// Increment the interpolated values.
			longInter += longDelta;
			shortInter += shortDelta;

			longColorInterSSE += longColorDeltaSSE;
			shortColorInterSSE += shortColorDeltaSSE;

			// Increment y
			pixels += width;
		}
	}
	
	// Draw the triangle (both halfs of it).
	//Helpers::DrawSpans(edges[0], edges[1], _triangle, (unsigned int*)GetPixels(), GetZBuffer(), GetWidth(), GetHeight(), GetWidth(), GetHeight());
	//Helpers::DrawSpans(edges[0], edges[2], _triangle, (unsigned int*)GetPixels(), GetZBuffer(), GetWidth(), GetHeight(), GetWidth(), GetHeight());
}

/* Draw text to the screen */
void IRenderTarget::PrintText(const std::string& _text, int _x, int _y, dkr::Color _color) {
}

void IRenderTarget::DrawSolidTriangles(int _bufferId) {
	// Loop through all the triangles and draw them.
	DrawBuffer& drawBuffer = memory->GetTempBuffer(_bufferId);
	const VertexBuffer& buffer = memory->GetBuffer(_bufferId);

	// Resolve indices to vertices
	for (unsigned int i = 0; i < drawBuffer.GetTriangles().size(); ++i) {
		for (int j = 0; j < 3; ++j) {
			drawBuffer.GetTriangles()[i].vertices[j] = drawBuffer.GetPositions()[drawBuffer.GetTriangles()[i].vertexIndices[j]];
		}
	}

	// Check what type of fill is needed
	if (memory->GetBuffer(_bufferId).GetDrawMode() == VertexBuffer::DRAW_LINES) {
		DrawLines(_bufferId);
	} else {
		// Flat shading or smooth?
		if (memory->GetBuffer(_bufferId).GetShading() == VertexBuffer::SHADE_FLAT) {
			if (memory->GetBuffer(_bufferId).GetTextureMode() == VertexBuffer::TEXTURE_BASIC && memory->GetBuffer(_bufferId).GetTexture() != NULL) {
				for (unsigned int i =0; i < drawBuffer.GetTriangles().size(); ++i) {
					DrawInterpolatedTriangleTextured(drawBuffer.GetTriangles()[i]);
				}
			} else {
				// Sort the faces. (Since we don't have a z-buffer for flat shading)
				drawBuffer.SortTriangles();
				for (unsigned int i = 0; i < drawBuffer.GetTriangles().size(); ++i) {
					DrawSolidTriangle(drawBuffer.GetTriangles()[i]);
				}
			}
		} else if (memory->GetBuffer(_bufferId).GetShading() == VertexBuffer::SHADE_SMOOTH) {
			// Textured or non textured?
			if (memory->GetBuffer(_bufferId).GetTextureMode() == VertexBuffer::TEXTURE_NONE) {
				for (unsigned int i = 0; i < drawBuffer.GetTriangles().size(); ++i) {
					DrawInterpolatedTriangle(drawBuffer.GetTriangles()[i]);
				}
			} else if (memory->GetBuffer(_bufferId).GetTextureMode() == VertexBuffer::TEXTURE_BASIC && memory->GetBuffer(_bufferId).GetTexture() != NULL) {
				for (unsigned int i = 0; i < drawBuffer.GetTriangles().size(); ++i) {
					DrawInterpolatedTriangleTexturedLighted(drawBuffer.GetTriangles()[i]);
				}
			}
		} else if (memory->GetBuffer(_bufferId).GetShading() == VertexBuffer::SHADE_PHONG) {
			// Textured or non textured?
			if (memory->GetBuffer(_bufferId).GetTextureMode() == VertexBuffer::TEXTURE_NONE) {
				for (unsigned int i = 0; i < drawBuffer.GetTriangles().size(); ++i) {
					DrawInterpolatedTrianglePhong(drawBuffer.GetTriangles()[i]);
				}
			} else {
				for (unsigned int i = 0; i < drawBuffer.GetTriangles().size(); ++i) {
					DrawInterpolatedTrianglePhongTextured(drawBuffer.GetTriangles()[i]);
				}
			}
		}
	}
}

int IRenderTarget::GetWidth() {
	return width;
}
int IRenderTarget::GetHeight() {
	return height;
}
int IRenderTarget::GetDepth() {
	return depth;
}