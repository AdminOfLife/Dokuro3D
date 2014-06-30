#ifndef VECTOR4_DEF
#define VECTOR4_DEF

namespace dkr {

	class Vector4 {
	public:
		Vector4();
		Vector4(float _v);
		Vector4(float _x, float _y, float _z, float _w);

		/* Operators */
		Vector4 operator* (const Vector4& _vec) const;
		Vector4& operator*= (const Vector4& _vec);
		Vector4 operator+ (const Vector4& _vec) const;
		Vector4& operator+= (const Vector4& _vec);
		Vector4 operator- (const Vector4 _vec) const;
		Vector4& operator-= (const Vector4 _vec);

		/* Scalar operators */
		Vector4 operator* (float _f) const;
		Vector4 operator/ (float _f) const;
		Vector4& operator/= (float _f);

		/* Dot product */
		float Dot(const Vector4& _vec) const;
		Vector4 Cross(const Vector4& _vec) const;

		/* Length */
		float Length();

		/* Normalize */
		void Normalize();

		/* Clamp */
		void Clamp();

		/* Abs */
		Vector4 Abs();

		/* Accesing data */
		float& operator[] (int _i);
		const float& operator[] (int _i) const;

		union {
			float data[4];
			struct {
				float x, y, z, w;
			};
		};
	};

}

#endif