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
uniform sampler2D u_Textures[32];
uniform float gamma = 2.2;
uniform bool blinn = true;
uniform bool attenuation = true;
uniform float minimalLight = 0.1f;
uniform Material material;
Light ll;

vec3 CalculateLight(Light light, Material material, vec3 normal, vec3 viewDir, bool blinn, bool atten){
    // Ambient
    vec3 Ambient = material.ambient * light.color;
    
    // Diffuse
    vec3 lightDir;
    if(light.vector.w == 0.0) // note: be careful for floating point errors
        lightDir = normalize(-light.vector.xyz);
    else if (light.vector.w >= 1.0)
        lightDir = normalize(light.vector.xyz - v_FragPos);  
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 Diffuse = pow(light.color * (diff * material.diffuse), vec3(gamma));

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);   
    float spec = 0.0;

    if(!blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess / 4);
    }
    vec3 Specular = light.color * (spec * material.specular);
    
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
    ll.vector = vec4(-0.2f, -1.0f, -0.3f, 1);
    ll.color = vec3(1.0f, 1.0f, 1.0f);
    ll.constant = 1.0f;
    ll.linear = 0.09f;
    ll.quadratic = 0.032f;
    ll.strenght = 0.5f;

    vec4 texColor = texture(u_Textures[int(v_TexIndex)], v_TexCoords);// * minimalLight;

    //texColor += vec4(CalculateLight(ll, material, normalize(v_Normal), normalize(viewPos - v_FragPos), blinn, attenuation), 1.0f);

    if(texColor.a < 0.1) discard;

    Result = texColor;
}