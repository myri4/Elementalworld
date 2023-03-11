#include "Light.glsl"
#include "constants.glsl"

vec3 ambientColor = vec3(0.03f);

float DistributionGGX(const in vec3 N, const in vec3 H, const in float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.f);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = NdotH2 * (a2 - 1.f) + 1.f;
	denom = PI * denom * denom;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(const in float NdotV, const in float roughness)
{
	float r = roughness + 1.f;
	float k = r * r * 0.125f;

	//float nom = NdotV;
	//float denom = NdotV * (1.f - k) + k;
	//	   nom   / denom
	return NdotV / (NdotV - NdotV * k + k);
}

vec3 fresnelSchlick(const in float cosTheta, const in vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(const in float cosTheta, const in vec3 F0, const in float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}