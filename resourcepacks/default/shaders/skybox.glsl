#type vertex
#version 450 core
layout (location = 0) in vec2 a_Pos;

void main()
{
    gl_Position = vec4(a_Pos, 0.0, 1.0); 
}  
#type fragment
#version 450 core

const float PI = 3.14159265358979323846264338327950288f;

struct Light {
	vec3 vector;
	uint color;
};

layout (std140, binding = 0) uniform Transforms
{
    mat4 u_ViewProjection;

	vec3 cameraPos;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
	vec2 windowSize;
	uint u_numLights;
};

layout (std140, binding = 1) uniform Lighting
{
    Light lights[1];
};

float hash( const in float n ) {
	return fract(sin(n)*4378.5453);
}

float pnoise(in vec3 o) 
{
	vec3 p = floor(o);
	vec3 fr = fract(o);
		
	float n = p.x + p.y*57.0 + p.z * 1009.0;

	float a = hash(n+  0.0);
	float b = hash(n+  1.0);
	float c = hash(n+ 57.0);
	float d = hash(n+ 58.0);
	
	float e = hash(n+  0.0 + 1009.0);
	float f = hash(n+  1.0 + 1009.0);
	float g = hash(n+ 57.0 + 1009.0);
	float h = hash(n+ 58.0 + 1009.0);
	
	
	vec3 fr2 = fr * fr;
	vec3 fr3 = fr2 * fr;
	
	vec3 t = 3.0 * fr2 - 2.0 * fr3;
	
	float u = t.x;
	float v = t.y;
	float w = t.z;

	// this last bit should be refactored to the same form as the rest :)
	float res1 = a + (b-a)*u +(c-a)*v + (a-b+d-c)*u*v;
	float res2 = e + (f-e)*u +(g-e)*v + (e-f+h-g)*u*v;
	
	float res = res1 * (1.0- w) + res2 * (w);
	
	return res;
}

const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float SmoothNoise( vec3 p )
{
    float f;
    f  = 0.5000*pnoise( p ); p = m*p*2.02;
    f += 0.2500*pnoise( p ); 
	
    return f * (1.0 / (0.5000 + 0.2500));
}

vec3 getStars(in vec3 from, in vec3 dir, int levels, float power) 
{
	vec3 color=vec3(0.0);
	vec3 st = (dir * 2.+ vec3(0.3,2.5,1.25)) * .3;
	for (int i = 0; i < levels; i++) st = abs(st) / dot(st,st) - .9;
    float star = min( 1., pow( min( 5., length(st) ), 3. ) * .0025 )*1.5;

   	vec3 randc = vec3(SmoothNoise( dir.xyz*10.0*float(levels) ), SmoothNoise( dir.xzy*10.0*float(levels) ), SmoothNoise( dir.yzx*10.0*float(levels) ));
	color += star * randc;

	return pow(color*2.25, vec3(power));
}

float earthRadius = 6360e3;      // In the paper this is usually Rg or Re (radius ground, eart) 
float atmosphereRadius = 6420e3; // In the paper this is usually R or Ra (radius atmosphere) 
float Hr = 7994.f;               // Thickness of the atmosphere if density was uniform (Hr) 
float Hm = 1200.f;               // Same as above but for Mie scattering (Hm)
vec3 betaR = vec3(3.8e-6f, 13.5e-6f, 33.1e-6f); 
vec3 betaM = vec3(21e-6f); 
const float kInfinity = 3.402823466e+38F;

vec3 position = vec3(0.f);
bool raySphereIntersect(const in vec3 rayOrigin, const in vec3 rayDirection, const in float radius, inout float t0, inout float t1) {
	vec3 L = position - rayOrigin;
	float tca = dot(L, rayDirection);

	//if (tca < 0) return false;

	float s2 = (dot(L, L)) - (tca * tca);

	if (s2 > radius * radius) return false;	

	float thc = sqrt((radius * radius) - s2);
	t0 = tca - thc; 
    t1 = tca + thc;

	if (t0 > t1) {
		float t = t0;
		t0 = t1;
		t1 = t;
	}

	return true;
}

vec3 computeIncidentLight(const in vec3 orig, const in vec3 dir, out float mixer) 
{ 
    uint numSamples = 16; 
    uint numSamplesLight = 8; 
    float t0 = 0.f, t1 = 0.f; 
    float g = 0.76f; 
	vec3 sunDirection = lights[0].vector;
	float tmin = 0.f, tmax = kInfinity;
    if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0) return vec3(0.f); 
    if (t0 > tmin && t0 > 0) tmin = t0; 
    if (t1 < tmax) tmax = t1; 
    float segmentLength = (tmax - tmin) / numSamples; 
    float tCurrent = tmin; 
    vec3 sumR = vec3(0.f), sumM = vec3(0.f); // mie and rayleigh contribution 
    float opticalDepthR = 0.f, opticalDepthM = 0.f; 
    float mu = dot(dir, sunDirection); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction 
    float phaseR = 3.f / (16.f * PI) * (1 + mu * mu); 
    float phaseM = 3.f / (8.f * PI) * ((1.f - g * g) * (1.f + mu * mu)) / ((2.f + g * g) * pow(1.f + g * g - 2.f * g * mu, 1.5f)); 
    for (uint i = 0; i < numSamples; ++i) { 
        vec3 samplePosition = orig + (tCurrent + segmentLength * 0.5f) * dir; 
        float height = length(samplePosition) - earthRadius; 
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength; 
        float hm = exp(-height / Hm) * segmentLength; 
        opticalDepthR += hr; 
        opticalDepthM += hm; 
        // light optical depth
        float t0Light = 0.f, t1Light = 0.f; 
        raySphereIntersect(samplePosition, sunDirection, atmosphereRadius, t0Light, t1Light); 
        float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0; 
        float opticalDepthLightR = 0.f, opticalDepthLightM = 0.f; 
        uint j = 0; 
        for (; j < numSamplesLight; ++j) { 
            vec3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5f) * sunDirection; 
            float heightLight = length(samplePositionLight) - earthRadius; 
            if (heightLight < 0) break; 
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight; 
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight; 
            tCurrentLight += segmentLengthLight; 
        } 
        if (j == numSamplesLight) { 
            vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1f * (opticalDepthM + opticalDepthLightM); 
            vec3 attenuation = vec3(exp(-tau)); 
            sumR += attenuation * hr; 
            sumM += attenuation * hm; 
        } 
        tCurrent += segmentLength; 
    } 
	mixer = phaseR;
    return (sumR * betaR * phaseR + sumM * betaM * phaseM) * 20.f;
}

layout(location = 0) out vec4 FragColor;

void main()
{    
    vec2 uv = gl_FragCoord.xy / windowSize;
    vec3 rayDir = normalize(lower_left_corner + uv.x * horizontal + uv.y * vertical - cameraPos);
	vec3 color;

    vec3 color1 = clamp(getStars(cameraPos, rayDir, 1, 0.5) * 1.5, 0.0, 1.0) * vec3(0.0, 0.0, 1.0);
	vec3 color2 = clamp(getStars(cameraPos, -rayDir, 2, 0.5) * 0.9, 0.0, 1.0) * vec3(1.0, 0.0, 0.0);
	vec3 color3 = clamp(getStars(cameraPos, -rayDir, 3, 0.5) * 0.7, 0.0, 1.0) * vec3(1.0, 1.0, 0.0);
	
	vec3 colorStars = getStars(cameraPos, rayDir, 17, 0.9);
	float mixer = 0.f;
	vec3 daySky = computeIncidentLight(vec3(cameraPos.x, earthRadius + 1.f, cameraPos.z), rayDir, mixer);
	vec3 nightSky = color1 + color2 + color3 + colorStars;
	color = mix(daySky, nightSky, mixer);

    FragColor = vec4(color, 1.f);
}