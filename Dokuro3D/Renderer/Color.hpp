#ifndef COLOR_DEF
#define COLOR_DEF

#include "../Maths/Vector4.hpp"

namespace dkr {
	/* A struct that defines a color */
	struct Color {
		Color(const Vector4& _vector);
		Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a);
		Color(float _r, float _g, float _b, float _a);
		Color(unsigned int _color);

		/* The color */
		union {
			unsigned int color;
			unsigned char colors[4];
		};
	};
}

#endif