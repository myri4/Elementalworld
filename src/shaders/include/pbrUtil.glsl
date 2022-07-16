#include "sceneData.glsl"
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

vec3 rayTrace(const in vec3 N, const vec4 materialData, const in vec3 albedo) {
	vec3 color;

	float metallic = materialData[0];
	float roughness = materialData[1];
	float ao = materialData[2];
	float emmision = materialData[3];

	vec3 V = normalize(cameraPos - p0);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	vec3 F0 = mix(vec3(0.04f), albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.f);
	for (uint i = 0; i < u_numLights; i++)
	{
		// calculate per-light radiance
		vec3 radiance;
		const float c = 1.f / 255.f;
		uint color = lights[i].color;
		radiance.r = float((color & uint(0x000000ff))) * c;
		radiance.g = float((color & uint(0x0000ff00)) >> 8) * c;
		radiance.b = float((color & uint(0x00ff0000)) >> 16) * c;
		float radius = float((color & uint(0xff000000)) >> 24) * c;

		vec3 L = lights[i].vector;
		if (radius > 0.f){
			L -= p0;
			radiance /= dot(L, L) * radius; // radiance / attenuation
		}
		L = normalize(L);

		vec3 H = normalize(L + V);

		float NdotL = max(dot(N, L), 0.f);
		float NdotV = max(dot(N, V), 0.f);
		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
		// fresnelSchlick
		vec3 F = fresnelSchlick(max(dot(H, V), 0.f), F0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.f * NdotV * NdotL + 0.001f; // 0.001 to prevent divide by zero.
		vec3 specular = numerator / denominator;

		vec3 kD = (1.f - F) * (1.f - metallic); // diffuse
		Lo += ((kD * albedo / PI + specular) * radiance * NdotL);  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	// ambient lighting (note that the next IBL tutorial will replace
	// this ambient lighting with environment lighting).

	vec3 ambient = ambientColor * ao * albedo;

	color = ambient + Lo;
	
	return color;
}