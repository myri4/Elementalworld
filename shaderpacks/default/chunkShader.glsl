#type vertex
#version 430 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_TexCoord;
layout (location = 2) in int a_Type;
layout (location = 3) in uint a_Color;
layout (location = 4) in vec3 a_Normal;

const float PI = 3.1415926535897932384626433832795;

out vec3 v_TexCoords;
flat out int v_Type;
out vec4 v_Color;
out float v_visibility;
out vec3 v_Normal;
out vec3 p0;

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

void main()
{
    vec3 currentVertex = a_Pos;

    if (a_Type == 1) currentVertex = vec3(a_Pos.x, a_Pos.y - 0.2f, a_Pos.z); // fluid

    vec4 PosRelativeToCam = u_View * vec4(currentVertex, 1.f); // * u_Model

	gl_Position = u_Projection * PosRelativeToCam;

	v_TexCoords = a_TexCoord;

    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow(dist * u_Density, u_Gradient));
    v_visibility = clamp(v_visibility, 0.f, 1.f);    
    v_Type = a_Type;

    const float c = 1.f / 255.f;
    v_Color.r = float((a_Color & uint(0xff000000)) >> 24) * c;
    v_Color.g = float((a_Color & uint(0x00ff0000)) >> 16) * c;
    v_Color.b = float((a_Color & uint(0x0000ff00)) >> 8) * c;
    v_Color.a = float((a_Color & uint(0x000000ff))) * c;

    //Normal
    v_Normal = normalize(a_Normal); // normalize

    p0 = currentVertex;
}

#type fragment
#version 430 core 
#include "shaderpacks/default/pbrUtil.glsl"

in vec3 v_TexCoords;
flat in int v_Type;
in vec4 v_Color;
in float v_visibility;
in vec3 v_Normal;
in vec3 p0;

#define POINT_LIGHT 1

struct Light {
	uint color;
	vec3 vector;
};

layout (std140, binding = 1) uniform Lighting
{
    Light lights[1];
};


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

layout(binding = 0) uniform sampler2DArray u_Texture;
layout(binding = 1) uniform sampler2DArray u_NormalMap;

vec3 rayTrace(in vec3 albedo) {
	vec3 color;

	float metallic = 0.f;
	float roughness = 1.f;
	float ao = 1.f;

	vec3 N = getNormalFromMap(texture(u_NormalMap, v_TexCoords).rgb, v_Normal, p0, v_TexCoords.xy);
	vec3 V = normalize(cameraPos - p0);

	//if (!gl_FrontFacing) N = -N;

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
		//if (type == POINT_LIGHT)
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


void main() {
    vec4 textureColor = texture(u_Texture, v_TexCoords) * v_Color;
    if ((!gl_FrontFacing && v_Type != 1 && v_Type != 2) || textureColor.a < 0.1) discard;

    vec4 finalColor = textureColor;// * vec4(rayTrace(textureColor.rgb), textureColor.a);
  
    gl_FragColor = mix(vec4(fogColor, 1.0f), finalColor, v_visibility);
}