#ifndef MESH_HPP
#define MESH_HPP

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gl/Shaders.hpp>

#include <string>
#include <vector>
#include <Renderer/Renderer.hpp>

#define MAX_BONE_INFLUENCE 4
#define MAX_BONE_WEIGHTS MAX_BONE_INFLUENCE * 25

struct MeshVertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;

    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];

    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct MeshTexture {
    uint32_t id = 0;
    std::string type;
    std::string path;
};

namespace wc {
class Mesh {
public:
    // mesh Data
    std::vector<MeshTexture> textures;
    uint32_t VAO;

    // constructor
    Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<MeshTexture>& textures)
    {
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        // create buffers/arrays

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        m_VertexBuffer.Create(vertices.data(), vertices.size() * sizeof(MeshVertex), GL_STATIC_DRAW);
        // load data into vertex buffers
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.

        m_IndexBuffer.Create(&indices[0], indices.size() * sizeof(uint32_t), GL_STATIC_DRAW);

        indexSize = indices.size();

        // set the vertex attribute pointers
        // vertex Positions
        Renderer::VertexAttribPointer(0, 3, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Position));
        // vertex normals
        Renderer::VertexAttribPointer(1, 3, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Normal));
        // vertex texture coords
        Renderer::VertexAttribPointer(2, 2, sizeof(MeshVertex), (void*)offsetof(MeshVertex, TexCoords));
        // ids
        Renderer::VertexAttribIntPointer(3, 4, sizeof(MeshVertex), (void*)offsetof(MeshVertex, m_BoneIDs));
        // weights
        Renderer::VertexAttribPointer(4, 4, sizeof(MeshVertex), (void*)offsetof(MeshVertex, m_Weights));

        glBindVertexArray(0);
    }

    // render the mesh
    void Draw(const gl::Shader& shader) const
    {
        // bind appropriate textures
        uint32_t diffuseNr = 1;
        uint32_t specularNr = 1;
        uint32_t normalNr = 1;
        uint32_t heightNr = 1;
        for (uint32_t i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            std::string number;
            std::string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "texture_specular")
                number = std::to_string(specularNr++); // transfer uint32_t to stream
            else if (name == "texture_normal")
                number = std::to_string(normalNr++); // transfer uint32_t to stream
            else if (name == "texture_height")
                number = std::to_string(heightNr++); // transfer uint32_t to stream

            // now set the sampler to the correct texture unit
            shader.setInt((name + number).c_str(), i);
            // and finally bind the texture
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // draw mesh
        glBindVertexArray(VAO);
        Renderer::DrawIndexed(indexSize);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
    uint32_t indexSize = 0;
    // render data 
    gl::IndexBuffer m_IndexBuffer;
    gl::VertexBuffer m_VertexBuffer;
    gl::VertexArray m_VertexArray;
};
}

#endif