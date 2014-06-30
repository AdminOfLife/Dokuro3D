#ifndef MATRIX4_DEF
#define MATRIX4_DEF

#include "Vector4.hpp"

namespace dkr {

	class Matrix4 {
	public:
		Matrix4();

		/* Static matrices */
		static Matrix4 Identity;

		/* Create matrices */
		static Matrix4 CreateIdentity();
		static Matrix4 CreateTranslation(float _x, float _y, float _z);
		static Matrix4 CreateRotation(float _x, float _y, float _z);
		static Matrix4 CreateScale(float _x, float _y, float _z);
		static Matrix4 CreatePerspective(float _fovy, float _aspect, float _zNear, float _zFar);

		/* Operators */
		Matrix4 operator* (const Matrix4& _matrix) const;
		Matrix4& operator*= (const Matrix4& _matrix);
		Vector4 operator* (const Vector4& _vec) const;

		/* Access operators */
		const float& operator () (int _x, int _y) const;
		float& operator() (int _x, int _y);
	private:
		/* The matrix data */
		float data[16];
	};

}

#endif