#include "SSEMatrix.hpp"
using namespace dkr;

#include <cmath>

/* Identity matrix */
SSEMatrix SSEMatrix::Identity;

SSEMatrix::SSEMatrix() {
	SetIdentity();
}
SSEMatrix::~SSEMatrix() {
}
SSEMatrix::SSEMatrix(const SSEMatrix& _mat) {
	for (int i = 0; i < 4; ++i) {
		rows[i] = _mat.rows[i];
	}
}

void SSEMatrix::SetBlank() {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			(*this)(i, j) = 0;
		}
	}
}

void SSEMatrix::SetIdentity() {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			if (i == j)
				(*this)(i, j) = 1;
			else
				(*this)(i, j) = 0;
		}
	}
}

void SSEMatrix::SetRotate(float _x, float _y, float _z) {
	SSEMatrix x;
	x(0, 0) = 1; x(1, 0) = 0;	    x(2, 0) = 0;	    x(3, 0) = 0;
	x(0, 1) = 0; x(1, 1) = cos(_x); x(2, 1) = -sin(_x); x(3, 1) = 0;
	x(0, 2) = 0; x(1, 2) = sin(_x); x(2, 2) = cos(_x);  x(3, 2) = 0;
	x(0, 3) = 0; x(1, 3) = 0;       x(2, 3) = 0;        x(3, 3) = 1;
			
	SSEMatrix y;
	y(0, 0) = cos(_y);  y(1, 0) = 0; y(2, 0) = sin(_y); y(3, 0) = 0;
	y(0, 1) = 0;        y(1, 1) = 1; y(2, 1) = 0;       y(3, 1) = 0;
	y(0, 2) = -sin(_y); y(1, 2) = 0; y(2, 2) = cos(_y); y(3, 2) = 0;
	y(0, 3) = 0;        y(1, 3) = 0; y(2, 3) = 0;       y(3, 3) = 1;
			
	SSEMatrix z;
	z(0, 0) = cos(_z); z(1, 0) = -sin(_z); z(2, 0) = 0; z(3, 0) = 0;
	z(0, 1) = sin(_z); z(1, 1) = cos(_z);  z(2, 1) = 0; z(3, 1) = 0;
	z(0, 2) = 0;	   z(1, 2) = 0;		   z(2, 2) = 1; z(3, 2) = 0;
	z(0, 3) = 0;	   z(1, 3) = 0;        z(2, 3) = 0; z(3, 3) = 1;

	SetIdentity();
	(*this) *= x;
	(*this) *= y;
	(*this) *= z;
}
void SSEMatrix::SetScale(float _x, float _y, float _z) {
	(*this)(0, 0) = _x; (*this)(1, 0) = 0;  (*this)(2, 0) = 0;  (*this)(3, 0) = 0;
	(*this)(0, 1) = 0;  (*this)(1, 1) = _y; (*this)(2, 1) = 0;  (*this)(3, 1) = 0;
	(*this)(0, 2) = 0;  (*this)(1, 2) = 0;  (*this)(2, 2) = _z; (*this)(3, 2) = 0;
	(*this)(0, 3) = 0;  (*this)(1, 3) = 0;  (*this)(2, 3) = 0;  (*this)(3, 3) = 1;
}
void SSEMatrix::SetTranslate(float _x, float _y, float _z) {
	(*this)(0, 0) = 1; (*this)(1, 0) = 0; (*this)(2, 0) = 0; (*this)(3, 0) = _x;
	(*this)(0, 1) = 0; (*this)(1, 1) = 1; (*this)(2, 1) = 0; (*this)(3, 1) = _y;
	(*this)(0, 2) = 0; (*this)(1, 2) = 0; (*this)(2, 2) = 1; (*this)(3, 2) = _z;
	(*this)(0, 3) = 0; (*this)(1, 3) = 0; (*this)(2, 3) = 0; (*this)(3, 3) = 1;
}
void SSEMatrix::SetPerspective(float _fovy, float _aspect, float _zNear, float _zFar) {
	(*this)(0, 0) = (1 / tan(_fovy / 2)) / _aspect; (*this)(1, 0) = 0;					(*this)(2, 0) = 0;  							     (*this)(3, 0) = 0;
	(*this)(0, 1) = 0;							    (*this)(1, 1) = 1 / tan(_fovy / 2); (*this)(2, 1) = 0;								     (*this)(3, 1) = 0;
	(*this)(0, 2) = 0;							    (*this)(1, 2) = 0;					(*this)(2, 2) = (_zFar + _zNear) / (_zNear - _zFar); (*this)(3, 2) = (2 * _zFar * _zNear) / (_zNear - _zFar);
	(*this)(0, 3) = 0;							    (*this)(1, 3) = 0;					(*this)(2, 3) = -1;								     (*this)(3, 3) = 0;
}

/* Operators */
void SSEMatrix::operator*= (const SSEMatrix& _mat) {
	SSEMatrix mat;
	mat.SetBlank();
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 4; ++k) {
				mat(i, j) += (*this)(k, j) * _mat(i, k);
			}
		}
	}
	*this = mat;
}
void SSEMatrix::operator-= (const SSEMatrix& _mat) {
	for (int i = 0; i < 4; ++i) {
		rows[i] = _mm_sub_ps(rows[i], _mat.rows[i]);
	}
}
void SSEMatrix::operator+= (const SSEMatrix& _mat) {
	for (int i = 0; i < 4; ++i) {
		rows[i] = _mm_add_ps(rows[i], _mat.rows[i]);
	}
}
void SSEMatrix::operator/= (const SSEMatrix& _mat) {
	for (int i = 0; i < 4; ++i) {
		rows[i] = _mm_div_ps(rows[i], _mat.rows[i]);
	}
}

/* Get elements of the matrix */
float& SSEMatrix::operator() (int _c, int _r) {
	return members[_c][_r];
}
const float& SSEMatrix::operator() (int _c, int _r) const {
	return members[_c][_r];
}

/* Transform a vector */
void SSEMatrix::Transform(SSEVector& _src, SSEVector& _dst) const {
	_dst.x = (*this)(0, 0) * _src.x + (*this)(1, 0) * _src.y + (*this)(2, 0) * _src.z + (*this)(3, 0) * _src.w;
	_dst.y = (*this)(0, 1) * _src.x + (*this)(1, 1) * _src.y + (*this)(2, 1) * _src.z + (*this)(3, 1) * _src.w;
	_dst.z = (*this)(0, 2) * _src.x + (*this)(1, 2) * _src.y + (*this)(2, 2) * _src.z + (*this)(3, 2) * _src.w;
	_dst.w = (*this)(0, 3) * _src.x + (*this)(1, 3) * _src.y + (*this)(2, 3) * _src.z + (*this)(3, 3) * _src.w;
}