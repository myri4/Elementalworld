#pragma once

#include <glad/glad.h>
#include <gl/IndexBuffer.h>
#include <gl/Cubemap.h>
#include <gl/VertexArray.h>
#include <Utilitiess/Lua.hpp>

namespace gl {

	class Skybox{
	public:
        Skybox() {

        }
        Skybox(const char* file) {
            Create(file);
        }
        ~Skybox() {
            skyboxArray.Destroy();
        }
        void Create(const char* file) {
            wc::Lua skyboxState(file);
            if(std::string(skyboxState.GetString("vertexPath")).empty() || std::string(skyboxState.GetString("vertexPath")).empty())
                shader.Create(skyboxState.GetString("shaderPath"));
            else
                shader.Create(skyboxState.GetString("vertexPath"), skyboxState.GetString("fragmentPath"));
            float size = (float)skyboxState.GetNumber("skyboxSize");

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
            skyboxArray.Create(&vertices, sizeof(vertices), GL_STATIC_DRAW);
            skyboxArray.Bind();
            gl::VertexAttribPointer(0, 3, 3 * sizeof(float), (void*)0);
            skyboxArray.Unbind();

            std::vector<const char*> faces;
            faces.push_back(skyboxState.GetString("right"));
            faces.push_back(skyboxState.GetString("left"));
            faces.push_back(skyboxState.GetString("top"));
            faces.push_back(skyboxState.GetString("bottom"));
            faces.push_back(skyboxState.GetString("front"));
            faces.push_back(skyboxState.GetString("back"));
            skyboxTexture.Create(faces);

            uint32_t indices[36];
            uint32_t offset = 0;
            for (int i = 0; i < 36; i += 6) {
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

        void Draw(const glm::mat4& view, const glm::mat4& projection){
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            // skybox cube
            skyboxArray.Bind();
            skyboxTexture.Bind();
            skyboxIndicies.Bind();
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            glDepthFunc(GL_LESS); // set depth function back to default
            skyboxIndicies.Unbind();
            skyboxArray.Unbind();
        }

        Shader shader;
	private:
        gl::Cubemap skyboxTexture;
        VertexBuffer skyboxArray;
        IndexBuffer skyboxIndicies;
	};
}