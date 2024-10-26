#include "Light.glsl"
#include "Geometry.glsl"

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

float SmoothNoise( vec3 p )
{
    float f;
    f  = 0.5  * pnoise( p ); p = m * p * 2.02;
    f += 0.25 * pnoise( p ); 
	
    return f * (1.0 / (0.5 + 0.25));
}

vec3 getNebula(in vec3 from, in vec3 dir, float level, float power) 
{
    vec3 color = vec3(0.0);
    float nebula = pow(SmoothNoise(dir+3.0), 12.0);
    
    if (nebula > 0.0)
    {
    	vec3 pos = (dir.xyz + dir.xzy + dir.zyx) / 3.0;
    	vec3 randc = vec3(SmoothNoise( dir.xyz*10.0*level));
		color = nebula * randc;
    }

	return pow(color*2.25, vec3(power));
}

vec3 getStars(in vec3 from, in vec3 dir, float power) 
{
	vec3 color = vec3(pow(SmoothNoise(dir*320.0), 16.0));
	return pow(color*2.25, vec3(power));
}

vec3 computeIncidentLight(const in vec3 orig, const in vec3 dir, out float mixer) 
{ 
    uint numSamples = 16;
    uint numSamplesLight = 8; 
    float t0 = 0.f, t1 = 0.f; 
    float g = 0.76f; 
	vec3 sunDirection = lights[0].vector;
	float tmin = 0.f, tmax = kInfinity;
    if (!raySphereIntersect(orig, dir, atmosphereRadius, earthPosition, t0, t1) || t1 < 0) return vec3(0.f); 
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
        raySphereIntersect(samplePosition, sunDirection, atmosphereRadius, earthPosition, t0Light, t1Light); 
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