#ifndef GEOMETRY_GLSL
#define GEOMETRY_GLSL

#include "Utils.glsl"
#include "constants.glsl"

struct Node {
    vec3 min;
	uint first;
	vec3 max;
	uint primCount;
};

struct ChunkNode {
	vec3 min;
	uint flags;
	vec3 max;
	uint parentID; // not really used in shader just for convinience
	uint children[8];
};

const uint IS_LEAF_BIT =  1;
const uint IS_EMPTY_BIT = 2;

struct DrawCommand {
	vec3 transform;
	uint bvhID;
};

struct Vertex {
	vec3 Position;
	uint _pad1;
	vec3 Normal;
	uint _pad2;
	vec3 Tangent;
	uint _pad3;
	vec3 Bitangent;
	uint _pad4;
	vec2 TexCoords;
	uint materialID;
	uint _pad;
};

struct BufferMaterial {
	uint albedo;
	float metallic;
	float roughness;
	float reflectance;
	float clearCoat;
	float clearCoatRoughness;
	float anisotropy;
	//vec3 anisotropyDirection;
	float ior;
	float emissive;
};

struct HitInfo {
    float minT;
    uint materialID;
    vec3 uvw;
    vec2 texCoords;
    bool isHit;
    vec3 N;
    vec3 B;
    vec3 T;
    vec3 p;
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

Vertex GetVertex(uint indexOffset) {
	return vertices[indices[indexOffset]];
}

vec3 Intersection(vec3 rayOrigin, vec3 rayDirection, uint indexOffset) {
	vec3 a = vertices[indices[indexOffset + 0]].Position;
	vec3 b = vertices[indices[indexOffset + 1]].Position;
	vec3 c = vertices[indices[indexOffset + 2]].Position;
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

	return vec3(-1.f, uv);	
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

vec3 GetCubeTangent(int faceIndex)
{
  switch (faceIndex)
  {
  case 0:  return vec3( 0.f, 0.f, -1.f); // +X
  case 1:  return vec3( 0.f, 0.f,  1.f); // -X
  case 2:  return vec3( 1.f, 0.f,  0.f); // +Y
  case 3:  return vec3(-1.f, 0.f,  0.f); // -Y
  case 4:  return vec3( 1.f, 0.f,  0.f); // +Z
  default: return vec3(-1.f, 0.f,  0.f); // -Z
  }
}

vec3 GetCubeBitangent(int faceIndex)
{
  switch (faceIndex)
  {
  case 0:  return vec3(0.f, 1.f,  0.f); // +X
  case 1:  return vec3(0.f, 1.f,  0.f); // -X
  case 2:  return vec3(0.f, 0.f, -1.f); // +Y
  case 3:  return vec3(0.f, 0.f, -1.f); // -Y
  case 4:  return vec3(0.f, 1.f,  0.f); // +Z
  default: return vec3(0.f, 1.f,  0.f); // -Z
  }
}

vec2 aabb_intersect(vec3 ro, vec3 invRayDirection, vec3 bmin, vec3 bmax) {
    vec3 tbot = (bmin - ro) * invRayDirection;
    vec3 ttop = (bmax - ro) * invRayDirection;
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
    hitInfo.uvw = vec3(0.f);
    hitInfo.isHit = false;
    hitInfo.N = vec3(0.f);
    hitInfo.p = vec3(0.f);

    uint shapeHit = 0;
    bool hitCube = false;

    uint stack[128];
    uint stackSize = 0u;
    stack[stackSize++] = 0;
    ivec3 vStep = ivec3(sign(refRayDirection));
	vec3 vRayUnitStepSize = abs(invRayDirection);

    while (stackSize > 0u) {
        uint nodeIndex = stack[--stackSize];
        ChunkNode node = chunkNodes[nodeIndex];
        vec2 boxT = aabb_intersect(refRayOrigin, invRayDirection, node.min, node.max);
        if (!(boxT.x <= boxT.y && boxT.y > bias))
            continue;        
        
        if (bool(node.flags & IS_LEAF_BIT)) {
            // perform leaf node operation            

			vec3 childMin = node.min;

            float tNear = boxT.x;
            if (tNear < hitInfo.minT) {

                tNear = max(tNear, bias);
                float tFar = boxT.y - bias;

                ivec3 vMapCheck;
                vec3 vRayLength1D;

                {
                    vec3 p = refRayOrigin + tNear * refRayDirection;
                    vec3 dir = normalize(p - childMin + vec3(chunkSize));
                    int faceIndex = GetCubeFaceIndex(dir);
                    vec3 N = GetCubeNormal(faceIndex);

                    vMapCheck = ivec3(floor(p - N * bias));
                }

	            // Establish Starting Conditions                        
	        	vRayLength1D[0] = (refRayDirection[0] < 0.f ? (refRayOrigin[0] - float(vMapCheck[0])) : (float(vMapCheck[0]) + 1.f - refRayOrigin[0])) * vRayUnitStepSize[0];
	        	vRayLength1D[1] = (refRayDirection[1] < 0.f ? (refRayOrigin[1] - float(vMapCheck[1])) : (float(vMapCheck[1]) + 1.f - refRayOrigin[1])) * vRayUnitStepSize[1];
	        	vRayLength1D[2] = (refRayDirection[2] < 0.f ? (refRayOrigin[2] - float(vMapCheck[2])) : (float(vMapCheck[2]) + 1.f - refRayOrigin[2])) * vRayUnitStepSize[2];

                vMapCheck = getBlockPos(vMapCheck);

	            float fDistance = 0.f;
	            while (fDistance < tFar)
	            {
                    // Walk along shortest path
                    int axis = vRayLength1D.x < vRayLength1D.y ? 
                    (vRayLength1D.x < vRayLength1D.z ? 0 : 2) : (vRayLength1D.y < vRayLength1D.z ? 1 : 2);

	        	    // Test tile at new test point       	        
                    uint blockHit = uint(blockIDs[to1D(vMapCheck, chunkSize) + node.children[0]]);
                    
	        	    if (blockHit != 0)
	        	    {                            
                        hitCube = true;
                        hitInfo.isHit = true;
                        fDistance = fDistance == 0.f ? tNear : fDistance + bias;

                        hitInfo.minT = fDistance;
                        hitInfo.p = refRayOrigin + fDistance * refRayDirection;

                        vec3 center = vec3(vMapCheck) + childMin + 0.5f;
                        vec3 dir = hitInfo.p - center; // if the size is different from 1 this should be normalized

                        int faceIndex = GetCubeFaceIndex(dir);
                        hitInfo.materialID = blockData[blockHit].materialIDs[faceIndex];
                        hitInfo.N = GetCubeNormal(faceIndex);
                        hitInfo.T = GetCubeTangent(faceIndex);
                        hitInfo.B = GetCubeBitangent(faceIndex);
                        hitInfo.uvw.xy = GetCubeUVFromDir(faceIndex, dir);

	        		    break;
	        	    }


	        	    vMapCheck[axis] += vStep[axis];
	        	    fDistance = vRayLength1D[axis];
	        	    vRayLength1D[axis] += vRayUnitStepSize[axis];		
	            }                    
            }			
        }
        else{

            for (int i = 7; i >= 0; i--) {
                uint childIndex = node.children[i];
                if (childIndex == 0u) 
                    continue;

                if (bool(node.flags & IS_EMPTY_BIT))
                    continue;
                
                stack[stackSize++] = childIndex;
            }
        }
    }

    for (int j = 0; j < u_NumDrawCommands; j++) {
        vec3 rayOrigin = refRayOrigin - drawCommands[j].transform;
        Node node = nodes[drawCommands[j].bvhID];
        vec2 t = aabb_intersect(rayOrigin, invRayDirection, node.min, node.max);
        if (t.x <= t.y && t.y > 0.f && t.x < hitInfo.minT) {
            for (uint i = node.first; i < node.primCount + node.first; i += 3) {

                vec3 intInfo = Intersection(rayOrigin, refRayDirection, i);

				if (intInfo.x > 0.f && intInfo.x < hitInfo.minT) {			
					hitInfo.minT = intInfo.x;
					hitInfo.uvw.xy = intInfo.yz;
					hitInfo.isHit = true;
                    shapeHit = i;
                    hitCube = false;
				}
            }
        }
    }



    if (hitInfo.isHit) {
		hitInfo.uvw.z = 1.f - hitInfo.uvw.x - hitInfo.uvw.y;

        if (!hitCube){
		    hitInfo.N = GetVertex(shapeHit).Normal;
		    hitInfo.T = GetVertex(shapeHit).Tangent;
		    hitInfo.B = GetVertex(shapeHit).Bitangent;
            hitInfo.materialID = GetVertex(shapeHit).materialID;
			hitInfo.p = refRayOrigin + hitInfo.minT * refRayDirection;
            hitInfo.texCoords = rtLerp(hitInfo.uvw, GetVertex(shapeHit + 1).TexCoords, 
            GetVertex(shapeHit + 2).TexCoords, GetVertex(shapeHit).TexCoords);		
        }
        else
            hitInfo.texCoords = hitInfo.uvw.xy;

        hitInfo.N = dot(refRayDirection, hitInfo.N) < 0 ? hitInfo.N : -hitInfo.N; 
        hitInfo.T = dot(refRayDirection, hitInfo.T) < 0 ? hitInfo.T : -hitInfo.T;
    }

	return hitInfo;
}

struct Ray {
    vec3 org;
    vec3 dir;
};

struct Hit {
    uint id;      // Primitive ID
    vec2 data; // Per-primitive hit information
};

// Note: The layout used for the BVH nodes is such that `first` points to
// the first child of the node for an inner node. The second child is
// assumed to be located at `first + 1`.
// If the children of a node are both leaves, their primitives should form
// a contiguous range.

bool is_leaf(Node node) { return node.primCount > 0; }

vec2 intersect_ray_box(vec3 org, vec3 inv_dir, vec3 box_min, vec3 box_max, float tnear, float tfar) {
    vec3 tmin = (box_min - org) * inv_dir;
    vec3 tmax = (box_max - org) * inv_dir;
    vec3 t0 = min(tmin, tmax);
    vec3 t1 = max(tmin, tmax);
    return vec2(
        max(t0.x, max(t0.y, max(t0.z, tnear))),
        min(t1.x, min(t1.y, min(t1.z, tfar))));
}

// Before using this function, make sure your BVH data layout matches those requirements:
// 1. The BVH is not just a single leaf,
// 2. The two children of one inner node are placed contiguously,
// 3. If one inner node has two leaves as children, their primitives form a contiguous range.
Hit intersect_ray_bvh(bool is_any, Ray ray, float tnear, inout float tfar) {
    Hit hit;

    

    return hit;
}

#endif