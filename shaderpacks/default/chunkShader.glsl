#type vertex
#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoords;
out vec3 v_FragPos;
out float v_visibility;

uniform const float u_Density = 0.017;
uniform const float u_Gradient = 0.5;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_View = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);
uniform bool u_Fog = true;


void main(){
    vec4 PosRelativeToCam = u_View * u_Model * vec4(a_Pos, 1.0);
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
	v_TexCoords = a_TexCoord;
	v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    if (u_Fog){
        float dist = length(PosRelativeToCam.xyz);
        v_visibility = exp(-pow((dist * u_Density), u_Gradient));
        v_visibility = clamp(v_visibility, 0.0f, 1.0f);
    }
}


#type fragment
#version 460 core 

layout (location = 0) out vec4 Result;

in vec2 v_TexCoords;
in vec3 v_FragPos;
in float v_visibility;


struct Light {
    vec4 vector;
    
    float constant;
    float linear;
    float quadratic;

    float strenght;

    vec3 color;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform vec3 viewPos;
uniform sampler2D u_Texture;
uniform float gamma = 2.2;
uniform bool blinn = true;
uniform bool attenuation = true;
uniform float minimalLight = 0.1f;
uniform bool fog = true;
uniform vec3 fogColor = vec3(0.5f);
Material material;
Light ll;


vec3 CalculateLight(Light light, Material mat, vec3 normal, vec3 viewDir, bool blinn, bool atten){
    // Ambient
    vec3 Ambient = mat.ambient * light.color;
    
    // Diffuse
    vec3 lightDir;
    if(light.vector.w == 0.0) // note: be careful for floating point errors
        lightDir = normalize(-light.vector.xyz);
    else if (light.vector.w >= 1.0)
        lightDir = normalize(light.vector.xyz - v_FragPos);  
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 Diffuse = pow(light.color * (diff * mat.diffuse), vec3(gamma));

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);   
    float spec = 0.0;

    if(!blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), mat.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess / 4);
    }
    vec3 Specular = light.color * (spec * mat.specular);
    
    // Attenuation    
    if(atten){
        float distance    = length(light.vector.rgb - v_FragPos);
        float attenuation;
        if(gamma != 0)
            attenuation = light.strenght / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); // / (gamma ? distance * distance : distance)
        else
            attenuation = light.strenght / (light.constant + light.linear * distance + light.quadratic * distance); // / (gamma ? distance * distance : distance)

        Ambient  *= attenuation; 
        Diffuse  *= attenuation;
        Specular *= attenuation;
    }
    
    return Ambient + Diffuse + Specular;
}

void main()
{
    ll.vector = vec4(viewPos, 1);
    ll.color = vec3(1.0f, 1.0f, 1.0f);
    ll.constant = 1.0f;
    ll.linear = 0.09f;
    ll.quadratic = 0.032f;
    ll.strenght = 0.1f;

    //normalmap calculations
    vec3 normal = vec3(0.5f, 0.5f, 1.0f);
    normal = normalize(normal * 2.0 - 1.0); 

    material.ambient = vec3(1.0f, 1.0f, 1.0f);
	material.diffuse = vec3(1.0f, 2.0f, 1.0f);
	material.specular = vec3(1.0f, 1.0f, 1.0f);
	material.shininess = 0.0f;

    vec4 texColor = texture(u_Texture, vec2(v_TexCoords));// * minimalLight;


    //texColor += vec4(CalculateLight(ll, material, normalize(normal), normalize(viewPos - v_FragPos), blinn, attenuation), 1.0f);

    if(texColor.a < 0.1) discard;
    if(fog)
        Result = mix(vec4(fogColor, 1.0f), texColor, v_visibility);
    else
        Result = texColor;
}