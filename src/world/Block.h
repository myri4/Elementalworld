#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <wc/Maths/AssimpGLMHelpers.h>
#include <wc/Utils/Log.h>
#include "../Game Mechanics/Item.h"
#include <magic_enum.hpp>
#include <wc/Utils/List.h>
#include <wc/Utils/YAML.h>
#include "../Rendering/AssetManager.h"
#include "../Rendering/Renderer3D.h"
#include "../Globals.h"

namespace wc{

	struct BlockMesh {
		VkDrawIndexedIndirectCommand cmd = {};
		glm::vec4 start;
		glm::vec4 end;

		BlockMesh() = default;

		void Load(const std::string& path, const uint32_t& materialID, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_FlipUVs);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				WC_ERROR(importer.GetErrorString());
				return;
			}
			uint32_t offset = 0;
			processNode(scene->mRootNode, *scene, offset, materialID, vertices, indices);

			uint32_t totalVertices = 0;
			GetMeshSize(scene->mRootNode, *scene, cmd.indexCount, totalVertices);
			cmd.instanceCount = 0;

			uint32_t vertexOffset = Renderer3D::vertexSize;

			Mesh mesh = Renderer3D::CreateMesh(totalVertices, cmd.indexCount);
			cmd.vertexOffset = mesh.vertexOffset;
			cmd.firstIndex = mesh.indexOffset;

			vertexOffset = cmd.vertexOffset - vertexOffset;

			start = glm::vec4(vertices[vertexOffset].Position, 1.f);
			end = glm::vec4(vertices[vertexOffset].Position, 1.f);
			
			for (uint32_t i = vertexOffset + 1; i < vertices.size(); i++) {
				start = glm::max(start, glm::vec4(vertices[i].Position, 1.f));
				end = glm::min(end, glm::vec4(vertices[i].Position, 1.f));
			}
			importer.FreeScene();
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

		void GetMeshSize(const aiNode* node, const aiScene& scene, uint32_t& totalIndices, uint32_t& totalVertices) {
			// process each mesh located at the current node		
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			for (uint32_t j = 0; j < node->mNumMeshes; j++) {
				auto& mesh = scene.mMeshes[node->mMeshes[j]];
				totalVertices += mesh->mNumVertices;
				for (uint32_t i = 0; i < mesh->mNumFaces; i++)
					totalIndices += mesh->mFaces[i].mNumIndices;

			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (uint32_t i = 0; i < node->mNumChildren; i++)
				GetMeshSize(node->mChildren[i], scene, totalIndices, totalVertices);
		}
	};

	struct Material {
		uint32_t albedo[6] = { 0 };
		uint32_t materialData[6] = { 0 };
		uint32_t flags = 0;
		uint32_t color = 0xFFFFFFFF;
	};

	struct Block {
		bool emitLight : 1;
		ConnectionType connectionType = ConnectionType::AIR;

		bool isCollidable : 1;
		MeshID meshID = 0;
		uint8_t variations = 0;
		MaterialID materialID = 0;
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
}