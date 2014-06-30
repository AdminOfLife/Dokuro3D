#ifndef MATRIX_DEF
#define MATRIX_DEF

#include <cstring>
#include <ostream>
#include <iomanip>
#include <cmath>
#include <xmmintrin.h>

#include "../Exceptions/OutOfBoundsException.hpp"

namespace dkr {

#ifdef DISABLE_SSE

	template <short U, short V>
	class Matrix {
		/* Prints the matrix to a stream */
		friend std::ostream& operator<< (std::ostream& _out, const Matrix<U, V>& _mat) {
			for (int i = 0; i < U; ++i) {
				for (int j = 0; j < V; ++j) {
					_out << std::setw(10) << _mat(i, j);
				}
				_out << std::endl;
			}

			return _out;
		}

	public:
		Matrix() {
			for (int i = 0; i < U; i++) {
				for (int j = 0; j < V; j++) {
					(*this)(i, j) = 0;
				}
			}
		}

		~Matrix() {
		}

		/* Copy a matrix */
		Matrix(const Matrix<U, V>& _mat) {
			(*this) = _mat;
		}

		/* Assign a matrix */
		const Matrix<U, V>& operator= (const Matrix<U, V>& _mat) {
			memcpy(data, _mat.data, sizeof(float)*U*V);
			return *this;
		}

		/* Addition */
		Matrix<U, V>& operator+= (const Matrix<U, V>& _mat) {
			for (int i = 0; i < U*V; ++i) {
				data[i] += _mat.data[i];
			}
			return *this;
		}
		const Matrix<U, V> operator+ (const Matrix<U, V>& _mat) const {
			Matrix<U, V> matrix = *this;
			matrix += _mat;
			return matrix;
		}

		/* Subtraction */
		Matrix<U, V>& operator-= (const Matrix<U, V>& _mat) {
			for (int i = 0; i < U*V; ++i) {
				data[i] -= _mat.data[i];
			}
			return *this;
		}
		const Matrix<U, V> operator- (const Matrix<U, V>& _mat) const {
			Matrix<U, V> matrix = *this;
			matrix -= _mat;
			return matrix;
		}

		/* Multiplication */
		Matrix<U, U>& operator*= (const Matrix<U, U>& _mat) {
			// Make a new matrix
			Matrix<U, U> mat;

			// Loop through each element in the new matrix
			for (int i = 0; i < U; ++i) {
				for (int j = 0; j < U; ++j) {
					for (int k = 0; k < U; ++k) {
						mat(i, j) += (*this)(k, j) * _mat(i, k);
					}
				}
			}

			(*this) = mat;
			return (*this);
		}
		template <short X>
		Matrix<X, V> operator* (const Matrix<X, U>& _mat) const {
			// Make a new matrix
			Matrix<X, V> mat;

			// Loop through each element in the new matrix
			for (int i = 0; i < X; ++i) {
				for (int j = 0; j < V; ++j) {
					for (int k = 0; k < U; ++k) {
						mat(i, j) += (*this)(k, j) * _mat(i, k);
					}
				}
			}

			return mat;
		}
		
		Matrix<U, V> Mult(const Matrix<U, V>& _mat) {
			for (int i = 0; i < U * V; i++) {
				data[i] *= _mat.data[i];
			}
		}

		/* Get Data */
		const float& operator() (int _x, int _y) const {
			if (_x >= U || _y >= V)
				throw OutOfBoundsException("Matrix indices are out of bounds!");

			return data[_y*U + _x];
		}
		float& operator() (int _x, int _y) {
			if (_x >= U || _y >= V)
				throw OutOfBoundsException("Matrix indices are out of bounds!");

			return data[_y*U + _x];
		}

		/* Return the identity matrix */
		static Matrix<U, V> CreateIdentity() {
			Matrix<U, V> m;
			for (int i = 0; i < U; i++) {
				for (int j = 0; j < V; j++) {
					if (i == j)
						m(i, j) = 1;
					else
						m(i, j) = 0;
				}
			}
			return m;
		}

		/* Create a test matrix */
		static Matrix<U, V> CreateTest() {
			Matrix<U, V> m;	
			for (int i = 0; i < U*V; ++i) {
				m.data[i] = i;
			}
			return m;
		}

		/* For vectors only! */
		float& X() {
			return data[0];
		}
		float& Y() {
			return data[1];
		}
		float& Z() {
			return data[2];
		}
		float& W() {
			return data[3];
		}

	protected:
		// The matrix data
		float data[U * V]; // Make sure this is aligned on a 16byte boundary
						   // So that SSE instructions will be fast
	};

	/* Matrix classes */
	typedef Matrix<4, 4> Matrix4x4;
	class Matrix<4, 4> {
	public:
		static Matrix<4, 4> CreateTranslation(float _x, float _y, float _z) {
			Matrix4x4 m;
			m(0, 0) = 1; m(1, 0) = 0; m(2, 0) = 0; m(3, 0) = _x;
			m(0, 1) = 0; m(1, 1) = 1; m(2, 1) = 0; m(3, 1) = _y;
			m(0, 2) = 0; m(1, 2) = 0; m(2, 2) = 1; m(3, 2) = _z;
			m(0, 3) = 0; m(1, 3) = 0; m(2, 3) = 0; m(3, 3) = 1;
			return m;
		}

		static Matrix<4, 4> CreateRotation(float _x, float _y, float _z) {
			Matrix4x4 x;
			x(0, 0) = 1; x(1, 0) = 0;	    x(2, 0) = 0;	    x(3, 0) = 0;
			x(0, 1) = 0; x(1, 1) = cos(_x); x(2, 1) = -sin(_x); x(3, 1) = 0;
			x(0, 2) = 0; x(1, 2) = sin(_x); x(2, 2) = cos(_x);  x(3, 2) = 0;
			x(0, 3) = 0; x(1, 3) = 0;       x(2, 3) = 0;        x(3, 3) = 1;
			
			Matrix4x4 y;
			y(0, 0) = cos(_y);  y(1, 0) = 0; y(2, 0) = sin(_y); y(3, 0) = 0;
			y(0, 1) = 0;        y(1, 1) = 1; y(2, 1) = 0;       y(3, 1) = 0;
			y(0, 2) = -sin(_y); y(1, 2) = 0; y(2, 2) = cos(_y); y(3, 2) = 0;
			y(0, 3) = 0;        y(1, 3) = 0; y(2, 3) = 0;       y(3, 3) = 1;
			
			Matrix4x4 z;
			z(0, 0) = cos(_z); z(1, 0) = -sin(_z); z(2, 0) = 0; z(3, 0) = 0;
			z(0, 1) = sin(_z); z(1, 1) = cos(_z);  z(2, 1) = 0; z(3, 1) = 0;
			z(0, 2) = 0;	   z(1, 2) = 0;		   z(2, 2) = 1; z(3, 2) = 0;
			z(0, 3) = 0;	   z(1, 3) = 0;        z(2, 3) = 0; z(3, 3) = 1;

			return x * y * z;
		}

		static Matrix<4, 4> CreateScale(float _x, float _y, float _z) {
			Matrix4x4 s;
			s(0, 0) = _x; s(1, 0) = 0;  s(2, 0) = 0;  s(3, 0) = 0;
			s(0, 1) = 0;  s(1, 1) = _y; s(2, 1) = 0;  s(3, 1) = 0;
			s(0, 2) = 0;  s(1, 2) = 0;  s(2, 2) = _z; s(3, 2) = 0;
			s(0, 3) = 0;  s(1, 3) = 0;  s(2, 3) = 0;  s(3, 3) = 1;
			return s;
		}

		static Matrix<4, 4> CreatePerspective(float _fovy, float _aspect, float _zNear, float _zFar) {
			Matrix4x4 p;
			p(0, 0) = (1 / tan(_fovy / 2)) / _aspect; p(1, 0) = 0;					p(2, 0) = 0;  							       p(3, 0) = 0;
			p(0, 1) = 0;							  p(1, 1) = 1 / tan(_fovy / 2); p(2, 1) = 0;								   p(3, 1) = 0;
			p(0, 2) = 0;							  p(1, 2) = 0;					p(2, 2) = (_zFar + _zNear) / (_zNear - _zFar); p(3, 2) = (2 * _zFar * _zNear) / (_zNear - _zFar);
			p(0, 3) = 0;							  p(1, 3) = 0;					p(2, 3) = -1;								   p(3, 3) = 0;
			return p;
		}

	private:
	};

	/* Vertex classes */
	class Vertex4f : public Matrix<1, 4> {
	public:
		Vertex4f() {
			W() = 1;
		}
	};

	/* Types */
	typedef Matrix<2, 2> Matrix2x2;
	typedef Matrix<3, 3> Matrix3x3;

	typedef Matrix<1, 4> Vector4f;

#endif

}

#endif