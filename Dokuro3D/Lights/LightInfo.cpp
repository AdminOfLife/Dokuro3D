#include "LightInfo.hpp"
using namespace dkr;

LightInfo::LightInfo() {
}

LightInfo::~LightInfo() {
}

/* Get the light value for a point in space */
Vector4 LightInfo::GetDirectionalLightOnPoint(const Vector4& _cameraSpacePoint, const Vector4& _normal, int _lightIndex, const Vector4& _diffuse, const Vector4& _specular) {
	Vector4 directionalColor = GetDirectionalLights()[_lightIndex].GetIntensity() * GetDirectionalLights()[_lightIndex].GetDirection().Dot(_normal) * _diffuse;
	directionalColor.Clamp();

	// Specular
	Vector4 halfAngle  = GetDirectionalLights()[_lightIndex].GetDirection() + _cameraSpacePoint;
	halfAngle.Normalize();
	Vector4 specular = _specular * powf(halfAngle.Dot(_normal), 100) * GetDirectionalLights()[_lightIndex].GetIntensity();
	specular.Clamp();
	directionalColor += specular;

	return directionalColor;
}
Vector4 LightInfo::GetPointLightOnPoint(const Vector4& _cameraSpacePoint, const Vector4& _point, const Vector4& _normal, int _lightIndex, const Vector4& _diffuse, const Vector4& _specular) {
	// Get the difference in position from the light and the point.
	Vector4 diff = GetPointLights()[_lightIndex].GetPosition() - _point;
	Vector4 lightDirection = diff;

	// The distance
	float distance = diff.Length();
					
	// The direction
	diff.Normalize();

	// The attenuation
	float attenuation = GetPointLights()[_lightIndex].GetAttenuation(distance);
			
	// Diffuse lighting.
	Vector4 pointColor = GetPointLights()[_lightIndex].GetIntensity() 
								* diff.Dot(_normal)
									* attenuation
										* _diffuse;
	pointColor.Clamp();

	// Specular
	// We get the half angle between the camera and the direction to the camera.
	Vector4 halfAngle = lightDirection + _cameraSpacePoint;
	halfAngle.Normalize();

	Vector4 specular = _specular * powf(halfAngle.Dot(_normal), 10) * attenuation;
	specular.Clamp();
	pointColor += specular;

	return pointColor;
}

// Get the spot light info on a point.
Vector4 LightInfo::GetSpotLightOnPoint(const Vector4& _cameraSpacePoint, const Vector4& _point, const Vector4& _normal, int _lightIndex, const Vector4& _diffuse, const Vector4& _specular) {
	// Get the difference in position from the light and the point.
	Vector4 diff = GetSpotLights()[_lightIndex].GetPosition() - _point;
	Vector4 lightDirection = diff;

	// The distance
	float distance = diff.Length();
					
	// The direction
	diff.Normalize();

	// Calculate the spot light values
	float spotLightValue = diff.Dot(GetSpotLights()[_lightIndex].GetDirection() * -1);
	if (spotLightValue > 0.2f)
		return Vector4(0, 0, 0, 0);
	float smoothing = GetSpotLights()[_lightIndex].SmoothStep(diff.Dot(_normal));

	// The attenuation
	float attenuation = GetSpotLights()[_lightIndex].GetAttenuation(distance);
			
	// Diffuse lighting.
	Vector4 pointColor = GetSpotLights()[_lightIndex].GetIntensity() 
								* diff.Dot(_normal)
									* attenuation
										* _diffuse
											* spotLightValue;

	// Specular
	// We get the half angle between the camera and the direction to the camera.
	Vector4 halfAngle = lightDirection + _cameraSpacePoint;
	halfAngle.Normalize();

	pointColor += _specular * powf(halfAngle.Dot(_normal), 10) * attenuation;

	return pointColor;	
}


// Uses SSEVectors (for phong shading).
void LightInfo::GetDirectionalLightOnPoint(const SSEVector& _cameraSpacePoint, const SSEVector& _normal, int _lightIndex, const SSEVector& _diffuse, const SSEVector& _specular, SSEVector& _out) {
	/* SSE Version */
	SSEVector intensity(GetDirectionalLights()[_lightIndex].GetIntensity().x, GetDirectionalLights()[_lightIndex].GetIntensity().y, GetDirectionalLights()[_lightIndex].GetIntensity().z, 0);
	SSEVector direction(GetDirectionalLights()[_lightIndex].GetDirection().x, GetDirectionalLights()[_lightIndex].GetDirection().y , GetDirectionalLights()[_lightIndex].GetDirection().z, 0);

	// Diffuse
	SSEVector diffuse = intensity;
	diffuse *= _diffuse;
	
	// Calculate the dot product of the direction and the normal
	float dot = direction.Dot(_normal);

	diffuse *= dot;

	// Clamp the diffuse
	diffuse.Clamp();

	// Specular
	SSEVector halfAngle = direction;
	halfAngle += _cameraSpacePoint;
	halfAngle.Normalize();
	float p = powf(halfAngle.Dot(_normal), 100);
	SSEVector specular = _specular;
	specular *= p;
	specular *= intensity;

	// Clamp specular
	specular.Clamp();

	// Modify the color
	_out += diffuse;
	_out += specular;
}
void LightInfo::GetPointLightOnPoint(const SSEVector& _cameraSpacePoint, const SSEVector& _point, const SSEVector& _normal, int _lightIndex, const SSEVector& _diffuse, const SSEVector& _specular, SSEVector& _out) {
	// Get the direction and distance from the light source.
	SSEVector diff(GetPointLights()[_lightIndex].GetPosition().x, GetPointLights()[_lightIndex].GetPosition().y, GetPointLights()[_lightIndex].GetPosition().z, 0);
	diff -= _point;
	SSEVector lightDirection = diff;

	// Get the distance
	float distance = diff.Length();

	// The direction 
	diff.Normalize();

	// The attenuation
	float attenuation = GetPointLights()[_lightIndex].GetAttenuation(distance);

	// Diffuse lighting
	// Get the light intensity
	SSEVector intensity(GetPointLights()[_lightIndex].GetIntensity().x, GetPointLights()[_lightIndex].GetIntensity().y, GetPointLights()[_lightIndex].GetIntensity().z, 0);

	// Calculate the light
	SSEVector color = intensity;
	color *= diff.Dot(_normal);
	color *= attenuation;
	color *= _diffuse;

	// Clamp the color.
	color.Clamp();

	// Specular
	// We get the half angle between the camera and the direction to the camera.
	SSEVector halfAngle = lightDirection;
	halfAngle += _cameraSpacePoint;
	halfAngle.Normalize();

	SSEVector specular = _specular;
	specular *= powf(halfAngle.Dot(_normal), 10);
	specular *= attenuation;
	specular *= intensity;
	
	// Clamp specular
	specular.Clamp();

	// Add to the color
	_out += color;
	_out += specular;
}
void LightInfo::GetAmbientLightOnPoint(SSEVector& _vector) {
	Vector4 temp = GetAmbient().GetIntensity();
	_vector.x += temp.x;
	_vector.y += temp.y;
	_vector.z += temp.z;
}


void LightInfo::SetAmbient(const AmbientLight& _light) {
	ambient = _light;
}

void LightInfo::AddDirectionalLight(const DirectionalLight& _light) {
	directionalLights.push_back(_light);
}

void LightInfo::AddPointLight(const PointLight& _light) {
	pointLights.push_back(_light);
}

void LightInfo::AddSpotLight(const SpotLight& _spot) {
	spotLights.push_back(_spot);
}

const AmbientLight& LightInfo::GetAmbient() const {
	return ambient;
}

std::vector<DirectionalLight>& LightInfo::GetDirectionalLights() const {
	return const_cast<std::vector<DirectionalLight>& >(directionalLights);
}

std::vector<PointLight>& LightInfo::GetPointLights() const {
	return const_cast<std::vector<PointLight>& >(pointLights);
}

std::vector<SpotLight>& LightInfo::GetSpotLights() const {
	return const_cast<std::vector<SpotLight>& >(spotLights);
}

void LightInfo::Clear() {
	ambient = AmbientLight();
	directionalLights.clear();
	pointLights.clear();
	spotLights.clear();
}