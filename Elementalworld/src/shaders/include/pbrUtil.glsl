#include "Light.glsl"
#include "constants.glsl"
#include "Math.glsl"
#include "fresnel.glsl"

// Visibility functions
#define SPECULAR_V_SMITH_GGX        0
#define SPECULAR_V_GGX_ANISOTROPIC  1

#define BRDF_ANISOTROPIC_V          SPECULAR_V_SMITH_GGX

//------------------------------------------------------------------------------
// Specular BRDF implementations
//------------------------------------------------------------------------------

float D_GGX(float roughness, float NoH, const vec3 h) {
    float oneMinusNoHSquared = 1.0 - NoH * NoH;

    float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / PI);
    return d;
}

float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) {
    float a2 = at * ab;
    vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * (1.0 / PI);
}

float D_Charlie(float roughness, float NoH) {
    float invAlpha  = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    float a2 = roughness * roughness;    
    float lambdaV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2); // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (lambdaV + lambdaL);
    return v;
}

float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
        float ToL, float BoL, float NoV, float NoL) {    
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV)); // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    float v = 0.5 / (lambdaV + lambdaL);
    return saturate(v);
}

float V_Kelemen(float LoH) {
    return saturate(0.25 / (LoH * LoH));
}

float V_Neubelt(float NoV, float NoL) {
    return saturate(1.0 / (4.0 * (NoL + NoV - NoL * NoV)));
}

//------------------------------------------------------------------------------
// Specular BRDF dispatch
//------------------------------------------------------------------------------

float visibilityAnisotropic(float roughness, float at, float ab,
        float ToV, float BoV, float ToL, float BoL, float NoV, float NoL) {
#if BRDF_ANISOTROPIC_V == SPECULAR_V_SMITH_GGX
    return V_SmithGGXCorrelated(roughness, NoV, NoL);
#elif BRDF_ANISOTROPIC_V == SPECULAR_V_GGX_ANISOTROPIC
    return V_SmithGGXCorrelated_Anisotropic(at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
#endif
}

//------------------------------------------------------------------------------
// Diffuse BRDF implementations
//------------------------------------------------------------------------------

float Fd_Burley(float roughness, float NoV, float NoL, float LoH) {
    // Burley 2012, "Physically-Based Shading at Disney"
    float f90 = 0.5 + 2.0 * roughness * LoH * LoH;
    float lightScatter = F_Schlick(1.0, f90, NoL);
    float viewScatter  = F_Schlick(1.0, f90, NoV);
    return lightScatter * viewScatter / PI;
}

// Energy conserving wrap diffuse term, does *not* include the divide by pi
float Fd_Wrap(float NoL, float w) {
    return saturate((NoL + w) / sq(1.0 + w));
}


/*
vec3 surfaceShading(const PixelParams pixel, const Light light, float occlusion) {
    vec3 h = normalize(shading_view + light.l);

    float NoV = shading_NoV;
    float NoL = saturate(light.NoL);
    float NoH = saturate(dot(shading_normal, h));
    float LoH = saturate(dot(light.l, h));

    float D = D_GGX(roughness, NoH, h);
    float V = V_SmithGGXCorrelated(roughness, NoV, NoL);
    vec3  F = fresnel(f0, LoH);

    vec3 Fr = (D * V) * F;
    vec3 Fd = pixel.diffuseColor * diffuse(pixel.roughness, NoV, NoL, LoH);

    // The energy compensation term is used to counteract the darkening effect
    // at high roughness
    vec3 color = Fd + Fr * pixel.energyCompensation;

    return (color * light.colorIntensity.rgb) *
            (light.colorIntensity.w * light.attenuation * NoL * occlusion);
}

vec3 computeF0(const vec4 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float computeDielectricF0(float reflectance) {
    return 0.16 * reflectance * reflectance;
}

float computeMetallicFromSpecularColor(const vec3 specularColor) {
    return max3(specularColor);
}

float iorToF0(float transmittedIor, float incidentIor) {
    return sq((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

float f0ToIor(float f0) {
    float r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

vec3 f0ClearCoatToSurface(const vec3 f0) {
    return saturate(f0 * (f0 * (0.941892 - 0.263008 * f0) + 0.346479) - 0.0285998);
}

*/