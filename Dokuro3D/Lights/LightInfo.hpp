#ifndef LIGHTINFO_DEF
#define LIGHTINFO_DEF

#include "AmbientLight.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "SpotLight.hpp"
#include "../Maths/SSEVector.hpp"
#include <vector>

namespace dkr {
	class LightInfo {
	public:
		LightInfo();
		virtual ~LightInfo();

		// Get the color at a certain point.
		Vector4 GetDirectionalLightOnPoint(const Vector4& _cameraSpacePoint, const Vector4& _normal, int _lightIndex, const Vector4& _diffuse, const Vector4& _specular);
		Vector4 GetPointLightOnPoint(const Vector4& _cameraSpacePoint, const Vector4& _point, const Vector4& _normal, int _lightIndex, const Vector4& _diffuse, const Vector4& _specular);
		Vector4 GetSpotLightOnPoint(const Vector4& _cameraSpacePoint, const Vector4& _point, const Vector4& _normal, int _lightIndex, const Vector4& _diffuse, const Vector4& _specular);

		void GetDirectionalLightOnPoint(const SSEVector& _cameraSpacePoint, const SSEVector& _normal, int _lightIndex, const SSEVector& _diffuse, const SSEVector& _specular, SSEVector& _out);
		void GetPointLightOnPoint(const SSEVector& _cameraSpacePoint, const SSEVector& _point, const SSEVector& _normal, int _lightIndex, const SSEVector& _diffuse, const SSEVector& _specular, SSEVector& _out);
		void GetSpotLightOnPoint(const SSEVector& _cameraSpacePoint, const SSEVector& _point, const SSEVector& _normal, int _lightIndex, const SSEVector& _diffuse, const SSEVector& _specular, SSEVector& _out);
		void GetAmbientLightOnPoint(SSEVector& _vector);

		void SetAmbient(const AmbientLight& _light);
		void AddDirectionalLight(const DirectionalLight& _light);
		void AddPointLight(const PointLight& _light);
		void AddSpotLight(const SpotLight& _spot);
		
		const AmbientLight& GetAmbient() const;
		std::vector<DirectionalLight>& GetDirectionalLights() const;
		std::vector<PointLight>& GetPointLights() const;
		std::vector<SpotLight>& GetSpotLights() const;

		void Clear();
	private:
		AmbientLight ambient;
		std::vector<DirectionalLight> directionalLights;
		std::vector<PointLight> pointLights;
		std::vector<SpotLight> spotLights;
	};
}

#endif