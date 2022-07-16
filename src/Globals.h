#pragma once
#include <glm/glm.hpp>

//@Todo try with size_t 
const uint16_t chunkSize = 16;
const uint32_t chunkVolume = chunkSize * chunkSize * chunkSize;

const uint32_t MaxFaceCount = chunkSize * chunkSize * 5;
const uint32_t MaxVertexCount = MaxFaceCount * 4;
const uint32_t MaxIndexCount = MaxFaceCount * 6;

static const uint32_t RenderDistance = 16;

using ChunkID = uint16_t; // This represents the chunk id in the chunk array
using BlockID = uint8_t;
using MaterialID = uint8_t;
using MeshID = uint32_t;

std::string resourceName = "default";

enum MenuMode { GAME, INVENTORY, MAINMENU, SETTINGS, MULTIPLAYER, ESCMENU };
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
};

Vertex* globalVertices = nullptr;
uint32_t* globalIndices = nullptr; // @TODO: completely remove this from here