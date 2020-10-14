#type vertex
#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in float a_TexIndex;
layout (location = 3) in vec3 a_Normal;

out vec2 v_TexCoords;
out float v_TexIndex;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 model = mat4(1.0f);
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vec4(a_Pos, 1.0);
	v_TexCoords = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_Normal = a_Normal;
	v_FragPos = vec3(model * vec4(a_Pos, 1.0));
}


#type fragment
#version 330 core 


layout (location = 0) out vec4 Result;

in vec2 v_TexCoords;
in float v_TexIndex;
in vec3 v_Normal;
in vec3 v_FragPos;


struct Light {
    vec4 vector;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;

    vec3 color;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform vec3 viewPos;
uniform sampler2D u_Textures[32];
uniform float gamma = 2.2;


vec3 Ambient(Material material, Light light){return material.ambient * light.color;}
vec3 Diff(Material material, Light light, vec3 lightDir, vec3 norm){
    if(light.vector.w == 0.0) // note: be careful for floating point errors
    lightDir = normalize(-light.vector.xyz);
    else if (light.vector.w >= 1.0)
    lightDir = normalize(light.vector.xyz - v_FragPos);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 DiffuseFinal = light.color * (diff * material.diffuse) * vec3(gamma);
    
    return DiffuseFinal;
}
vec3 Specular(Material material, Light light, vec3 lightDir, vec3 norm, bool blinn){
    vec3 viewDir = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);   
    float spec = 0.0;

    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, norm);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 SpecularFinal = light.color * (spec * material.specular);  

    return SpecularFinal;
}
void main()
{
//light.vector = vec4(-0.2f, -1.0f, -0.3f, 1);
//light.color = vec3(1.0f, 1.0f, 1.0f);
////Ambient
//vec3 Ambient = Ambient(material,light);
//
////Diffuse
//vec3 norm = normalize(v_Normal);
//vec3 lightDir;
//vec3 Diffuse = Diff(material, light, lightDir, norm);
//
////Specular
//vec3 Specular = Specular(material, light, lightDir, norm, true);

    int index = int(v_TexIndex);
    vec4 texColor = texture(u_Textures[index], v_TexCoords);

    if(texColor.a < 0.1) discard;


    Result = texColor;
}