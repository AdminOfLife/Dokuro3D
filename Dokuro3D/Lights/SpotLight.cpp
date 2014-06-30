#include "SpotLight.hpp"
using namespace dkr;

SpotLight::SpotLight(const Vector4& _pos, const Vector4& _intensity, const Vector4& _direction, float _innerAngle, float _outerAngle, float _minLight, float _linearDrop, float _quadraticDrop) {
	position = _pos;
	intensity = _intensity;

	direction = _direction;
	direction.Normalize();

	innerAngle = _innerAngle;
	outerAngle = _outerAngle;

	linearDrop = _linearDrop;
	minLight = _minLight;
	quadraticDrop = _quadraticDrop;
}

void SpotLight::SetPosition(const Vector4& _pos) {
	position = _pos;
}

void SpotLight::SetIntensity(const Vector4& _intensity) {
	intensity = _intensity;
}

void SpotLight::SetDirection(const Vector4& _direction) {
	direction = _direction;
	direction.Normalize();
}

void SpotLight::SetCoefficients(float _minLight, float _linearDrop, float _quadraticDrop) {
	minLight = _minLight;
	linearDrop = _linearDrop;
	quadraticDrop = _quadraticDrop;
}

void SpotLight::SetInnerAngle(float _innerAngle) {
	innerAngle = _innerAngle;
}

void SpotLight::SetOuterAngle(float _outerAngle) {
	outerAngle = _outerAngle;
}

const Vector4& SpotLight::GetPosition() const {
	return position;
}
const Vector4& SpotLight::GetIntensity() const {
	return intensity;
}
const Vector4& SpotLight::GetDirection() const {
	return direction;
}
float SpotLight::GetAttenuation(float _distance) const {
	return (1/(minLight + linearDrop*_distance + quadraticDrop*_distance*_distance));
}
float SpotLight::GetInnerAngle() const {
	return innerAngle;
}
float SpotLight::GetOuterAngle() const {
	return outerAngle;
}

// Smoothstep
float SpotLight::SmoothStep(float _value) {
	float x = (_value - outerAngle) / (innerAngle - outerAngle);
	if (x < 0)
		x = 0;
	if (x > 1)
		x = 1;
	return x * x * (3 - 2 * x);
}