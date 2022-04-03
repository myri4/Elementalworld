#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.hpp"

#include <unordered_map>
#include <Maths/AssimpGLMHelpers.hpp>

namespace wc {

struct BoneInfo {
	int32_t id = -1; //For uniquely indentifying the bone and for indexing bone transformation in shaders map from bone name to offset matrix.
	glm::mat4 offset = glm::mat4(1.f); // offset matrix transforms bone from bone space to local space
};

struct Mesh {
	gl::VertexArray m_VertexArray;
	gl::Buffer m_IndexBuffer;
	gl::Buffer m_VertexBuffer;

	uint32_t indexSize = 0;
	gl::Texture diffuseTexture;
};

class Model{
public:
	Model() {}

	void Create(const std::string& path) {
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_OptimizeMeshes);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			WC_ERROR("ASSIMP: {0}", importer.GetErrorString());
			return;
		}

		// process ASSIMP's root node recursively
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, *scene);
		meshes.reserve(scene->mNumMeshes);
	}

	// draws the model, and thus all its meshes
	void Draw()
	{
		// bind appropriate textures
		// draw mesh
		for (uint32_t i = 0; i < meshes.size(); i++) {
			meshes[i].diffuseTexture.Bind();
			meshes[i].m_VertexArray.Bind();
			Renderer::DrawIndexed(meshes[i].indexSize);
		}
	}

	std::unordered_map<std::string, BoneInfo> m_OffsetMatMap;
private:
	std::string directory;
	std::vector<Mesh> meshes;

	void processNode(const aiNode* node, const aiScene& scene) {
		// process each mesh located at the current node		
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		for (uint32_t j = 0; j < node->mNumMeshes; j++) {
			Mesh m;
			auto& mesh = scene.mMeshes[node->mMeshes[j]];
			std::vector<MeshVertex> vertices;
			std::vector<uint32_t> indices;
			vertices.reserve(mesh->mNumVertices);
			indices.reserve(mesh->mNumFaces);
			for (uint32_t i = 0; i < mesh->mNumVertices; i++)
			{
				MeshVertex vertex;
				vertex.Position = wc::AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
				vertex.Normal = wc::AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

				if (mesh->mTextureCoords[0])				
					vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);				

				vertices.emplace_back(vertex);
			}
			for (uint32_t i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace& face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) 
					indices.emplace_back(face.mIndices[j]);				
			}

			//ExtractBoneWeightForVertices
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

			m.m_VertexArray.Create();

			m.indexSize = indices.size();
			m.m_VertexBuffer.Create(vertices.data(), vertices.size() * sizeof(MeshVertex), 0);

			m.m_IndexBuffer.Create(indices.data(), m.indexSize * sizeof(uint32_t), 0);

			m.m_VertexArray.AddIndexBuffer(m.m_IndexBuffer);
			m.m_VertexArray.AddVertexBuffer(m.m_VertexBuffer, sizeof(MeshVertex));

			aiString str;
			scene.mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			std::string file = directory + '/' + str.C_Str();
			load(file.c_str(), m.diffuseTexture);

			// set the vertex attribute pointers
			// vertex Positions
			m.m_VertexArray.VertexAttribPointer(0, 3, offsetof(MeshVertex, Position));
			// vertex normals
			m.m_VertexArray.VertexAttribPointer(1, 3, offsetof(MeshVertex, Normal));
			// vertex texture coords
			m.m_VertexArray.VertexAttribPointer(2, 2, offsetof(MeshVertex, TexCoords));
			// ids
			m.m_VertexArray.VertexAttribIntPointer(3, 4, offsetof(MeshVertex, m_BoneIDs));
			// weights
			m.m_VertexArray.VertexAttribPointer(4, 4, offsetof(MeshVertex, m_Weights));
			meshes.emplace_back(m);
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
};
}