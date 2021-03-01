#ifndef SKY_HPP
#define SKY_HPP

#include <glad/glad.h>
#include <gl/IndexBuffer.hpp>
#include <gl/CubeMap.hpp>
#include <gl/Vertex.hpp>
#include <lua/lua.hpp>
#include <sol/sol.hpp>

namespace gl {

	class Skybox{
	public:
        Skybox() {}
        Skybox(const char* file, const float& playerFarPlane) {Create(file, playerFarPlane);}
        ~Skybox() { skyboxVertexBuffer.Destroy();}
        void Create(const char* file, const float& playerFarPlane) {
            sol::state skyboxState;
            skyboxState.script_file(file);
            if (skyboxState["vertexPath"].valid() && skyboxState["fragmentPath"].valid()) {
                std::string vpath = skyboxState["vertexPath"], fpath = skyboxState["fragmentPath"];
                shader.Create(vpath.c_str(), fpath.c_str());
            }
            else if (skyboxState["shaderPath"].valid()) {
                std::string path = skyboxState["shaderPath"];
                shader.Create(path.c_str());
            }

            float size = playerFarPlane / 2;

            float vertices[] = {
                // positions         
                // Right face
                -size, -size, -size,
                -size,  size, -size,
                 size,  size, -size, 
                 size, -size, -size,

                 // Back face
                -size, -size, -size,
                -size, -size,  size,
                -size,  size,  size,
                -size,  size, -size,
                 
                 // Front face
                 size, -size, -size,
                 size,  size, -size, 
                 size,  size,  size,
                 size, -size,  size,
                              
                 // Left face
                -size, -size,  size,
                 size, -size,  size,
                 size,  size,  size,
                -size,  size,  size,
                       
                 // Top face
                 size,  size, -size,
                -size,  size, -size,
                -size,  size,  size,
                 size,  size,  size,
                            
                 // Bottom face
                -size, -size,  size, 
                -size, -size, -size,
                 size, -size, -size,
                 size, -size,  size
            };
            skyBoxArray.Create();
            skyBoxArray.Bind();
            skyboxVertexBuffer.Create(vertices, sizeof(vertices), GL_STATIC_DRAW);
            skyBoxArray.VertexAttribPointer(0, 3, 3 * sizeof(float), (void*)0);

            const char** faces;
            std::array<std::string, 6> sfaces;

            sfaces[0] = skyboxState["right"];
            sfaces[1] = skyboxState["left"];
            sfaces[2] = skyboxState["top"];
            sfaces[3] = skyboxState["bottom"];
            sfaces[4] = skyboxState["front"];
            sfaces[5] = skyboxState["back"];

            faces[0] = sfaces[0].c_str();
            faces[1] = sfaces[1].c_str();
            faces[2] = sfaces[2].c_str();
            faces[3] = sfaces[3].c_str();
            faces[4] = sfaces[4].c_str();
            faces[5] = sfaces[5].c_str();
            skyboxTexture.Create(faces);

            uint32_t indices[36];
            uint32_t offset = 0;
            for (int8_t i = 0; i < 36; i += 6) {
                indices[i + 0] = 0 + offset;
                indices[i + 1] = 1 + offset;
                indices[i + 2] = 2 + offset;

                indices[i + 3] = 2 + offset;
                indices[i + 4] = 3 + offset;
                indices[i + 5] = 0 + offset;

                offset += 4;
            }
            skyboxIndicies.Create(indices, sizeof(indices));
        }

        void Draw(const glm::mat4& view, const glm::mat4& projection, const float& deltaTime){
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            // skybox cube
            skyBoxArray.Bind();
            skyboxTexture.Bind();
            skyboxIndicies.Bind();
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            glDepthFunc(GL_LESS); // set depth function back to default
            skyboxIndicies.Unbind();
            skyboxVertexBuffer.Unbind();
        }

        Shader shader;
	private:
        float rotateSpeed = 1.0f;
        float angle = 0.0f;
        Cubemap skyboxTexture;
        VertexBuffer skyboxVertexBuffer;
        VertexArray skyBoxArray;
        IndexBuffer skyboxIndicies;
	};
}
#endif