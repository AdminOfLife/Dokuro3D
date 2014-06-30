#ifndef SSEVectorInteger_DEF
#define SSEVectorInteger_DEF

#include <xmmintrin.h>

namespace dkr {

	class SSEVectorInteger {
	public:
		SSEVectorInteger();
		~SSEVectorInteger();
		SSEVectorInteger(__m128& _data);
		SSEVectorInteger(int _v);
		SSEVectorInteger(int _x, int _y, int _z, int _w);
		SSEVectorInteger(const SSEVectorInteger& _vec);
		
		/* Operators */
		void operator *= (const SSEVectorInteger& _vec);
		void operator -= (const SSEVectorInteger& _vec);
		void operator += (const SSEVectorInteger& _vec);
		void operator /= (const SSEVectorInteger& _vec);

		/* Scalar operators */
		void operator *= (int _f);

		/* Store the __m128 in a union */
		union {
			__m128 data;
			struct {
				int w, z, y, x;
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