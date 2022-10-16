#pragma once
#include <glm/glm.hpp>
#include <vk/Buffer.h>
#include <vk/Pipeline.h>

//@Todo try with size_t 
const uint16_t chunkSize = 16;
const uint32_t chunkVolume = chunkSize * chunkSize * chunkSize;

const uint32_t MaxFaceCount = chunkSize * chunkSize * 4;
const uint32_t MaxVertexCount = MaxFaceCount * 4;
const uint32_t MaxIndexCount = MaxFaceCount * 6;

static const uint32_t RenderDistance = 16;

using ChunkID = uint16_t; // This represents the chunk id in the chunk array
using BlockID = uint8_t;
using MaterialID = uint8_t;
using MeshID = uint32_t;

std::string resourceName = "default";

//@TODO: REMOVE
enum MenuMode { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU, WORLD_SELECTION, WORLD_CREATION};
uint32_t mode = MenuMode::MAINMENU;

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
	glm::vec3 TexCoords = { 0,0,0 };
	uint32_t materialID = 0;
	glm::vec3 Normal = { 0,0,0 };
	Vertex() {}
	Vertex(const glm::vec3& pos, const glm::vec3& texCoord, const glm::vec3& normal, const uint32_t& mat) : Position(pos), Normal(normal), materialID(mat), TexCoords(texCoord) { }

	static wc::VertexInputDescription get_vertex_description() {
		wc::VertexInputDescription description;

		//we will have just 1 vertex buffer binding, with a per-vertex rate
		VkVertexInputBindingDescription mainBinding = {};
		mainBinding.binding = 0;
		mainBinding.stride = sizeof(Vertex);
		mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		description.bindings.push_back(mainBinding);

		//Position will be stored at Location 0
		VkVertexInputAttributeDescription positionAttribute = {};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		positionAttribute.offset = offsetof(Vertex, Position);
		description.attributes.push_back(positionAttribute);

		//TexCoords will be stored at Location 1
		VkVertexInputAttributeDescription texCoordAttribute = {};
		texCoordAttribute.binding = 0;
		texCoordAttribute.location = 1;
		texCoordAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		texCoordAttribute.offset = offsetof(Vertex, TexCoords);
		description.attributes.push_back(texCoordAttribute);

		//Normal will be stored at Location 2
		VkVertexInputAttributeDescription normalAttribute = {};
		normalAttribute.binding = 0;
		normalAttribute.location = 2;
		normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttribute.offset = offsetof(Vertex, Normal);
		description.attributes.push_back(normalAttribute);

		//materialID will be stored at Location 3
		VkVertexInputAttributeDescription materialAttribute = {};
		materialAttribute.binding = 0;
		materialAttribute.location = 3;
		materialAttribute.format = VK_FORMAT_R32_UINT;
		materialAttribute.offset = offsetof(Vertex, materialID);
		description.attributes.push_back(materialAttribute);


		return description;
	}
};