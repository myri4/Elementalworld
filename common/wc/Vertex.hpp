#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

namespace gl {

struct Vertex {
    glm::vec3 Position = { 0,0,0 };
    glm::vec3 TexCoords = { 0,0,0 };
    uint32_t type = 0;
    uint32_t color = 0;
    glm::vec3 Normal = { 0,0,0 };
    Vertex() = default;
    Vertex(const Vertex& vertex) : Position(vertex.Position), TexCoords(vertex.TexCoords) {}
    Vertex(const glm::vec3& pos, const glm::vec3& texCoord, const uint32_t& Type, const uint32_t& Color, const glm::vec3& normal) : Position(pos), TexCoords(texCoord), type(Type), color(Color), Normal(normal) {}
};
}

#endif