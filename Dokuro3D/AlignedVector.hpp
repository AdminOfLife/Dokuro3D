#ifndef ALIGNEDVECTOR_DEF
#define ALIGNEDVECTOR_DEF

#include <malloc.h>

namespace dkr {

	template <typename T, short A=16>
	class AlignedVector {
	public:
		AlignedVector() {
			data = 0;
			size = 0;
			capacity = 0;
		}
		~AlignedVector() {
			Clear();
		}

		/* Copying */
		AlignedVector<T>& operator= (const AlignedVector<T>& _vector) {
			// Clear ourselves
			Clear();

			// Loop through and copy the other vector
			for (int i = 0; i < _vector.GetSize(); ++i) {
				Add(_vector[i]);
			}

			return *this;
		}
		AlignedVector(const AlignedVector<T>& _vector) {
			// Set data to 0 to avoid trying to free memory that isn't allocated
			data = 0;
			*this = _vector;
		}

		/* Resize the array */
		void Resize(int _size) {
			T* tempData = (T*)_aligned_malloc(sizeof(T) * _size, A);

			// Copy the data that was in the array
			if (data != 0) {
				// Copy the data
				for (int i = 0; i < _size; ++i) {
					tempData[i] = data[i];
				}

				// Free memory
				_aligned_free(data);

				// Set the capacity
				capacity = _size;
			}

			// Set the data to point to the temporary data
			data = tempData;
		}

		/* Add / Remove */
		void Add(const T& _t) {
			if (size >= capacity) {
				if (capacity == 0)
					Resize(2);
				else
					Resize(capacity * 2);
			}

			size++;
			data[size - 1] = _t;
		}

		/* Clear */
		void Clear() {
			size = 0;
			capacity = 0;
			_aligned_free(data);
			data = 0;
		}

		/* Get the front */
		const T& GetTop() const {
			return data[size - 1];
		}

		// Remove an item from the top
		void RemoveTop() {
			size -= 1;
		}

		/* Get the size */
		int GetSize() const {
			return size;
		}

		/* Get the data */
		T* GetData() const {
			return data;
		}
		T& operator[] (int _i) {
			return data[_i];
		}
		const T& operator[] (int _i) const {
			return data[_i];
		}

	private:
		T* data;
		int size;
		int capacity;
	};

}

#endif