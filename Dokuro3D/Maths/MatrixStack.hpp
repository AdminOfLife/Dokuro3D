#ifndef MATRIXSTACK_DEF
#define MATRIXSTACK_DEF

#include <vector>
#include "Matrix4.hpp"

namespace dkr {

	class MatrixStack {
	public:
		MatrixStack() {
		}
		~MatrixStack() {}

		/* Push a Matrix4 onto the stack */
		void Push(Matrix4& _matrix) {
			Matrix4 matrix = _matrix;
			if (stack.size() != 0) {
				matrix *= stack.back();
			}

			stack.push_back(matrix);
		}

		/* Pop a Matrix4 of the stack */
		void Pop() {
			if (!stack.empty())
				stack.pop_back();
		}

		/* Peek at the Matrix4 on top of the stack */
		const Matrix4& Peek() const {
			if (stack.size() == 0)
				return Matrix4::Identity;
			
			return stack.back();
		}

	private:
		/* The stack of matrices */
		std::vector<Matrix4> stack;
	};
}

#endif