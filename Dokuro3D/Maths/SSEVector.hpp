#ifndef SSEVECTOR_DEF
#define SSEVECTOR_DEF

#include <xmmintrin.h>
#include <mmintrin.h>
#include <cmath>

namespace dkr {

	class SSEVector {
	public:
		SSEVector();
		~SSEVector();
		SSEVector(__m128& _data);
		SSEVector(float _v);
		SSEVector(float _x, float _y, float _z, float _w);
		SSEVector(const SSEVector& _vec);
		
		/* Operators */
		void operator *= (const SSEVector& _vec);
		void operator -= (const SSEVector& _vec);
		void operator += (const SSEVector& _vec);
		void operator /= (const SSEVector& _vec);

		/* Scalar operators */
		void operator *= (float _f);
		void operator /= (float _f);

		/* Clamp */
		void Clamp();

		/* Length */
		float Length();

		/* Normalize */
		void Normalize();

		/* Calculate the dot product */
		float Dot(const SSEVector& _vec);
		
		/* Store the __m128 in a union */
		union {
			__m128 data;
			struct {
				float w, z, y, x;
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
	};

}

#endif