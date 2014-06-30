#ifndef CAMERA_DEF
#define CAMERA_DEF

#include "../Maths/Matrix4.hpp"

namespace dkr {
	class Camera {
	public:
		Camera();
		~Camera();

		// Set the position
		void SetPosition(float _x, float _y, float _z);
		void Move(float _dx, float _dy, float _dz);

		// Set the rotation
		void SetRotation(float _x, float _y, float _z);
		void Rotate(float _dx, float _dy, float _dz);

		// FPS Controls
		void FPSRotate(float _x, float _y);
		void FPSTranslate(float _x, float _y, float _z);

		// Get's
		const Vector4& GetPosition() const;
		const Vector4& GetRotation() const;

		// Get the vectors.
		const Vector4 GetForwardVector() const;
		const Vector4 GetRightVector() const;
		const Vector4 GetUpVector() const;

		// Get the camera matrix
		Matrix4 GetMatrix();

	private:
		Vector4 position;
		Vector4 rotation;
		Matrix4 matrix;
	};
}

#endif