#ifndef GEOMETRY_GLSL
#define GEOMETRY_GLSL

#include "Utils.glsl"
#include "constants.glsl"

struct Node {
    vec3 start;
	uint left;
    vec3 end;
	uint right;
	uint first;
	uint count;
	uint flags;
	uint _pad;
};

struct ChunkNode {
	vec3 start;
	uint isLeaf;
	vec3 end;
	uint parentID; // not really used in shader just for convinience
	uint children[8];
};

struct DrawCommand {
	uint count;
	uint firstIndex;
	uint bvhID;
	uint baseVertex;
	vec3 transform;
	uint _pad1;
};

struct Vertex {
	vec3 Position;
	uint materialID;
	vec2 TexCoords;
	uint _pad2[2];
	vec3 Normal;
	uint _pad3;
};

struct BufferMaterial {
	uint albedo;
	float metallic;
	float roughness;
	float reflectance;
	//float clearCoat;
	//float clearCoatRoughness;
	//float anisotropy;
	//vec3 anisotropyDirection;
	//float ior;
	float emissive;
};

struct HitInfo {
    float minT;
    uint shapeHit;
    uint blockHit;
    uint materialID;
    vec3 uvw;
    bool isHit;
    bool hitCube;
    vec3 N;
    vec3 p;
    DrawCommand cmd;
};

struct Block {
	uint materialIDs[6];
};

layout (std430, binding = 1) readonly buffer VertexBuffer { Vertex vertices[]; };
layout (std430, binding = 2) readonly buffer IndexBuffer { uint indices[]; };
layout (std430, binding = 3) readonly buffer DrawCommandBuffer { DrawCommand drawCommands[]; };
layout (std430, binding = 4) readonly buffer BVHBuffer { Node nodes[]; };
layout (std430, binding = 5) readonly buffer ChunkNodeBuffer { ChunkNode chunkNodes[]; };
layout (std430, binding = 6) readonly buffer MaterialData { BufferMaterial materials[]; };
layout (std430, binding = 7) readonly buffer BlockIDs { uint8_t blockIDs[]; };
layout (std430, binding = 8) readonly buffer Blocks { Block blockData[]; };

Vertex GetVertex(uint indexOffset, DrawCommand cmd) {
	return vertices[indices[indexOffset] + cmd.baseVertex];
}

vec3 Intersection(vec3 rayOrigin, vec3 rayDirection, uint indexOffset, DrawCommand cmd) {
	vec3 a = vertices[indices[indexOffset + 0] + cmd.baseVertex].Position + cmd.transform;
	vec3 b = vertices[indices[indexOffset + 1] + cmd.baseVertex].Position + cmd.transform;
	vec3 c = vertices[indices[indexOffset + 2] + cmd.baseVertex].Position + cmd.transform;
	vec3 e1 = b - a;
	vec3 e2 = c - a;
    vec3 crossRDE2 = cross(rayDirection, e2);
    float dotE1CrossRDE2 = 1.f / dot(e1, crossRDE2);
	vec3 rOa = rayOrigin - a;
	vec3 crossROAE1 = cross(rOa, e1);
	vec2 uv;
	uv.x = dot(rOa, crossRDE2) * dotE1CrossRDE2;
	uv.y = dot(rayDirection, crossROAE1 * dotE1CrossRDE2);

	float t = dot(e2, crossROAE1) * dotE1CrossRDE2;
		if (!(t <= 0.f || uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.x + uv.y > 1.f))
			return vec3(t, uv);
		else 
			return vec3(0.f, uv);	
}

bool isCoordInBounds(const in ivec3 coords) {
	return coords.x >= 0 && coords.x < chunkSize &&
		   coords.y >= 0 && coords.y < chunkSize &&
		   coords.z >= 0 && coords.z < chunkSize;
}

ivec3 getBlockPos(const in int x, const in int y, const in int z)
{
	return ivec3( (chunkSize + (x % chunkSize)) % chunkSize,
				  (chunkSize + (y % chunkSize)) % chunkSize,
				  (chunkSize + (z % chunkSize)) % chunkSize );
}

ivec3 getBlockPos(const in ivec3 pos)
{
	return getBlockPos(pos.x, pos.y, pos.z);
}

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
  case 0: uv =  vec2(-dir.z, -dir.y); break; // +X
  case 1: uv =  vec2( dir.z, -dir.y); break; // -X
  case 2: uv =  vec2( dir.x,  dir.z); break; // +Y
  case 3: uv =  vec2( dir.x, -dir.z); break; // -Y
  case 4: uv =  vec2( dir.x, -dir.y); break; // +Z
  default: uv = vec2(-dir.x, -dir.y); break; // -Z
  }
  return uv + 0.5f;
}

vec3 GetCubeNormal(int faceIndex)
{
  switch (faceIndex)
  {
  case 0:  return vec3( 1.f,  0.f,  0.f); // +X
  case 1:  return vec3(-1.f,  0.f,  0.f); // -X
  case 2:  return vec3( 0.f,  1.f,  0.f); // +Y
  case 3:  return vec3( 0.f, -1.f,  0.f); // -Y
  case 4:  return vec3( 0.f,  0.f,  1.f); // +Z
  default: return vec3( 0.f,  0.f, -1.f); // -Z
  }
}

vec2 aabb_intersect(vec3 ro, vec3 rrd, vec3 bmin, vec3 bmax) {
    vec3 tbot = (bmin - ro) * rrd;
    vec3 ttop = (bmax - ro) * rrd;
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    float t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    float t1 = min(t.x, t.y);

    return vec2(t0, t1);
}

bool raySphereIntersect(vec3 rayOrigin, vec3 rayDirection, float radius, vec3 position, inout float t0, inout float t1) {
	vec3 L = position - rayOrigin;
	float tca = dot(L, rayDirection);

	//if (tca < 0) return false;

	float s2 = (dot(L, L)) - (tca * tca);

	if (s2 > radius * radius) return false;	

	float thc = sqrt((radius * radius) - s2);
	t0 = tca - thc; 
    t1 = tca + thc;

	if (t0 > t1) {
		float t = t0;
		t0 = t1;
		t1 = t;
	}

	return true;
}

HitInfo intersect(const in vec3 refRayOrigin, const in vec3 refRayDirection, const in vec3 invRayDirection) {
	HitInfo hitInfo;
    hitInfo.minT = 1000.f;
    hitInfo.shapeHit = 0;
    hitInfo.blockHit = 0;
    hitInfo.uvw = vec3(0.f);
    hitInfo.isHit = false;
    hitInfo.hitCube = false;
    hitInfo.N = vec3(0.f);
    hitInfo.p = vec3(0.f);

    uint stack[75];
    uint stackSize = 0u;
    stack[stackSize++] = 0;
    ivec3 vStep = ivec3(sign(refRayDirection));
	vec3 vRayUnitStepSize = abs(invRayDirection);

    while (stackSize > 0u) {
        uint nodeIndex = stack[--stackSize];
        ChunkNode node = chunkNodes[nodeIndex];
        float tmin, tmax;
        vec2 boxT = aabb_intersect(refRayOrigin, invRayDirection, node.start, node.end);
        if (!(boxT.x <= boxT.y && boxT.y > 0.f))
            continue;        
        
        if (node.isLeaf == 1u) {
            // perform leaf node operation

            vec3 parentStart = node.start;
			vec3 parentEnd = node.end;
			vec3 mid = (parentStart + parentEnd) * 0.5f;

            for (int i = 7; i >= 0; i--) {
				vec3 childStart = parentStart;
				vec3 childEnd = parentEnd;
				if (bool(i & 1)) { childStart.x = mid.x; }
				else { childEnd.x = mid.x; }
				if (bool(i & 2)) { childStart.y = mid.y; }
				else { childEnd.y = mid.y; }
				if (bool(i & 4)) { childStart.z = mid.z; }
				else { childEnd.z = mid.z; }

                vec2 t = aabb_intersect(refRayOrigin, invRayDirection, childStart, childEnd);
                float tNear = t.x;
                float tFar = t.y;
                if (tNear <= tFar && tFar > 0.f && tNear < hitInfo.minT) {

                        tNear = max(tNear, 0.f);
                        tNear += bias;
                        tFar += bias;                 


                        ivec3 vMapCheck;// = ivec3(floor(refRayOrigin + tNear * refRayDirection));
                        vec3 vRayLength1D;

                        {
                        vec3 p = refRayOrigin + tNear * refRayDirection;
                        vec3 dir = normalize(p - childStart + vec3(chunkSize));
                        int faceIndex = GetCubeFaceIndex(dir);
                        vec3 N = GetCubeNormal(faceIndex);

                        vMapCheck = ivec3(floor(p - N * bias));
                        }

	                    // Establish Starting Conditions
                        [[unroll]]
	                    for (int i = 0; i < 3; i++)	
	            		    vRayLength1D[i] = refRayDirection[i] < 0.f ? (refRayOrigin[i] - float(vMapCheck[i])) : (float(vMapCheck[i] + 1) - refRayOrigin[i]);

                        vRayLength1D *= vRayUnitStepSize;
                        vMapCheck = getBlockPos(vMapCheck);

	                    float fDistance = 0.f;
	                    while (fDistance < tFar)
	                    {
                            // Walk along shortest path
                            int axis = vRayLength1D.x < vRayLength1D.y ? 
                            (vRayLength1D.x < vRayLength1D.z ? 0 : 2) : (vRayLength1D.y < vRayLength1D.z ? 1 : 2);

	            	        // Test tile at new test point
	            	        if(isCoordInBounds(vMapCheck)) {
                                hitInfo.blockHit = uint(blockIDs[to1D(vMapCheck, chunkSize) + node.children[i] * chunkVolume]);
                                
	            	        	if (hitInfo.blockHit != 0)
	            	        	{                            
                                    hitInfo.hitCube = true;
                                    hitInfo.isHit = true;
                                    fDistance = fDistance == 0.f ? tNear : fDistance += bias;

                                    hitInfo.minT = fDistance;
                                    hitInfo.p = refRayOrigin + fDistance * refRayDirection;

                                    vec3 center = vec3(vMapCheck) + childStart + 0.5f;
                                    vec3 dir = hitInfo.p - center; // if the size is different from 1 this should be normalized

                                    int faceIndex = GetCubeFaceIndex(dir);
                                    hitInfo.materialID = blockData[hitInfo.blockHit].materialIDs[faceIndex];
                                    hitInfo.N = GetCubeNormal(faceIndex);
                                    hitInfo.uvw.xy = GetCubeUVFromDir(faceIndex, dir);
									hitInfo.uvw.z = 1.f - hitInfo.uvw.x - hitInfo.uvw.y;

	            			        break;
	            		        }
	            	        }
                            else break;


	            	        vMapCheck[axis] += vStep[axis];
	            	        fDistance = vRayLength1D[axis];
	            	        vRayLength1D[axis] += vRayUnitStepSize[axis];		
	                    }

                    
                }
			}
        }
        else{

            for (int i = 7; i >= 0; i--) {
                uint childIndex = node.children[i];
                if (childIndex == 0u) 
                    continue;
                
                stack[stackSize++] = childIndex;
            }
        }
    }

    for (int j = 0; j < u_NumDrawCommands; j++) {
        float boxT = 0.f;
        vec2 t = aabb_intersect(refRayOrigin, invRayDirection, nodes[drawCommands[j].bvhID].start + drawCommands[j].transform, 
        nodes[drawCommands[j].bvhID].end + drawCommands[j].transform);
        if (t.x <= t.y && t.y > 0.f && t.x < hitInfo.minT) {
            for (uint i = drawCommands[j].firstIndex; i < drawCommands[j].count + drawCommands[j].firstIndex; i += 3) {

                vec3 intInfo = Intersection(refRayOrigin, refRayDirection, i, drawCommands[j]);

				if (intInfo.x > 0.f && intInfo.x < hitInfo.minT) {			
					hitInfo.minT = intInfo.x;
					hitInfo.uvw.xy = intInfo.yz;
					hitInfo.uvw.z = 1.f - hitInfo.uvw.x - hitInfo.uvw.y;
					hitInfo.isHit = true;
                    hitInfo.shapeHit = i;
                    hitInfo.cmd = drawCommands[j];
                    hitInfo.materialID = GetVertex(hitInfo.shapeHit, hitInfo.cmd).materialID;
                    hitInfo.hitCube = false;

			        hitInfo.N = GetVertex(hitInfo.shapeHit, hitInfo.cmd).Normal;
				}
            }
        }
    }

    hitInfo.N = dot(refRayDirection, hitInfo.N) < 0 ? hitInfo.N : -hitInfo.N; 

	return hitInfo;
}
#endif