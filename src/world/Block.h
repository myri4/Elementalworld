#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Maths/AssimpGLMHelpers.h>
#include <Utils/Log.h>
#include "../Game Mechanics/Item.h"
#include <magic_enum.hpp>
<<<<<<< Updated upstream
#include <Utils/List.h>
#include <Utils/YAML.h>
#include <sol/sol.hpp>
#include <GUI/AssetManager.h>
=======
#include <wc/Utils/List.h>
#include <wc/Utils/YAML.h>
#include "../Rendering/AssetManager.h"
>>>>>>> Stashed changes
#include "../Globals.h"

namespace wc{

enum ConnectionType : uint8_t { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, 
	SLAB_DOWN, SLAB_UP, SLAB_LEFT, SLAB_RIGHT, SLAB_FRONT, SLAB_BACK,
	CANT_CONNECT, AIR, CUSTOM_MODEL, NON_EXISTENT};
enum class BlockTexture : uint8_t { RIGHT, TOP, FRONT, LEFT, BOTTOM, BACK, LENGTH };

const uint8_t WC_MODEL_BIT = 0b00000001;
const uint8_t WC_CULL_BIT = 0b00000010;

struct BlockMesh { // @TODO: Improve
	gl::DrawElementsIndirectCommand cmd;
	gl::Buffer vertexBuffer;
	gl::Buffer indexBuffer;

	BlockMesh() = default;

	void Load(const std::string& path, const uint32_t& materialID) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);
		
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			WC_ERROR(importer.GetErrorString());
			return;
		}
		uint32_t offset = 0;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		processNode(scene->mRootNode, *scene, offset, materialID, vertices, indices);
		uint32_t bits = 0;
		indexBuffer.Create(sizeof(uint32_t) * indices.size(), bits, indices.data());
		vertexBuffer.Create(sizeof(Vertex) * vertices.size(), bits, vertices.data());
		cmd.count = indices.size();
		cmd.instanceCount = 0;

		importer.FreeScene();
		vertices.clear();
		indices.clear();
	}

	void processNode(const aiNode* node, const aiScene& scene, uint32_t& offset, const uint32_t& materialID, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
		// process each mesh located at the current node		
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		for (uint32_t m = 0; m < node->mNumMeshes; m++) {
			auto& mesh = *scene.mMeshes[node->mMeshes[m]];
			for (uint32_t i = 0; i < mesh.mNumVertices; i++)
			{
				Vertex vertex;
				vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh.mVertices[i]) + glm::vec3(0.5f, 0.f, 0.5f);
				vertex.Normal = -AssimpGLMHelpers::GetGLMVec(mesh.mNormals[i]);
				vertex.materialID = materialID;

				if (mesh.mTextureCoords[0])
					vertex.TexCoords = glm::vec3(mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y, 0.f);

				vertices.push_back(vertex);
			}

			for (uint32_t i = 0; i < mesh.mNumFaces; i++)
			{
				aiFace& face = mesh.mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) 
					indices.push_back(face.mIndices[j] + offset);
			}

			offset += mesh.mNumVertices;
		}

		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			processNode(node->mChildren[i], scene, offset, materialID, vertices, indices);
	}
};

struct Material {
	uint32_t albedo[6] = { 0 };
	uint32_t materialData[6] = { 0 };
	uint32_t flags = 0;
	uint32_t color = 0xFFFFFFFF;
};

struct Block{
	bool emitLight : 1;
	ConnectionType connectionType = ConnectionType::AIR;

	bool isCollidable : 1;
	MeshID meshID = 0;
	uint8_t variations = 0;
	MaterialID material = 0;
	glm::vec3 rotation = glm::vec3(0.f);

	std::string name = "air";

	Block() {
		isCollidable = false;
		emitLight = false;
	}
};

List<Block, 40> blockData;
List<BlockMesh, 10> blockMeshes;
PointerList<Material, 40> materialData;

struct RarityTable {
	BlockID block = 0;

	float start = 0.f;
	float end = 0.f;
};
}