#ifndef UTILS_GLSL
#define UTILS_GLSL

float rtLerp(const in vec3 uvw, const in float a, const in float b, const in float c) { return uvw.x * a + uvw.y * b + uvw.z * c; }
vec2 rtLerp(const in vec3 uvw, const in vec2 a, const in vec2 b, const in vec2 c) { return uvw.x * a + uvw.y * b + uvw.z * c; }
vec3 rtLerp(const in vec3 uvw, const in vec3 a, const in vec3 b, const in vec3 c) { return uvw.x * a + uvw.y * b + uvw.z * c; }
vec4 rtLerp(const in vec3 uvw, const in vec4 a, const in vec4 b, const in vec4 c) { return uvw.x * a + uvw.y * b + uvw.z * c; }

uint convertColor(const in vec4 color) { // Remember! Convert from 0-1 to 0-255!
	int r = int(color.r);
	int g = int(color.g);
	int b = int(color.b);
	int a = int(color.a);
	return a << 24 | b << 16 | g << 8 | r;
}

vec4 decompress(const in uint num) { // Remember! Convert from 0-255 to 0-1!
    vec4 Output;
    Output.r = float((num & uint(0x000000ff)));
    Output.g = float((num & uint(0x0000ff00)) >> 8);
    Output.b = float((num & uint(0x00ff0000)) >> 16);
    Output.a = float((num & uint(0xff000000)) >> 24);
    return Output;
}

// NOTE: x and z are swapped here cuz idk... gpus...
int to1D(const in ivec3 pos, const in uint size) { return int((pos.x * size * size) + (pos.y * size) + pos.z); }
	
ivec3 to3D(int i, const in ivec3 size) {
	int z = i / (size.x * size.y);
	i -= (z * size.x * size.y);
	int y = i / size.x;
	int x = i % size.x;
	return ivec3(x, y, z);
}

#endif