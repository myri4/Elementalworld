vec3 fresnelSchlick(float HoV, vec3 F0)
{
    float x = 1.0 - HoV;
    return F0 + (1.0 - F0) * x * x * x * x * x;
}

// Source: https://github.com/ssteinberg/ste/blob/master/Simulation/src/ste/framework_graphics/radiometry/radiance/BxDFs/shaders/fresnel.glsl
float fresnel_schlick_tir(float F0, float cos_theta_incident, float cos_critical) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float p = 1.f - (cos_theta_incident - cos_critical) / (1 - cos_critical);
	float p2 = p*p;
	float a = p2 * p2;

	return mix(F0, 1.f, a);
}

float fresnel_steinberg(float F0, float cos_theta_incident, float cos_critical, float refractive_ratio) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float p = 1.f - (cos_theta_incident - cos_critical) / (1 - cos_critical);
	float a = pow(p, 6.f + 18.f*exp(-13.f*max(.0f, refractive_ratio - 1)));
	return mix(F0, 1.f, a);
}

float fresnel(float cos_theta_incident, float cos_critical, float refractive_ratio) {
	if (cos_theta_incident <= cos_critical)
		return 1.f;

	float sin_theta_incident2 = 1.f - cos_theta_incident*cos_theta_incident;
	float t = sqrt(1.f - sin_theta_incident2 / (refractive_ratio * refractive_ratio));
	float sqrtRs = (cos_theta_incident - refractive_ratio * t) / (cos_theta_incident + refractive_ratio * t);
	float sqrtRp = (t - refractive_ratio * cos_theta_incident) / (t + refractive_ratio * cos_theta_incident);

	return mix(sqrtRs * sqrtRs, sqrtRp * sqrtRp, .5f);
}

vec3 F_Schlick(vec3 f0, float f90, float HoV) {
    return f0 + (f90 - f0) * pow(1.0 - HoV, 5.f);
}

float F_Schlick(float f0, float f90, float HoV) {
    return f0 + (f90 - f0) * pow(1.0 - HoV, 5.f);
}

vec3 fresnel(const vec3 f0, float LoH) {
    float f90 = saturate(dot(f0, vec3(50.0 * 0.33)));
    return F_Schlick(f0, f90, LoH);
}