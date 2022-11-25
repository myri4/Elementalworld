struct AABB{
    vec4 start;
    vec4 end;
};

struct Vertex {
	vec3 Position;
	uint materialID;
	vec3 TexCoords;
	uint _pad1;
	vec3 Normal;
	uint _pad2;
};

layout (std430, binding = 2) readonly buffer ChunkBVHBuffer { AABB chunkBvh[]; };
layout (std430, binding = 3) readonly buffer VertexBuffer { Vertex vertices[]; };

int GetCubeFaceIndex(vec3 dir)
{
  float x = abs(dir.x);
  float y = abs(dir.y);
  float z = abs(dir.z);
  if (x > y && x > z)
    return 0 + (dir.x > 0 ? 0 : 1);
  else if (y > z)
    return 2 + (dir.y > 0 ? 0 : 1);
  return 4 + (dir.z > 0 ? 0 : 1);
}

vec2 GetCubeUVFromDir(int faceIndex, vec3 dir)
{
  vec2 uv;
  switch (faceIndex)
  {
    case 0:  uv = vec2(-dir.z,  dir.y); break; // +X
    case 1:  uv = vec2( dir.z,  dir.y); break; // -X
    case 2:  uv = vec2( dir.x, -dir.z); break; // +Y
    case 3:  uv = vec2( dir.x,  dir.z); break; // -Y
    case 4:  uv = vec2( dir.x,  dir.y); break; // +Z
    default: uv = vec2(-dir.x,  dir.y); break; // -Z
  }
  return uv * .5 + .5;
}

vec3 BoxNormal(AABB box, const vec3 point)
{
	vec3 center = (box.end.xyz + box.start.xyz) * 0.5f;
	vec3 size = (box.end.xyz - box.start.xyz) * 0.5f;
	vec3 pc = point - center;
	// step(edge,x) : x < edge ? 0 : 1
	vec3 normal = vec3(0.0);
	normal += vec3(sign(pc.x), 0.0, 0.0) * step(abs(abs(pc.x) - size.x), 1.0e-4);
	normal += vec3(0.0, sign(pc.y), 0.0) * step(abs(abs(pc.y) - size.y), 1.0e-4);
	normal += vec3(0.0, 0.0, sign(pc.z)) * step(abs(abs(pc.z) - size.z), 1.0e-4);
	return normalize(normal);
}

bool BoxIntersect(vec3 rayOrigin, vec3 invRayDir, vec3 boxMin, vec3 boxMax, out float t) {
    vec3 tMin = (boxMin - rayOrigin) * invRayDir;
    vec3 tMax = (boxMax - rayOrigin) * invRayDir;
	for (int a = 0; a < 3; a++){
		if (invRayDir[a] < 0.f){
			float temp = tMin[a];
			tMin[a] = tMax[a];
			tMax[a] = temp;
		}
	}
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    if (tNear <= tFar) t = tNear;
    else t = tFar;
    return tNear <= tFar && tFar > 0.f;
}