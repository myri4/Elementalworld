#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Maths/AssimpGLMHelpers.hpp>
#include <Utils/Log.hpp>

namespace wc{

enum ConnectionType : uint8_t { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, X_CONNECT, CANT_CONNECT};
enum class BlockTexture : uint8_t { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK };

const float blockSize = 1.f;

struct Face {
	glm::vec3 corner1;
	glm::vec3 corner2;
	glm::vec3 corner3;
	glm::vec3 corner4;
	BlockTexture texID;
	glm::vec3 normal;

	void CalculateNormal() {
		normal = glm::cross(corner3 - corner1, corner2 - corner1);		
	}
};

uint32_t convertColor(const glm::vec4& color) {
	return (uint32_t)(color.r * 255.f) << 24 | (uint32_t)(color.g * 255.f) << 16 | (uint32_t)(color.b * 255.f) << 8 | (uint32_t)(color.a * 255.f);
}

glm::vec4 convertColor(const uint32_t& color) {
	const float c = 1.f / 255.f;
	glm::vec4 Color;
	Color.r = float((color & uint32_t(0xff000000)) >> 24) * c;
	Color.g = float((color & uint32_t(0x00ff0000)) >> 16) * c;
	Color.b = float((color & uint32_t(0x0000ff00)) >> 8) * c;
	Color.a = float((color & uint32_t(0x000000ff))) * c;
	return Color;
}

class Vertex {
public:
	glm::vec3 Position = { 0,0,0 };
	glm::vec3 TexCoords = { 0,0,0 };
	uint32_t color = 0xFFFFFFFF;
	uint32_t NormalType = 0;
	Vertex() {}
	Vertex(const glm::vec3& pos, const glm::vec3& texCoord, const uint8_t& Type, const uint32_t& Color, const glm::vec3& normal) : Position(pos), TexCoords(texCoord), color(Color) {

		NormalType = (uint32_t)(normal.r * 255.f) << 24 | (uint32_t)(normal.g * 255.f) << 16 | (uint32_t)(normal.b * 255.f) << 8 | (uint32_t)(Type);
	}
};

struct BlockMesh {
	std::vector<Vertex> vertices;

	void Load(const char* path) {
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			WC_ERROR(import.GetErrorString());
			return;
		}
		processNode(scene->mRootNode, *scene);
	}

	void processNode(const aiNode* node, const aiScene& scene) {
		// process each mesh located at the current node		
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		for (uint32_t j = 0; j < node->mNumMeshes; j++) {
			auto& mesh = scene.mMeshes[node->mMeshes[j]];
			vertices.reserve(mesh->mNumVertices);
			for (uint32_t i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
				//vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
				//vertex.color = convertColor(AssimpGLMHelpers::GetGLMVec(mesh->mColors[0][i]));

				vertices.emplace_back(vertex);
			}
			std::vector<uint32_t> indices;
			indices.reserve(mesh->mNumFaces);
			for (uint32_t i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace& face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) {
					indices.emplace_back(face.mIndices[j]);
					WC_INFO(face.mIndices[j]);
				}
			}
		}

		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			processNode(node->mChildren[i], scene);
	}
}BlockMeshes;

typedef int8_t BlockID;

Face BACK_FACE = {
	glm::vec3( 0.f,  blockSize, 0.f), // top-left
	glm::vec3( 0.f, 0.f, 0.f), // Bottom-left  
	glm::vec3( blockSize, 0.f, 0.f), // bottom-right 
	glm::vec3( blockSize,  blockSize, 0.f), // top-right
	BlockTexture::BACK
};

Face FRONT_FACE = {
	glm::vec3( blockSize,  blockSize,  blockSize), // top-right
	glm::vec3( blockSize, 0.f,  blockSize), // bottom-right        
	glm::vec3( 0.f, 0.f,  blockSize), // bottom-left
	glm::vec3( 0.f,  blockSize,  blockSize), // top-left   
	BlockTexture::FRONT
};

Face LEFT_FACE = {
	glm::vec3(0.f,  blockSize,  blockSize),  // top-right
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-right
	glm::vec3(0.f, 0.f, 0.f),  // bottom-left 
	glm::vec3(0.f,  blockSize, 0.f),  // top-left
	BlockTexture::LEFT
};

Face RIGHT_FACE = {
	 glm::vec3(blockSize,  blockSize, 0.f),  // top-right      
	 glm::vec3(blockSize, 0.f, 0.f),  // bottom-right          
	 glm::vec3(blockSize, 0.f,  blockSize),  // bottom-left
	 glm::vec3(blockSize,  blockSize,  blockSize),  // top-left
	 BlockTexture::RIGHT
};

Face BOTTOM_FACE = {
	glm::vec3(0.f, 0.f, 0.f),  // top-right 
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-right
	glm::vec3( blockSize, 0.f,  blockSize),  // bottom-left
	glm::vec3( blockSize, 0.f, 0.f),  // top-left  
	BlockTexture::BOTTOM
};

Face TOP_FACE = {
	glm::vec3( blockSize,  blockSize, 0.f), // top-right
	glm::vec3( blockSize,  blockSize,  blockSize), // bottom-right                 
	glm::vec3(0.f,  blockSize,  blockSize), // bottom-left  
	glm::vec3(0.f,  blockSize, 0.f), // top-left 
	BlockTexture::TOP
};

Face X_FACE1 = {
	glm::vec3( blockSize,  blockSize, 0.f),  // top-right      
	glm::vec3( blockSize, 0.f, 0.f),  // bottom-right          
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-left
	glm::vec3(0.f,  blockSize,  blockSize),  // top-left
	BlockTexture::TOP
};

Face X_FACE2 = {
	glm::vec3( blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3( blockSize, 0.f,  blockSize),  // bottom-right
	glm::vec3(0.f, 0.f, 0.f),  // bottom-left 
	glm::vec3(0.f,  blockSize, 0.f),  // top-left
	BlockTexture::TOP
};

struct Block{
	bool isCollidable : 1;
	bool emitLight : 1;

	uint32_t texture[6] = {0};
	uint32_t normalTexture[6] = { 0 };
	int32_t meshID = -1;
	uint8_t blockConnectionType = ConnectionType::CONNECT_DEFAULT;

	Block() {
		isCollidable = true;
		emitLight = false;
	}
};
}