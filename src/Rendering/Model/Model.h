#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

#include <unordered_map>
#include <wc/Maths/AssimpGLMHelpers.h>
#include <wc/Shader.h>
#include "../AssetManager.h"

namespace wc {

	struct BoneInfo {
		int32_t id = -1; //For uniquely indentifying the bone and for indexing bone transformation in shaders map from bone name to offset matrix.
		glm::mat4 offset = glm::mat4(1.f); // offset matrix transforms bone from bone space to local space
	};

	class Model {
	public:
		Model() {}

		void Create(const std::string& path, const wc::RenderPass& renderPass, const glm::vec2& windowSize) {
			// read file via ASSIMP
			wc::ShaderCreateInfo createInfo;
			createInfo.vertexShader =   GetAssetPath() + "/shaders/modelShader.vert";
			createInfo.fragmentShader = GetAssetPath() + "/shaders/modelShader.frag";
			createInfo.windowSize = windowSize;
			createInfo.renderPass = renderPass;
			createInfo.vertexDescription = MeshVertex::get_vertex_description();
			createInfo.blending = false;
			createInfo.depthTest = true;
			shader.Create(createInfo);
			

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_OptimizeMeshes);
			// check for errors
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
			{
				WC_ERROR("ASSIMP: {0}", importer.GetErrorString());
				return;
			}

			std::string file = GetAssetPath() + "/models/playermodel_tex.png";

			VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

			sampler.magFilter = VK_FILTER_NEAREST;
			sampler.minFilter = VK_FILTER_NEAREST;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			//wc::loadTexture(file, diffuseTexture);
			//diffuseTexture.SetSamplerInfo(sampler);

			{
				//wc::DescriptorWriter writer;
				//writer.dstSet = shader.descriptorSet;
				//writer.write_image(0, diffuseTexture.GetDescriptorData(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				//
				//wc::UpdateDescriptorSets(writer.writes.size(), writer.writes.data());
			}

			uint32_t totalIndices = 0, totalVertices = 0;
			getNodeParameters(scene->mRootNode, *scene, totalIndices, totalVertices);

			m_IndexBuffer.Create(sizeof(uint32_t) * totalIndices, wc::INDEX_BUFFER);
			m_VertexBuffer.Create(sizeof(MeshVertex) * totalVertices, wc::VERTEX_BUFFER);

			vertices.Create(sizeof(MeshVertex) * totalVertices);
			vertices.Map();

			indices.Create(sizeof(uint32_t) * totalIndices);
			indices.Map();

			// process ASSIMP's root node recursively
			cmds.reserve(scene->mNumMeshes);
			processNode(scene->mRootNode, *scene);

			vertices.Unmap();
			indices.Unmap();

			m_IndexBuffer.SetData(indices.GetBuffer(), sizeof(uint32_t) * totalIndices);
			m_IndexBuffer.SetData(vertices.GetBuffer(), sizeof(MeshVertex) * totalVertices);

			m_IndirectBuffer.Create(sizeof(VkDrawIndexedIndirectCommand) * cmds.size(), wc::INDIRECT_BUFFER);
		}

		// draws the model, and thus all its meshes
		void Draw(const wc::CommandBuffer& cmd)
		{
			shader.Bind(cmd);
			cmd.DrawIndexedIndirect(m_IndirectBuffer, (uint32_t)cmds.size());
		}

		std::unordered_map<std::string, BoneInfo> m_OffsetMatMap;
		wc::Shader shader;
	private:
		wc::Buffer m_IndexBuffer;
		wc::Buffer m_VertexBuffer;

		//wc::Texture diffuseTexture;

		wc::Buffer m_IndirectBuffer;
		std::vector<VkDrawIndexedIndirectCommand> cmds;

		wc::CPUBuffer<MeshVertex> vertices;
		wc::CPUBuffer<uint32_t> indices;
		uint32_t vertexOffset = 0;
		uint32_t indexOffset = 0;

		void processNode(const aiNode* node, const aiScene& scene) {
			// @TODO: drop the mesh updates a little down to fix animation if it doesnt work
			for (uint32_t j = 0; j < node->mNumMeshes; j++) {
				VkDrawIndexedIndirectCommand cmd;
				cmd.vertexOffset = vertexOffset;
				cmd.firstIndex = indexOffset;
				auto& mesh = scene.mMeshes[node->mMeshes[j]];
				for (uint32_t i = 0; i < mesh->mNumVertices; i++)
				{
					MeshVertex& vertex = vertices[vertexOffset];
					vertex.Position = wc::AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
					vertex.Normal = wc::AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
			
					if (mesh->mTextureCoords[0])
						vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			
					vertexOffset++;
				}
				for (uint32_t i = 0; i < mesh->mNumFaces; i++)
				{
					aiFace& face = mesh->mFaces[i];
					memcpy((void*)(indices + indexOffset), face.mIndices, sizeof(uint32_t) * face.mNumIndices);
					cmd.indexCount += face.mNumIndices;
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
						boneID = (int32_t)m_OffsetMatMap.size();
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