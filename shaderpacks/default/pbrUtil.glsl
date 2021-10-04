//#ifndef PBR_UTIL
//#define PBR_UTIL

#define Epsilon 1.192092896e-07F
const float PI = 3.14159265358979323846264338327950288f;

vec3 getNormalFromMap(const in vec3 TN, const in vec3 N, const in vec3 p0, const in vec2 TexCoords)
{
    vec3 tangentNormal = TN * 2.f - 1.f;

    vec3 Q1  = dFdx(p0);
    vec3 Q2  = dFdy(p0);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return N;//-normalize(TBN * tangentNormal);
}

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
// ----------------------------------------------------------------------------
float GeometrySmith(const in float NdotL, const in float NdotV, const in float roughness)
{
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

#define POINT_LIGHT 1

layout (std140, binding = 0) uniform Transforms
{
    mat4 u_Projection;
    mat4 u_View;
    float deltaTime;
	int u_numLights;

	vec3 fogColor;
	vec3 cameraPos;
	vec3 ambientColor;
	float u_Density;
	float u_Gradient;
};

struct Light {
	uint color;
	vec3 vector;
};

layout (std140, binding = 1) uniform Lighting
{
    Light lights[1];
};

vec3 rayTrace(const in vec3 albedo, const in vec3 N) {
	vec3 color;

	float metallic = 0.f;
	float roughness = 1.f;
	float ao = 1.f;

	vec3 V = normalize(cameraPos - p0);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, albedo, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.f);
	for (uint i = 0; i < u_numLights; i++)
	{
		// calculate per-light radiance
		vec3 radiance;
		const float c = 1.f / 255.f;
		uint color = lights[i].color;
		radiance.r = float((color & uint(0xff000000)) >> 24) * c;
		radiance.g = float((color & uint(0x00ff0000)) >> 16) * c;
		radiance.b = float((color & uint(0x0000ff00)) >> 8) * c;
		float type = float((color & uint(0x000000ff))) * c;
		vec3 lightVector = lights[i].vector;
		vec3 L = -lightVector;
		if (type == POINT_LIGHT)
		{
			L = lightVector - p0;
			float Distance = length(L);
			float attenuation = Distance * Distance / 4.f;
			radiance /= attenuation;
		}

		L = normalize(L);

		vec3 H = normalize(V + L);

		float NdotL = max(dot(N, L), 0.f);
		float NdotV = max(dot(N, V), 0.f);
		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(NdotL, NdotV, roughness);
		// fresnelSchlick
		// @TODO: check if its ok to use this formula
		vec3 F = F0 + (max(vec3(1.f - roughness), F0) - F0) * pow(max(1.f - max(dot(H, V), 0.f), 0.f), 5.f);

		vec3 numerator = NDF * G * F;
		float denominator = 4.f * NdotV * NdotL + 0.001f; // 0.001 to prevent divide by zero.
		vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F; // specular
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.f) - kS; // diffuse
		// multiply kD by the inverse metalness such that only non-metals
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.f - metallic;

		// add to outgoing radiance Lo
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	// ambient lighting (note that the next IBL tutorial will replace
	// this ambient lighting with environment lighting).

	vec3 ambient = ambientColor * ao * albedo;

	color = ambient + Lo;
	// HDR tonemapping
	color = color / (color + 1.f);
	// gamma correct
	color = pow(color, vec3(1.f / 2.2f));
	
	return color;
}

//#endif