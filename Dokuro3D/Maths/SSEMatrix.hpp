#ifndef SSEMATRIX_DEF
#define SSEMATRIX_DEF

#include "SSEVector.hpp"

namespace dkr {

	class SSEMatrix {
	public:
		SSEMatrix();
		~SSEMatrix();
		SSEMatrix(const SSEMatrix& _mat);

		/* Set's */
		void SetBlank();
		void SetIdentity();
		void SetRotate(float _x, float _y, float _z);
		void SetScale(float _x, float _y, float _z);
		void SetTranslate(float _x, float _y, float _z);
		void SetPerspective(float _fovy, float _aspect, float _zNear, float _zFar);
		//void SetOrthographic() {};

		/* Operators */
		void operator*= (const SSEMatrix& _mat);
		void operator-= (const SSEMatrix& _mat);
		void operator+= (const SSEMatrix& _mat);
		void operator/= (const SSEMatrix& _mat);

		/* Get elements of the matrix */
		float& operator() (int _c, int _r);
		const float& operator() (int _c, int _r) const;

		/* Transform a vector */
		void Transform(SSEVector& _src, SSEVector& _dst) const;

		/* The data for the matrix */
		union {
			struct {
				__m128 rows[4];
			};
			struct {
				float members[4][4];
			};
		};

		/* Make sure this is allocated aligned on the heap */
		void* operator new (size_t _size) {
			void* p = _aligned_malloc(_size, 16);
			if (p == 0)
				return 0;

			return p;
		}
		void operator delete (void* _p) {
			_aligned_free(_p);
		}

		/* Static identity matrix */
		static SSEMatrix Identity;
	};

}

#endif