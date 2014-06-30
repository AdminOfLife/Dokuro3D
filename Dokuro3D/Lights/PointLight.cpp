#include "PointLight.hpp"
using namespace dkr;

PointLight::PointLight(const Vector4& _pos, const Vector4& _intensity, float _minLight, float _linearDrop, float _quadraticDrop) {
	position = _pos;
	intensity = _intensity;
	linearDrop = _linearDrop;
	minLight = _minLight;
	quadraticDrop = _quadraticDrop;
}

void PointLight::SetPosition(const Vector4& _pos) {
	position = _pos;
}

void PointLight::SetIntensity(const Vector4& _intensity) {
	intensity = _intensity;
}

void PointLight::SetCoefficients(float _minLight, float _linearDrop, float _quadraticDrop) {
	minLight = _minLight;
	linearDrop = _linearDrop;
	quadraticDrop = _quadraticDrop;
}

const Vector4& PointLight::GetPosition() const {
	return position;
}
const Vector4& PointLight::GetIntensity() const {
	return intensity;
}
float PointLight::GetAttenuation(float _distance) const {
	return (1/(minLight + linearDrop*_distance + quadraticDrop*_distance*_distance));
}