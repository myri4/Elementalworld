#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Maths/AssimpGLMHelpers.hpp>
#include <Utils/Log.hpp>

namespace wc{

enum ConnectionType : uint8_t { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, X_CONNECT, CANT_CONNECT, CUSTOM_MODEL, AIR, NON_EXISTENT};
enum class BlockTexture : uint8_t { RIGHT, TOP, FRONT, LEFT, BOTTOM, BACK };

const float blockSize = 1.f;

struct Face {
	glm::vec3 corner[4];
	uint32_t texID;
	glm::vec3 normal;

	void CalculateNormal() {
		normal = glm::normalize(glm::cross(corner[2] - corner[0], corner[1] - corner[0]));		
	}
};

uint32_t convertColor(const glm::vec4& color) {
	int32_t r = color.r * 255.f;
	int32_t g = color.g * 255.f;
	int32_t b = color.b * 255.f;
	int32_t a = color.a * 255.f;
	return a << 24 | b << 16 | g << 8 | r;
}

glm::vec4 convertColor(const uint32_t& color) {
	const float c = 1.f / 255.f;
	glm::vec4 Color;
	Color.r = float((uint32_t)(color & uint32_t(0x000000ff))) * c;
	Color.g = float((uint32_t)(color & uint32_t(0x0000ff00)) >> 8) * c;
	Color.b = float((uint32_t)(color & uint32_t(0x00ff0000)) >> 16) * c;
	Color.a = float((uint32_t)(color & uint32_t(0xff000000)) >> 24) * c;

	return Color;
}

Face X_FACE1 = {
	glm::vec3(blockSize,  blockSize, 0.f),  // top-right      
	glm::vec3(blockSize, 0.f, 0.f),  // bottom-right          
	glm::vec3(0.f, 0.f,  blockSize),  // bottom-left
	glm::vec3(0.f,  blockSize,  blockSize),  // top-left
};

Face X_FACE2 = {
	glm::vec3(blockSize,  blockSize,  blockSize),  // top-right
	glm::vec3(blockSize, 0.f,  blockSize),  // bottom-right
	glm::vec3(0.f, 0.f, 0.f),  // bottom-left 
	glm::vec3(0.f,  blockSize, 0.f),  // top-left
};

class Vertex {
public:
	glm::vec3 Position = { 0,0,0 };
	uint32_t TexCoords = 0;
	uint32_t color = 0xFFFFFFFF;
	glm::vec3 Normal = { 0,0,0 };
	Vertex() {}
	Vertex(const glm::vec3& pos, const glm::vec3& texCoord, const uint8_t& Type, const uint32_t& Color, const glm::vec3& normal) : Position(pos), color(Color), Normal(normal) {
		TexCoords = convertColor(glm::vec4(texCoord / 255.f, Type));
	}
};

struct BlockMesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	void Load(const char* path) {
		Assimp::Importer import;
		const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals);
		
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
				glm::mat4 model = glm::mat4(1.f);
				glm::vec3 pos = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
				model = glm::scale(model, glm::vec3(0.31f)); // magica voxel mesh size - 1
				vertex.Position = glm::vec3(glm::vec4(pos.x, pos.z, pos.y, 0.f) * model) ;
				glm::vec3 normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
				vertex.Normal = glm::vec3(normal.x, normal.z, normal.y);
				vertex.color = convertColor(AssimpGLMHelpers::GetGLMVec(mesh->mColors[0][i]));

				vertices.emplace_back(vertex);
			}
			indices.reserve(mesh->mNumFaces);
			for (uint32_t i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace& face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) 
					indices.emplace_back(face.mIndices[j]);
			}
		}

		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			processNode(node->mChildren[i], scene);
	}
}BlockMeshes;

typedef int8_t BlockID;

struct Block{
	bool isCollidable : 1;
	bool emitLight : 1;

	uint32_t texture[6] = {0};
	uint32_t normalTexture[6] = { 0 };
	uint32_t color = 0xFFFFFFFF;
	int32_t meshID = -1;
	uint8_t connectionType = ConnectionType::AIR;

	Block() {
		isCollidable = true;
		emitLight = false;
	}
};
}