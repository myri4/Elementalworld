#type vertex
#version 330 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in int a_Type;

const float PI = 3.1415926535897932384626433832795;

const float waveLength = 1.0;
const float waveAmplitude = 1.0;

out vec3 v_TexCoords;
out float v_visibility;
out vec3 v_Normal;
out vec3 v_FragPos;
uniform float waveTime;

uniform const float u_Density = 0.001f;
uniform const float u_Gradient = 1.5f;

uniform mat4 u_Model = mat4(1.f);
uniform mat4 u_View = mat4(1.f);
uniform mat4 u_Projection = mat4(1.f);

float generateOffset(float x, float z, float val1, float val2){
	float radiansX = ((mod(x+z*x*val1, waveLength)/waveLength) + waveTime * mod(x * 0.8f + z, 1.5f)) * 2.f * PI;
	float radiansZ = ((mod(val2 * (z*x +x*z), waveLength)/waveLength) + waveTime * 2.f * mod(x , 2.f) ) * 2.f * PI;

	return waveAmplitude * 0.5f * (sin(radiansZ) + cos(radiansX));
}

vec3 applyDistortion(vec3 vertex){                            
	float xDistortion = generateOffset(vertex.x, vertex.z, 0.1f, 0.1f);
	float yDistortion = generateOffset(vertex.x, vertex.z, 0.1f, 0.1f);
	float zDistortion = generateOffset(vertex.x, vertex.z, 0.1f, 0.1f);

	return vertex + vec3(xDistortion, yDistortion, zDistortion);
}

void main()
{
    vec3 currentVertex = a_Pos;

    if (a_Type > 1) currentVertex = vec3(a_Pos.x, a_Pos.y - 0.2f, a_Pos.z); // fluid

    vec4 PosRelativeToCam = u_View * u_Model * vec4(currentVertex, 1.f);
    //currentVertex = applyDistortion(currentVertex);

	gl_Position = u_Projection * PosRelativeToCam;
	v_TexCoords = a_TexCoord;

    //Lighting
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_FragPos = vec3(u_Model * vec4(currentVertex, 1.f));

    // Fog
    float dist = length(PosRelativeToCam.xyz);
    v_visibility = exp(-pow((dist * u_Density), u_Gradient));
    v_visibility = clamp(v_visibility, 0.f, 1.f);    
}

//#type geom4etry
//#version 330 core
//layout (triangles) in;
//layout (triangle_strip, max_vertices = 3) out;
//
//in  vec3 v_TexCoords[];
//out vec3 g_TexCoords;
//
//float time;
//
//vec4 explode(vec4 position, vec3 normal)
//{
//    float magnitude = 2.0;
//    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
//    return position + vec4(direction, 0.0);
//}
//
//vec3 GetNormal()
//{
//    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
//    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
//    return normalize(cross(a, b));
//}
//
//void main() {    
//    vec3 normal = GetNormal();
//
//    gl_Position = explode(gl_in[0].gl_Position, normal);
//    g_TexCoords = v_TexCoords[0];
//    EmitVertex();
//    gl_Position = explode(gl_in[1].gl_Position, normal);
//    g_TexCoords = v_TexCoords[1];
//    EmitVertex();
//    gl_Position = explode(gl_in[2].gl_Position, normal);
//    g_TexCoords = v_TexCoords[2];
//    EmitVertex();
//    EndPrimitive();
//}


#type fragment
#version 330 core 

in vec3 v_TexCoords;
in vec3 g_Normal;
in float v_visibility;
in vec3 v_FragPos;

uniform vec3 viewPos;

uniform sampler2DArray u_Texture;
uniform bool blinn = true;
uniform bool gamma = true;
uniform vec3 fogColor = vec3(0.1f, 3.5f, 5.f);
uniform float deltaTime = 0.f;

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

uniform Light lights;
Material mat;

vec4 CalculateLight(Material material, Light light){
    vec4 tex = texture(u_Texture, v_TexCoords);

    // Ambient
    vec3 Ambient = material.ambient * light.ambient;
  	
    // Diffuse 
    vec3 normal = normalize(g_Normal);
    vec3 lightDir;

    if(light.vector.w == 0.f) lightDir = normalize(-light.vector.xyz);
    else if (light.vector.w >= 1.f) lightDir = normalize(light.vector.xyz - v_FragPos);  

    float diff = max(dot(normal, lightDir), 0.f);
    vec3 Diffuse = diff * light.diffuse;
    
    // Specular
    vec3 viewDir = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.f), material.shininess);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess / 2);
    }
    vec3 Specular = vec3(0.f); 
    if(material.shininess > 0.f){
        float contrast = 1.f;
        float depthValue = tex.r;
        vec3 specularMap = vec3(depthValue);
        specularMap = (specularMap - 0.5f) * (1.f + contrast) + 0.5f;
        Specular = light.specular * spec * specularMap;
    }

    if (light.vector.w >= 1.f){
        float distance    = length(light.vector.xyz - v_FragPos);
        float attenuation = light.strenght / (light.constant + light.linear * distance + light.quadratic * (gamma ? distance * distance : distance));
        Ambient  *= attenuation; 
        Diffuse  *= attenuation;
        Specular *= attenuation;
    }

    return 
    //vec4(Ambient + Diffuse + Specular, 1.0f) * //vec4(vec3(gl_FragCoord.z), 1.0)
    tex;
}

void main()
{

    vec4 finalColor;

    mat.shininess = 0.f;
    mat.ambient = vec3(1.f);

    finalColor += CalculateLight(mat, lights);
    //finalColor = vec4(g_Normal, 1);
    if(finalColor.a < 0.1) discard;
  
    gl_FragColor = mix(vec4(fogColor, 1.0f), finalColor, v_visibility);
}