#include "Vector4.hpp"
using namespace dkr;

#include <cmath>

Vector4::Vector4() {
	for (int i = 0; i < 4; ++i) {
		data[i] = 0;
	}
}
Vector4::Vector4(float _v) {
	for (int i = 0; i < 4; ++i) {
		data[i] = _v;
	}
}
Vector4::Vector4(float _x, float _y, float _z, float _w) {
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

/* Operators */
Vector4 Vector4::operator* (const Vector4& _vec) const {
	Vector4 temp;
	for (int i = 0; i < 4; ++i) {
		temp[i] = (*this)[i] * _vec[i];
	}
	return temp;
}
Vector4& Vector4::operator*= (const Vector4& _vec) {
	for (int i = 0; i < 4; ++i) {
		(*this)[i] *= _vec[i];
	}
	return (*this);
}
Vector4 Vector4::operator+ (const Vector4& _vec) const {
	Vector4 temp;
	for (int i = 0; i < 4; ++i) {
		temp[i] = (*this)[i] + _vec[i];
	}
	return temp;
}
Vector4& Vector4::operator+= (const Vector4& _vec) {
	for (int i = 0; i < 4; ++i) {
		(*this)[i] += _vec[i];
	}
	return (*this);
}
Vector4 Vector4::operator- (const Vector4 _vec) const {
	Vector4 temp;
	for (int i = 0; i < 4; ++i) {
		temp[i] = (*this)[i] - _vec[i];
	}
	return temp;
}
Vector4& Vector4::operator-= (const Vector4 _vec) {
	for (int i = 0; i < 4; ++i) {
		(*this)[i] -= _vec[i];
	}
	return (*this);
}

/* Scalar operators */
Vector4 Vector4::operator* (float _f) const {
	Vector4 temp;
	temp.x = x * _f;
	temp.y = y * _f;
	temp.z = z * _f;
	temp.w = w * _f;
	return temp;
}

Vector4 Vector4::operator/ (float _f) const {
	Vector4 temp;
	temp.x = x/_f;
	temp.y = y/_f;
	temp.z = z/_f;
	temp.w = w/_f;
	return temp;
}

Vector4& Vector4::operator/= (float _f) {
	x /= _f;
	y /= _f;
	z /= _f;
	w /= _f;
	return *this;
}

/* Dot product */
float Vector4::Dot(const Vector4& _vec) const {
	float total = 0;
	for (int i = 0; i < 3; ++i) {
		total += (*this)[i] * _vec[i];
	}
	return total;
}

/* Cross product */
Vector4 Vector4::Cross(const Vector4& _vec) const {
	Vector4 temp;
	temp.x = y * _vec.z - z * _vec.y;
	temp.y = z * _vec.x - x * _vec.z;
	temp.z = x * _vec.y - y * _vec.x;
	return temp;
}

float Vector4::Length() {
	return sqrt(x*x + y*y + z*z);
}

void Vector4::Normalize() {
	float length = Length();
	x /= length;
	y /= length;
	z /= length;
}

void Vector4::Clamp() {
	if (x > 1)
		x = 1;
	if (x < 0)
		x = 0;
	if (y > 1)
		y = 1;
	if (y < 0)
		y = 0;
	if (z > 1)
		z = 1;
	if (z < 0)
		z = 0;
}

Vector4 Vector4::Abs() {
	return Vector4(abs(x), abs(y), abs(z), abs(w));
}

/* Accesing data */
float& Vector4::operator[] (int _i) {
	return data[_i];
}
const float& Vector4::operator[] (int _i) const {
	return data[_i];
}