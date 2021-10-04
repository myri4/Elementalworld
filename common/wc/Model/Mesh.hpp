#pragma once

#include <gl/Buffer.hpp>
#include <gl/Texture.hpp>
#include <gl/VertexArray.hpp>

#include <vector>
#include <Renderer/Renderer.hpp>

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

    MeshVertex() {
        memset(m_BoneIDs,-1, sizeof(m_BoneIDs));
        memset(m_Weights, 0, sizeof(m_Weights));
    }
};


namespace wc {
class Mesh {
public:
    // mesh Data
    gl::Texture diffuseTexture;

    // constructor
    Mesh() {}
    Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const gl::Texture& Textures) { Create(vertices, indices, Textures); }

    void Create(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const gl::Texture& Textures) {
        diffuseTexture = Textures;

        m_VertexArray.Create();

        m_VertexBuffer.Create(vertices.data(), vertices.size() * sizeof(MeshVertex), 0);

        m_IndexBuffer.Create(indices.data(), indices.size() * sizeof(uint32_t), 0);

        m_VertexArray.AddIndexBuffer(m_IndexBuffer);
        m_VertexArray.AddVertexBuffer(m_VertexBuffer, sizeof(MeshVertex));
        indexSize = indices.size();

        // set the vertex attribute pointers
        // vertex Positions
        m_VertexArray.VertexAttribPointer(0, 3, offsetof(MeshVertex, Position));
        // vertex normals
        m_VertexArray.VertexAttribPointer(1, 3, offsetof(MeshVertex, Normal));
        // vertex texture coords
        m_VertexArray.VertexAttribPointer(2, 2, offsetof(MeshVertex, TexCoords));
        // ids
        m_VertexArray.VertexAttribIntPointer(3, 4, offsetof(MeshVertex, m_BoneIDs));
        // weights
        m_VertexArray.VertexAttribPointer(4, 4, offsetof(MeshVertex, m_Weights));
    }

    // render the mesh
    void Draw() const {
        // bind appropriate textures
        diffuseTexture.Bind();
        // draw mesh
        m_VertexArray.Bind();
        Renderer::DrawIndexed(indexSize);
    }

private:
    uint32_t indexSize = 0;
    // render data 
    gl::VertexArray m_VertexArray;
    gl::IndexBuffer m_IndexBuffer;
    gl::VertexBuffer m_VertexBuffer;
};
}