#include "AmbientLight.hpp"
using namespace dkr;

AmbientLight::AmbientLight() {
}

AmbientLight::AmbientLight(const Vector4& _intensity) {
	intensity = _intensity;
}

const Vector4& AmbientLight::GetIntensity() const {
	return intensity;
}