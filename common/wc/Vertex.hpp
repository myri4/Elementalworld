#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <glm/glm.hpp>

namespace gl {

struct Vertex {
    glm::vec3 Position = { 0,0,0 };
    glm::vec3 TexCoords = { 0,0,0 };
    int type = 0;
    Vertex() {}
    Vertex(const Vertex& vertex) : Position(vertex.Position), TexCoords(vertex.TexCoords) {}
    Vertex(const glm::vec3& pos, const glm::vec3& texCoord, const int& Type) : Position(pos), TexCoords(texCoord), type(Type) {}
    ~Vertex() {}
};
}

#endif