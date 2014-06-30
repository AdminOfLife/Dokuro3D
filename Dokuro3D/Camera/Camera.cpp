#include "Camera.hpp"
using namespace dkr;

Camera::Camera() {
	// Generate the matrix
	GetMatrix();
}

Camera::~Camera() {
}

/* Set the positions */
void Camera::SetPosition(float _x, float _y, float _z) {
	position.x = _x;
	position.y = _y;
	position.z = _z;
}

void Camera::Move(float _dx, float _dy, float _dz) {
	position.x += _dx;
	position.y += _dy;
	position.z += _dz;
}

void Camera::FPSTranslate(float _x, float _y, float _z) {
	Vector4 forward = Vector4(matrix(0, 2), matrix(1, 2), matrix(2, 2), 0);
	Vector4 up = Vector4(matrix(0, 1), matrix(1, 1), matrix(2, 1), 0);
	Vector4 right = Vector4(matrix(0, 0), matrix(1, 0), matrix(2, 0), 0);

	position = position + (forward * _z);
	position = position + (right * _x);
	position = position + (up * _y);
}

/* Set the rotations */
void Camera::SetRotation(float _x, float _y, float _z) {
	rotation.x = _x;
	rotation.y = _y;
	rotation.z = _z;
}

void Camera::Rotate(float _dx, float _dy, float _dz) {
	rotation.x += _dx;
	rotation.y += _dy;
	rotation.z += _dz;
}

void Camera::FPSRotate(float _x, float _y) {
	rotation.x += _y;
	rotation.y += _x;
}

/* Get's */
const Vector4& Camera::GetPosition() const {
	return position;
}
const Vector4& Camera::GetRotation() const {
	return rotation;
}

const Vector4 Camera::GetForwardVector() const {
	return Vector4(matrix(0, 2), matrix(1, 2), matrix(2, 2), 0);
}
const Vector4 Camera::GetRightVector() const {
	return Vector4(matrix(0, 0), matrix(1, 0), matrix(2, 0), 0);
}
const Vector4 Camera::GetUpVector() const {
	return Vector4(matrix(0, 1), matrix(1, 1), matrix(2, 1), 0);
}

/* Get the matrix */
Matrix4 Camera::GetMatrix() {
	matrix = Matrix4::CreateRotation(-rotation.x, -rotation.y, -rotation.z) *
				Matrix4::CreateTranslation(-position.x, -position.y, -position.z);
	return matrix;
}