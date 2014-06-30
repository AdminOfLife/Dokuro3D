#ifndef POINTLIGHT_DEF
#define POINTLIGHT_DEF

#include "../Maths/Vector4.hpp"

namespace dkr {
	class PointLight {
	public:
		PointLight(const Vector4& _pos, const Vector4& _intensity, float _minLight=0.4f, float _linearDrop=100.0f, float _quadraticDrop=1.0f);

		void SetPosition(const Vector4& _pos);
		void SetIntensity(const Vector4& _intensity);
		void SetCoefficients(float _minLight, float _linearDrop, float _quadraticDrop);

		const Vector4& GetPosition() const;
		const Vector4& GetIntensity() const;
		float GetAttenuation(float _distance) const;

	private:
		Vector4 position;
		Vector4 intensity;

		// Attenuation coefficients.
		float minLight;
		float linearDrop;
		float quadraticDrop;
	};
}

#endif