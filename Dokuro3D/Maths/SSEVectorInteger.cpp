#include "SSEVectorInteger.hpp"
using namespace dkr;

SSEVectorInteger::~SSEVectorInteger() {
}

SSEVectorInteger::SSEVectorInteger() {
	data = _mm_set_ps1(0);
}

SSEVectorInteger::SSEVectorInteger(__m128& _data) {
	data = _data;
}

SSEVectorInteger::SSEVectorInteger(int _v) {
	data = _mm_set_ps1((float)_v);
}

SSEVectorInteger::SSEVectorInteger(int _x, int _y, int _z, int _w) {
	data = _mm_set_ps((float)_x, (float)_y, (float)_z, (float)_w);
}

SSEVectorInteger::SSEVectorInteger(const SSEVectorInteger& _vec) {
	data = _vec.data;
}

/* --------------------------------- OPERATORS -------------------------------- */
void SSEVectorInteger::operator*= (const SSEVectorInteger& _vec) {
	data = _mm_mul_ps(data, _vec.data);
}
void SSEVectorInteger::operator-= (const SSEVectorInteger& _vec) {
	data = _mm_sub_ps(data, _vec.data);
}
void SSEVectorInteger::operator+= (const SSEVectorInteger& _vec) {
	data = _mm_add_ps(data, _vec.data);
}
void SSEVectorInteger::operator/= (const SSEVectorInteger& _vec) {
	data = _mm_div_ps(data, _vec.data);
}

void SSEVectorInteger::operator*= (int _f) {
	data = _mm_mul_ps(data, _mm_set_ps1((float)_f));
}