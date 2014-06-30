#include "Color.hpp"
using namespace dkr;

Color::Color(const Vector4& _vector) {
	colors[0] = (unsigned char)((float)255*_vector[0]);
	colors[1] = (unsigned char)((float)255*_vector[1]);
	colors[2] = (unsigned char)((float)255*_vector[2]);
	colors[3] = (unsigned char)((float)255*_vector[3]);
}

Color::Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a) {
	colors[0] = _r;
	colors[1] = _g;
	colors[2] = _b;
	colors[3] = _a;
}

Color::Color(float _r, float _g, float _b, float _a) {
	colors[0] = (unsigned char)((float)255*_r);
	colors[1] = (unsigned char)((float)255*_g);
	colors[2] = (unsigned char)((float)255*_b);
	colors[3] = (unsigned char)((float)255*_a);
}

Color::Color(unsigned int _color) {
	color = _color;
}