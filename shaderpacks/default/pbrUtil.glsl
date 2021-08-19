#ifndef PBR_UTIL
#define PBR_UTIL

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

#endif