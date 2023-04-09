#ifndef CONSTANTS_GLSL
#define CONSTANTS_GLSL

const float Epsilon = 1e-5;
const float bias = 1e-5f;
const float kInfinity = 3.402823466e+38F;

const float PI = 3.14159265358979323846264338327950288;
const float TAU = 6.28318530718;
const float RECIPROCAL_TAU = 0.15915494309;
const float HALF_PI = 1.57079632680;
const float QUARTER_PI = 0.78539816339;
const float RECIPROCAL_PI = 0.31830988618;

// Golden ratio: (1 + Math.sqrt(5)) / 2
const float PHI = 1.61803398875;

const float MIN_ROUGHNESS = 0.002025;
const float MIN_PERCEPTUAL_ROUGHNESS = 0.045;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float earthRadius = 6360e3;      // In the paper this is usually Rg or Re (radius ground, eart) 
float atmosphereRadius = 6420e3; // In the paper this is usually R or Ra (radius atmosphere) 
float Hr = 7994.f;               // Thickness of the atmosphere if density was uniform (Hr) 
float Hm = 1200.f;               // Same as above but for Mie scattering (Hm)
vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f); 
vec3 betaM = vec3(21e-6f); 
vec3 earthPosition = vec3(0.f);

const int WC_MODEL_BIT = 1;
const int WC_CULL_BIT = 2;

const uint chunkSize = 16;
const uint chunkVolume = chunkSize * chunkSize * chunkSize;
#endif