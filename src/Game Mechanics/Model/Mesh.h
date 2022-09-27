#pragma once

#include <pch.h>

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS MAX_BONE_INFLUENCE * 25

struct MeshVertex {
    // position
    glm::vec3 Position = glm::vec3(0.f);
    // normal
    glm::vec3 Normal = glm::vec3(0.f);
    // texCoords
    glm::vec2 TexCoords = glm::vec2(0.f);

    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE] = { -1 };

    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE] = { 0.f };

    //uint32_t materialID = 0;

    MeshVertex() {}

	static wc::VertexInputDescription get_vertex_description() {
		wc::VertexInputDescription description;

		//we will have just 1 vertex buffer binding, with a per-vertex rate
		VkVertexInputBindingDescription mainBinding = {};
		mainBinding.binding = 0;
		mainBinding.stride = sizeof(MeshVertex);
		mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		description.bindings.push_back(mainBinding);

		//Position will be stored at Location 0
		VkVertexInputAttributeDescription positionAttribute = {};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		positionAttribute.offset = offsetof(MeshVertex, Position);

		//Normal will be stored at Location 1
		VkVertexInputAttributeDescription normalAttribute = {};
		normalAttribute.binding = 0;
		normalAttribute.location = 1;
		normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttribute.offset = offsetof(MeshVertex, TexCoords);

		//TexCoords will be stored at Location 2
		VkVertexInputAttributeDescription texCoordAttribute = {};
		texCoordAttribute.binding = 0;
		texCoordAttribute.location = 2;
		texCoordAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		texCoordAttribute.offset = offsetof(MeshVertex, TexCoords);

		//boneIDs will be stored at Location 3
		VkVertexInputAttributeDescription boneIDAttribute = {};
		boneIDAttribute.binding = 0;
		boneIDAttribute.location = 3;
		boneIDAttribute.format = VK_FORMAT_R32G32B32A32_SINT;
		boneIDAttribute.offset = offsetof(MeshVertex, m_BoneIDs);

		//boneIDs will be stored at Location 4
		VkVertexInputAttributeDescription weightsAttribute = {};
		weightsAttribute.binding = 0;
		weightsAttribute.location = 4;
		weightsAttribute.format = VK_FORMAT_R32G32B32A32_SINT;
		weightsAttribute.offset = offsetof(MeshVertex, m_Weights);

		description.attributes.push_back(positionAttribute);
		description.attributes.push_back(texCoordAttribute);
		description.attributes.push_back(normalAttribute);
		description.attributes.push_back(boneIDAttribute);
		description.attributes.push_back(weightsAttribute);
		return description;
	}
};
