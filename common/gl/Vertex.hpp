#pragma once
#include <glm/glm.hpp>
namespace gl {
class Vertex {
public:
	glm::vec3 Position = { 0,0,0 };
	glm::vec2 TexCoords = { 0,0 };
	glm::vec3 Normal = { 0, 0, 0 };
	float TexID = 0;
	Vertex() {}
	Vertex(const Vertex& vertex) : Position(vertex.Position), TexCoords(vertex.TexCoords), TexID(vertex.TexID) {}
	Vertex(const glm::vec3& pos, const glm::vec2& texCoord, const glm::vec3& Normal, const float& texID) : Position(pos), TexCoords(texCoord), Normal(Normal), TexID(texID) {}
	~Vertex() {}
};
}