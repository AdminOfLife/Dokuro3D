#ifndef AMBIENTLIGHT_DEF
#define AMBIENTLIGHT_DEF

#include "../Maths/Vector4.hpp"

namespace dkr {
	class AmbientLight {
	public:
		AmbientLight();
		AmbientLight(const Vector4& _intensity);

		/* Get the intensity of the ambient light */
		const Vector4& GetIntensity() const;

	private:
		Vector4 intensity;
	};
}

#endif