#type vertex
#version 460 core

const float PI = 3.1415926535897932384626433832795;

const float waveLength = 1.0;
const float waveAmplitude = 1.0;

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoords;

uniform mat4 u_Model = mat4(1.0f);
uniform mat4 u_View = mat4(1.0f);
uniform mat4 u_Projection = mat4(1.0f);
uniform float waveTime;
uniform vec3 chunkPos = vec3(1.0f);

float generateOffset(float x, float z, float val1, float val2){
	float radiansX = ((mod(x+z*x*val1, waveLength)/waveLength) + waveTime * mod(x * 0.8 + z, 1.5)) * 2.0 * PI;
	float radiansZ = ((mod(val2 * (z*x +x*z), waveLength)/waveLength) + waveTime * 2.0 * mod(x , 2.0) ) * 2.0 * PI;

	return waveAmplitude * 0.5 * (sin(radiansZ) + cos(radiansX));
}

vec3 applyDistortion(vec3 vertex){
	float xDistortion = generateOffset(vertex.x, vertex.z, 0.1, 0.1);
	float yDistortion = generateOffset(vertex.x, vertex.z, 0.1, 0.1);
	float zDistortion = generateOffset(vertex.x, vertex.z, 0.1, 0.1);

	return vertex + vec3(xDistortion, yDistortion, zDistortion);
}

void main(){
    vec3 currentVertex = vec3(a_Pos.x, a_Pos.y - 0.2, a_Pos.z);
    //currentVertex = applyDistortion(currentVertex);
	gl_Position = u_Projection * u_View * u_Model * vec4(currentVertex, 1.0);
	v_TexCoords = a_TexCoord;
}


#type fragment
#version 460 core

in vec2 v_TexCoords;
uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoords) * vec4(1,1,1, 0.5);

    if(texColor.a <= 0) discard;
       
    gl_FragColor = texColor;
}