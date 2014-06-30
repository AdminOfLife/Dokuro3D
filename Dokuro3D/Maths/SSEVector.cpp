#include "SSEVector.hpp"
using namespace dkr;

SSEVector::~SSEVector() {
}

SSEVector::SSEVector() {
	data = _mm_set_ps1(0);
}

SSEVector::SSEVector(__m128& _data) {
	data = _data;
}

SSEVector::SSEVector(float _v) {
	data = _mm_set_ps1(_v);
}

SSEVector::SSEVector(float _x, float _y, float _z, float _w) {
	data = _mm_set_ps(_x, _y, _z, _w);
}

SSEVector::SSEVector(const SSEVector& _vec) {
	data = _vec.data;
}

/* --------------------------------- OPERATORS -------------------------------- */
void SSEVector::operator*= (const SSEVector& _vec) {
	data = _mm_mul_ps(data, _vec.data);
}
void SSEVector::operator-= (const SSEVector& _vec) {
	data = _mm_sub_ps(data, _vec.data);
}
void SSEVector::operator+= (const SSEVector& _vec) {
	data = _mm_add_ps(data, _vec.data);
}
void SSEVector::operator/= (const SSEVector& _vec) {
	data = _mm_div_ps(data, _vec.data);
}

void SSEVector::operator*= (float _f) {
	data = _mm_mul_ps(data, _mm_set_ps1(_f));
}

void SSEVector::operator/= (float _f) {
	data = _mm_div_ps(data, _mm_set_ps1(_f));
}

/* ---------------------------------- CLAMP ----------------------------------- */
void SSEVector::Clamp() {
	data = _mm_min_ps(data, _mm_set_ps1(1));
	data = _mm_max_ps(data, _mm_set_ps1(0));
}

/* -------------------------------- LENGTH ------------------------------------ */
float SSEVector::Length() {
	__m128 square = _mm_mul_ps(data, data);
	return _mm_sqrt_ss(_mm_set_ps1(square.m128_f32[3] + square.m128_f32[2] + square.m128_f32[1])).m128_f32[0];
}

/* --------------------------------- NORMALIZE -------------------------------- */
void SSEVector::Normalize() {
	__m128 square = _mm_mul_ps(data, data);
	float rSquare = _mm_rsqrt_ss(_mm_set_ps1(square.m128_f32[3] + square.m128_f32[2] + square.m128_f32[1])).m128_f32[0];

	data = _mm_mul_ps(data, _mm_set_ps1(rSquare));
}

/* -------------------------- DOT PRODUCT ------------------------------------- */
float SSEVector::Dot(const SSEVector& _vec) {
	return (x * _vec.x + y * _vec.y + z * _vec.z);
}