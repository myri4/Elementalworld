#type vertex
#version 460 core

const float PI = 3.14159265358979323846264338327950288;

const float waveLength = 10.0;
const float waveAmplitude = 1.0;

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoords;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_View = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);
uniform float waveTime;

float generateOffset(float x, float z){

    float radiansX = (x / waveLength + waveTime) * 2.0 * PI;
    float radiansZ = (x / waveLength + waveTime) * 2.0 * PI;
    return waveAmplitude * 0.5 * (sin(radiansZ) + sin(radiansX));
}

vec3 applyDistortion(vec3 vertex){
    float xdistorition = generateOffset(vertex.x, vertex.z);
    float ydistorition = generateOffset(vertex.x, vertex.z);
    float zdistorition = generateOffset(vertex.x, vertex.z);

    return vertex + vec3(xdistorition, ydistorition, zdistorition);
}

void main(){
    vec3 currentVertex = a_Pos;
    currentVertex = applyDistortion(currentVertex);
	gl_Position = u_Projection * u_View * u_Model * vec4(currentVertex, 1.0);
	v_TexCoords = a_TexCoord;
}


#type fragment
#version 460 core

in vec2 v_TexCoords;
uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoords) * vec4(1,1,1,0.4);

    if(texColor.a <= 0) discard;
       
    gl_FragColor = texColor;
}