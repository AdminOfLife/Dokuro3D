#ifndef DIRECTIONALLIGHT_DEF
#define DIRECTIONALLIGHT_DEF

#include "../Maths/Vector4.hpp"

namespace dkr {
	class DirectionalLight {
	public:
		DirectionalLight(const Vector4& _dir, const Vector4& _intensity);
		
		void SetDirection(const Vector4& _dir);
		void SetIntensity(const Vector4& _intensity);

		const Vector4& GetDirection() const;
		const Vector4& GetIntensity() const;

	private:
		Vector4 direction;
		Vector4 intensity;
	};
}

#endif