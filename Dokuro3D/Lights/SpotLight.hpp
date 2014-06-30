#ifndef SPOTLIGHT_DEF
#define SPOTLIGHT_DEF

#include "../Maths/Vector4.hpp"

namespace dkr {
	class SpotLight {
	public:
		SpotLight(const Vector4& _pos, const Vector4& _intensity, const Vector4& _direction, float innerAngle, float outerAngle, float _minLight=0.4f, float _linearDrop=100.0f, float _quadraticDrop=1.0f);

		void SetPosition(const Vector4& _pos);
		void SetIntensity(const Vector4& _intensity);
		void SetCoefficients(float _minLight, float _linearDrop, float _quadraticDrop);
		void SetDirection(const Vector4& _direction);
		void SetInnerAngle(float _innerAngle);
		void SetOuterAngle(float _outerAngle);

		const Vector4& GetPosition() const;
		const Vector4& GetIntensity() const;
		float GetAttenuation(float _distance) const;
		const Vector4& GetDirection() const;
		float GetInnerAngle() const;
		float GetOuterAngle() const;

		float SmoothStep(float _value);

	private:
		Vector4 position;
		Vector4 intensity;
		Vector4 direction;

		// The spot light angles
		float innerAngle;
		float outerAngle;

		// Attenuation coefficients.
		float minLight;
		float linearDrop;
		float quadraticDrop;
	};
}

#endif