#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Maths/AssimpGLMHelpers.hpp>
#include <Utils/Log.hpp>
#include "../Game Mechanics/Item.hpp"
#include <magic_enum.hpp>
#include <Utils/List.hpp>

namespace wc{

enum ConnectionType : uint8_t { CONNECT_DEFAULT, FLUID_CONNECT, NO_CONNECT, 
	SLAB_DOWN, SLAB_UP, SLAB_LEFT, SLAB_RIGHT, SLAB_FRONT, SLAB_BACK,
	CANT_CONNECT, CUSTOM_MODEL, AIR, NON_EXISTENT};
enum class BlockTexture : uint8_t { RIGHT, TOP, FRONT, LEFT, BOTTOM, BACK };

const float blockSize = 1.f;

struct Face {
	glm::vec3 corner[4] = { glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f) , glm::vec3(0.f) };
	glm::vec3 normal = glm::vec3(0.f);

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

struct Vertex {
	glm::vec3 Position = { 0,0,0 };
	uint32_t TexCoords = 0;
	uint32_t materialID = 0;
	glm::vec3 Normal = { 0,0,0 };
	Vertex() {}
	Vertex(const glm::vec3& pos, const glm::vec3& texCoord, const uint8_t& Type, const glm::vec3& normal, const uint32_t& mat) : Position(pos), Normal(normal), materialID(mat) {
		TexCoords = convertColor(glm::vec4(texCoord / 255.f, Type));
	}
};

struct BlockMesh {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	bool optimize : 1;

	BlockMesh() {
		optimize = false;
	}

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
				//vertex.color = convertColor(AssimpGLMHelpers::GetGLMVec(mesh->mColors[0][i]));

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
};

using BlockID = glm::uint8_t;
using MaterialID = glm::uint8_t;

struct Material {
	//alignas(16) uint32_t albedo[6] = { 0 };
	//alignas(16) uint32_t normal[6] = { 0 };
	//alignas(16) uint32_t MRA[6] = { 0 };
	alignas(16) uint32_t flags = 0;
	uint32_t color = 0xFFFFFFFF;
};

struct Block{
	bool isCollidable : 1;
	bool emitLight : 1;

	uint32_t texture[6] = { 0 };
	int32_t meshID = -1;
	ConnectionType connectionType = ConnectionType::AIR;
	uint8_t variations = 0;
	MaterialID material = 0;

	std::string name = "air";

	Block() {
		isCollidable = false;
		emitLight = false;
	}
};

List<Block, 40> blockData;
List<BlockMesh, 3> blockMeshes;
List<Material, 40> materialData;

void AddBlockScript(const char* script) {
	std::string conType;
	sol::state blockState;
	blockState.script_file(script);

	Block block;
	Material mat;

	if (blockState["name"].valid()) block.name = blockState["name"];
	else WC_WARN("No block name is specified in '{0}'. Block name 'air' assumed.", script);

	if (blockState["isCollidable"].valid()) block.isCollidable = blockState["isCollidable"];
	if (blockState["ConnectionType"].valid()) conType = blockState["ConnectionType"];
	if (blockState["color"].valid()) mat.color = blockState["color"];

	for (uint8_t i = 0; i < ConnectionType::NON_EXISTENT; i++)
		if (conType == magic_enum::enum_name((ConnectionType)i)) block.connectionType = (ConnectionType)i;
	mat.flags = block.connectionType;

	if (block.connectionType == ConnectionType::CUSTOM_MODEL) block.meshID = 0;

	std::string diffusePath = "assets/textures/block/diffuse/";
	std::string normalPath = "assets/textures/block/normal/";

	if (blockState["allTextures"].valid()) {
		std::string path = blockState["allTextures"];
		block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(diffusePath + path);
		block.texture[(int)BlockTexture::BOTTOM] = block.texture[(int)BlockTexture::TOP];
		block.texture[(int)BlockTexture::FRONT] = block.texture[(int)BlockTexture::TOP];
		block.texture[(int)BlockTexture::BACK] = block.texture[(int)BlockTexture::TOP];
		block.texture[(int)BlockTexture::LEFT] = block.texture[(int)BlockTexture::TOP];
		block.texture[(int)BlockTexture::RIGHT] = block.texture[(int)BlockTexture::TOP];

		load((diffusePath + path).c_str(), items[numItems].texture);
	}
	else {
		std::string itemPath;
		std::string path;
		if (blockState["top"].valid()) {
			itemPath = blockState["top"];
			path = blockState["top"]; block.texture[(int)BlockTexture::TOP] = assets.LoadTexture(diffusePath + path);
		}
		if (blockState["bottom"].valid()) {
			path = blockState["bottom"]; block.texture[(int)BlockTexture::BOTTOM] = assets.LoadTexture(diffusePath + path);
		}
		if (blockState["front"].valid()) {
			path = blockState["front"];  block.texture[(int)BlockTexture::FRONT] = assets.LoadTexture(diffusePath + path);
		}
		if (blockState["back"].valid()) {
			path = blockState["back"];   block.texture[(int)BlockTexture::BACK] = assets.LoadTexture(diffusePath + path);
		}
		if (blockState["left"].valid()) {
			path = blockState["left"];   block.texture[(int)BlockTexture::LEFT] = assets.LoadTexture(diffusePath + path);
		}
		if (blockState["right"].valid()) {
			path = blockState["right"];  block.texture[(int)BlockTexture::RIGHT] = assets.LoadTexture(diffusePath + path);
		}

		itemPath = diffusePath + itemPath;
		load(itemPath.c_str(), items[numItems].texture);
	}
	if (blockState["emitLight"].valid()) block.emitLight = blockState["emitLight"];

	items[numItems].block = blockData.size();
	numItems++;
	//mat.albedo[0] = block.texture[0];
	//mat.albedo[1] = block.texture[1];
	//mat.albedo[2] = block.texture[2];
	//mat.albedo[3] = block.texture[3];
	//mat.albedo[4] = block.texture[4];
	//mat.albedo[5] = block.texture[5];
	block.material = materialData.push_back(mat);

	blockData.push_back(block);

	if (blockState["canSlab"].valid()) {
		block.variations = 1;
	
		Block slab1;
		slab1.name = block.name + "_slab_up";
		slab1.texture[0] = block.texture[0];
		slab1.texture[1] = block.texture[1];
		slab1.texture[2] = block.texture[2];
		slab1.texture[3] = block.texture[3];
		slab1.texture[4] = block.texture[4];
		slab1.texture[5] = block.texture[5];
		slab1.meshID = 1;
		slab1.emitLight = block.emitLight;
		slab1.connectionType = ConnectionType::CUSTOM_MODEL;
		slab1.isCollidable = block.isCollidable;
		slab1.material = block.material;
		blockData.push_back(slab1);
	}
}

struct RarityTable {
	BlockID block = 0;

	float start = 0.f;
	float end = 0.f;
};
}