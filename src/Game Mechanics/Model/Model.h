#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <unordered_map>
#include <Maths/AssimpGLMHelpers.h>

namespace wc {

	struct BoneInfo {
		int32_t id = -1; //For uniquely indentifying the bone and for indexing bone transformation in shaders map from bone name to offset matrix.
		glm::mat4 offset = glm::mat4(1.f); // offset matrix transforms bone from bone space to local space
	};

	class Model {
	public:
		Model() {}

		void Create(const std::string& path) {
			// read file via ASSIMP
			shader.Create("resourcepacks/" + resourceName + "/shaders/modelShader.vert", "resourcepacks/" + resourceName + "/shaders/modelShader.frag");
			shader.depthTest = true;
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_OptimizeMeshes);
			// check for errors
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
			{
				WC_ERROR("ASSIMP: {0}", importer.GetErrorString());
				return;
			}

			// set the vertex attribute pointers
			// vertex Positions
			shader.VertexAttribPointer(0, 3, offsetof(MeshVertex, Position));
			// vertex normals
			shader.VertexAttribPointer(1, 3, offsetof(MeshVertex, Normal));
			// vertex texture coords
			shader.VertexAttribPointer(2, 2, offsetof(MeshVertex, TexCoords));
			// ids
			shader.VertexAttribPointer(3, 4, offsetof(MeshVertex, m_BoneIDs));
			// weights
			shader.VertexAttribPointer(4, 4, offsetof(MeshVertex, m_Weights));

			std::string file = "resourcepacks/default/models/playermodel_tex.png";
			load(file.c_str(), diffuseTexture);

			uint32_t totalIndices = 0, totalVertices = 0;
			getNodeParameters(scene->mRootNode, *scene, totalIndices, totalVertices);

			m_IndexBuffer.Create(sizeof(uint32_t) * totalIndices, GL_DYNAMIC_STORAGE_BIT);
			m_VertexBuffer.Create(sizeof(MeshVertex) * totalVertices, GL_DYNAMIC_STORAGE_BIT);

			shader.SetVertexBuffer(m_VertexBuffer, sizeof(MeshVertex));
			shader.SetIndexBuffer(m_IndexBuffer);

			// process ASSIMP's root node recursively
			cmds.reserve(scene->mNumMeshes);
			processNode(scene->mRootNode, *scene);
		}

		// draws the model, and thus all its meshes
		void Draw()
		{
			diffuseTexture.Bind();
			shader.use();
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, cmds.data(), cmds.size(), sizeof(gl::DrawElementsIndirectCommand));
		}

		std::unordered_map<std::string, BoneInfo> m_OffsetMatMap;
		gl::Shader shader;
	private:
		gl::Buffer m_IndexBuffer;
		gl::Buffer m_VertexBuffer;
		gl::Texture diffuseTexture;
		std::vector<gl::DrawElementsIndirectCommand> cmds;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		void processNode(const aiNode* node, const aiScene& scene) {
			// @TODO: drop the mesh updates a little down to fix animation if it doesnt work
			for (uint32_t j = 0; j < node->mNumMeshes; j++) {
				gl::DrawElementsIndirectCommand cmd;
				cmd.baseVertex = vertexOffset;
				cmd.firstIndex = indexOffset;
				auto& mesh = scene.mMeshes[node->mMeshes[j]];
				std::vector<MeshVertex> vertices;
				for (uint32_t i = 0; i < mesh->mNumVertices; i++)
				{
					MeshVertex vertex;
					vertex.Position = wc::AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
					vertex.Normal = wc::AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

					if (mesh->mTextureCoords[0])
						vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

					vertices.push_back(vertex);
					m_VertexBuffer.SetData(sizeof(MeshVertex), &vertex, vertexOffset * sizeof(MeshVertex));
					vertexOffset++;
				}
				for (uint32_t i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace& face = mesh->mFaces[i];
					m_IndexBuffer.SetData(sizeof(uint32_t) * face.mNumIndices, face.mIndices, sizeof(uint32_t) * indexOffset);
					cmd.count += face.mNumIndices;
					indexOffset += face.mNumIndices;
				}

				cmds.emplace_back(cmd);

				m_OffsetMatMap.reserve(mesh->mNumBones);
				for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
				{
					int32_t boneID = -1;
					std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
					if (m_OffsetMatMap.find(boneName) == m_OffsetMatMap.end())
					{
						boneID = m_OffsetMatMap.size();
						m_OffsetMatMap[boneName] = { boneID , wc::AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix) };
					}
					else
						boneID = m_OffsetMatMap[boneName].id;

					assert(boneID != -1);
					auto& weights = mesh->mBones[boneIndex]->mWeights;
					uint32_t& numWeights = mesh->mBones[boneIndex]->mNumWeights;

					for (uint32_t weightIndex = 0; weightIndex < numWeights; ++weightIndex)
						SetVertexBoneData(vertices[weights[weightIndex].mVertexId], boneID, weights[weightIndex].mWeight);
				}								
			}
			// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
			for (uint32_t i = 0; i < node->mNumChildren; i++)
				processNode(node->mChildren[i], scene);
		}

		void SetVertexBoneData(MeshVertex& vertex, const int& boneID, const float& weight)
		{
			for (uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++)
			{
				if (vertex.m_BoneIDs[i] < 0)
				{
					vertex.m_Weights[i] = weight;
					vertex.m_BoneIDs[i] = boneID;
					return;
				}
			}
		}

		void getNodeParameters(const aiNode* node, const aiScene& scene, uint32_t& totalIndices, uint32_t& totalVertices) {
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
				getNodeParameters(node->mChildren[i], scene, totalIndices, totalVertices);
		}
	};
}