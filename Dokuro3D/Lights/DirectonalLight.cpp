#include "DirectionalLight.hpp"
using namespace dkr;

DirectionalLight::DirectionalLight(const Vector4& _dir, const Vector4& _intensity) {
	SetDirection(_dir);
	SetIntensity(_intensity);
}

void DirectionalLight::SetDirection(const Vector4& _dir) {
	direction = _dir;
	direction.Normalize();
}

void DirectionalLight::SetIntensity(const Vector4& _intensity) {
	intensity = _intensity;
}

const Vector4& DirectionalLight::GetDirection() const {
	return direction;
}

const Vector4& DirectionalLight::GetIntensity () const {
	return intensity;
}
