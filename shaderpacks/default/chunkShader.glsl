#type vertex
#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;

out vec2 v_TexCoords;
out float v_visibility;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform const float u_Density = 0.001f;
uniform const float u_Gradient = 1.5f;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_View = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);

void main()
{
    vec4 PosRelativeToCam = u_View * u_Model * vec4(a_Pos, 1.0);
	gl_Position = u_Projection * PosRelativeToCam;
	v_TexCoords = a_TexCoord;

    //Lighting
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));

    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow((dist * u_Density), u_Gradient));
    v_visibility = clamp(v_visibility, 0.0f, 1.0f);    
}

#type fragment
#version 330 core 

in vec2 v_TexCoords;
in vec3 v_Normal;
in float v_visibility;
in vec3 v_FragPos;

uniform vec3 viewPos;

uniform sampler2D u_Texture;
uniform bool blinn = true;
uniform bool gamma = true;
uniform vec3 fogColor = vec3(0.1f, 3.5f, 5.0f);

struct Material {
    vec3 ambient;
    float shininess;
};

struct Light {
    vec4 vector;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float strenght;
};

Material mat;
Light light;

vec4 CalculateLight(Material material, Light light){
    vec4 tex = texture(u_Texture, v_TexCoords);

    // Ambient
    vec3 Ambient = material.ambient * light.ambient;
  	
    // Diffuse 
    vec3 normal = normalize(v_Normal);
    vec3 lightDir;

    if(light.vector.w == 0.0f) lightDir = normalize(-light.vector.xyz);
    else if (light.vector.w >= 1.0f) lightDir = normalize(light.vector.xyz - v_FragPos);  

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 Diffuse = diff * light.diffuse;
    
    // Specular
    vec3 viewDir = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess / 2);
    }

    float contrast = 1.0f;
    float depthValue = tex.r;
    vec3 specularMap = vec3(depthValue);
    specularMap = (specularMap - 0.5f) * (1.0f + contrast) + 0.5f;
    vec3 Specular = light.specular * spec * specularMap; 

    if (light.vector.w >= 1.0f){
        float distance    = length(light.vector.xyz - v_FragPos);
        float attenuation = light.strenght / (light.constant + light.linear * distance + light.quadratic * (gamma ? distance * distance : distance));
        Ambient  *= attenuation; 
        Diffuse  *= attenuation;
        Specular *= attenuation;
    }

    return vec4(Ambient + Diffuse + Specular, 1.0f) * tex;
}

void main()
{

    vec4 finalColor;

    mat.shininess = 0.0f;
    mat.ambient = vec3(0.7f);

    light.vector = vec4(viewPos, 0.0f);
    light.ambient = vec3(1.0f);
    light.diffuse = vec3(1.0f);
    light.specular =vec3(1.0f);
    light.constant = 1.0f;
    light.linear = 0.09f;
    light.quadratic = 0.032f;
    light.strenght = 0.5f;

    finalColor = texture(u_Texture, v_TexCoords);
    if(finalColor.a < 0.1) discard;
  
    gl_FragColor = mix(vec4(fogColor, 1.0f), finalColor, v_visibility);
}