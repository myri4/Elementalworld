#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.hpp"

#include <unordered_map>
#include <Maths/AssimpGLMHelpers.hpp>

namespace wc {

struct BoneInfo {
	int id; //For uniquely indentifying the bone and for indexing bone transformation in shaders map from bone name to offset matrix.
	glm::mat4 offset = glm::mat4(1.f); // offset matrix transforms bone from bone space to local space
};

class Model{
public:
	// model data 
	Mesh modelMesh;
	std::string directory;

	// constructor, expects a filepath to a 3D model.
	Model() {}

	void Create(const std::string& path) {
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			WC_ERROR("ASSIMP: {0}", importer.GetErrorString());
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, *scene);
	}

	// draws the model, and thus all its meshes
	void Draw()
	{
		modelMesh.Draw();
	}

	std::unordered_map<std::string, BoneInfo> m_OffsetMatMap;

private:

	void processNode(const aiNode* node, const aiScene& scene) {
		// process each mesh located at the current node		
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		for (uint32_t j = 0; j < node->mNumMeshes; j++) {
			auto& mesh = scene.mMeshes[node->mMeshes[j]];
			std::vector<MeshVertex> vertices;
			std::vector<uint32_t> indices;
			vertices.reserve(mesh->mNumVertices);
			for (uint32_t i = 0; i < mesh->mNumVertices; i++)
			{
				MeshVertex vertex;
				vertex.Position = wc::AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
				vertex.Normal = wc::AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

				if (mesh->mTextureCoords[0])
				{
					glm::vec2 vec;
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.TexCoords = vec;
				}

				vertices.emplace_back(vertex);
			}
			indices.reserve(mesh->mNumFaces);
			for (uint32_t i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace& face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++) {
					indices.emplace_back(face.mIndices[j]);
					//WC_INFO(face.mIndices[j]);
				}
			}
			aiString str;
			scene.mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			gl::Texture tex;
			std::string file = directory + '/' + str.C_Str();
			load(file.c_str(), tex);

			//ExtractBoneWeightForVertices
			m_OffsetMatMap.reserve(mesh->mNumBones);
			for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
			{
				int boneID = -1;
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
				{
					uint32_t& vertexId = weights[weightIndex].mVertexId;
					float& weight = weights[weightIndex].mWeight;
					SetVertexBoneData(vertices[vertexId], boneID, weight);
				}
			}
			modelMesh.Create(vertices, indices, tex);
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