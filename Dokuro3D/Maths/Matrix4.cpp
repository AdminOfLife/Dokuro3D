#include "Matrix4.hpp"
#include <cmath>
using namespace dkr;

/* Static matrices */
Matrix4 Matrix4::Identity = Matrix4::CreateIdentity();

Matrix4::Matrix4() {
	for (int i = 0; i < 16; ++i) {
		data[i] = 0;
	}
}

/* Create matrices */
Matrix4 Matrix4::CreateIdentity() {
	Matrix4 temp;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (i == j)
				temp(i, j) = 1;
			else
				temp(i, j) = 0;
		}
	}
	return temp;
}

Matrix4 Matrix4::CreateTranslation(float _x, float _y, float _z) {
	Matrix4 m;
	m(0, 0) = 1; m(1, 0) = 0; m(2, 0) = 0; m(3, 0) = _x;
	m(0, 1) = 0; m(1, 1) = 1; m(2, 1) = 0; m(3, 1) = _y;
	m(0, 2) = 0; m(1, 2) = 0; m(2, 2) = 1; m(3, 2) = _z;
	m(0, 3) = 0; m(1, 3) = 0; m(2, 3) = 0; m(3, 3) = 1;
	return m;
}
Matrix4 Matrix4::CreateRotation(float _x, float _y, float _z) {
	Matrix4 x;
	x(0, 0) = 1; x(1, 0) = 0;	    x(2, 0) = 0;	    x(3, 0) = 0;
	x(0, 1) = 0; x(1, 1) = cos(_x); x(2, 1) = -sin(_x); x(3, 1) = 0;
	x(0, 2) = 0; x(1, 2) = sin(_x); x(2, 2) = cos(_x);  x(3, 2) = 0;
	x(0, 3) = 0; x(1, 3) = 0;       x(2, 3) = 0;        x(3, 3) = 1;
			
	Matrix4 y;
	y(0, 0) = cos(_y);  y(1, 0) = 0; y(2, 0) = sin(_y); y(3, 0) = 0;
	y(0, 1) = 0;        y(1, 1) = 1; y(2, 1) = 0;       y(3, 1) = 0;
	y(0, 2) = -sin(_y); y(1, 2) = 0; y(2, 2) = cos(_y); y(3, 2) = 0;
	y(0, 3) = 0;        y(1, 3) = 0; y(2, 3) = 0;       y(3, 3) = 1;
			
	Matrix4 z;
	z(0, 0) = cos(_z); z(1, 0) = -sin(_z); z(2, 0) = 0; z(3, 0) = 0;
	z(0, 1) = sin(_z); z(1, 1) = cos(_z);  z(2, 1) = 0; z(3, 1) = 0;
	z(0, 2) = 0;	   z(1, 2) = 0;		   z(2, 2) = 1; z(3, 2) = 0;
	z(0, 3) = 0;	   z(1, 3) = 0;        z(2, 3) = 0; z(3, 3) = 1;

	return x * y * z;
}
Matrix4 Matrix4::CreateScale(float _x, float _y, float _z) {
	Matrix4 s;
	s(0, 0) = _x; s(1, 0) = 0;  s(2, 0) = 0;  s(3, 0) = 0;
	s(0, 1) = 0;  s(1, 1) = _y; s(2, 1) = 0;  s(3, 1) = 0;
	s(0, 2) = 0;  s(1, 2) = 0;  s(2, 2) = _z; s(3, 2) = 0;
	s(0, 3) = 0;  s(1, 3) = 0;  s(2, 3) = 0;  s(3, 3) = 1;
	return s;
}
Matrix4 Matrix4::CreatePerspective(float _fovy, float _aspect, float _zNear, float _zFar) {
	Matrix4 p;
	p(0, 0) = (1 / tan(_fovy / 2)) / _aspect; p(1, 0) = 0;					p(2, 0) = 0;  							       p(3, 0) = 0;
	p(0, 1) = 0;							  p(1, 1) = 1 / tan(_fovy / 2); p(2, 1) = 0;								   p(3, 1) = 0;
	p(0, 2) = 0;							  p(1, 2) = 0;					p(2, 2) = (_zFar + _zNear) / (_zNear - _zFar); p(3, 2) = (2 * _zFar * _zNear) / (_zNear - _zFar);
	p(0, 3) = 0;							  p(1, 3) = 0;					p(2, 3) = -1;								   p(3, 3) = 0;
	return p;
}

/* Operators */
Matrix4 Matrix4::operator* (const Matrix4& _matrix) const {
	Matrix4 temp;
	
	// Loop through each element in the matrix
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			// Now calculate the value for this position
			for (int k = 0; k < 4; ++k) {
				temp(i, j) += (*this)(k, j) * _matrix(i, k);
			}
		}
	}

	return temp;
}
Matrix4& Matrix4::operator*= (const Matrix4& _matrix) {
	Matrix4 temp;
	
	// Loop through each element in the matrix
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			// Now calculate the value for this position
			for (int k = 0; k < 4; ++k) {
				temp(i, j) += (*this)(k, j) * _matrix(i, k);
			}
		}
	}

	*this = temp;
	return (*this);
}
Vector4 Matrix4::operator* (const Vector4& _vec) const {
	Vector4 temp;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			temp[i] += (*this)(j, i) * _vec[j];
		}
	}
	
	return temp;
}

/* Access operators */
const float& Matrix4::operator () (int _x, int _y) const {
	return (data[_y*4 + _x]);
}
float& Matrix4::operator() (int _x, int _y) {
	return (data[_y*4 + _x]);
}