#pragma once

#include <glad/glad.h>
#include <gl/IndexBuffer.h>
#include <gl/VertexArray.h>
#include <Utilitiess/Lua.hpp>

namespace gl {

	class Skybox{
	public:
        ~Skybox() {
            skyboxArray.Destroy();
        }
        void Create(const char* file, uint32_t activeTexture = 0) {
            this->activeTexture = activeTexture;
            wc::Lua skyboxState(file);
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

            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

            int width, height, nrChannels;

            std::vector<const char*> faces;
            faces.push_back(skyboxState.GetString("right"));
            faces.push_back(skyboxState.GetString("left"));
            faces.push_back(skyboxState.GetString("top"));
            faces.push_back(skyboxState.GetString("bottom"));
            faces.push_back(skyboxState.GetString("front"));
            faces.push_back(skyboxState.GetString("back"));

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
            

            for (uint32_t i = 0; i < faces.size(); i++){
                auto* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
                if (data){
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                    );
                    stbi_image_free(data);
                }
                else{
                    std::cout << "Cubemap tex failed to load at path: " << faces[i] << "\n";
                    stbi_image_free(data);
                }
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            shader.use();
            shader.setInt("skybox", this->activeTexture);
        }

        void Draw(float screenWidth, float screenHeight, glm::mat4 view, glm::mat4 projection){
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            // skybox cube
            skyboxArray.Bind();
            glActiveTexture(GL_TEXTURE0 + activeTexture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
            skyboxIndicies.Bind();
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            glDepthFunc(GL_LESS); // set depth function back to default
            skyboxIndicies.Unbind();
            skyboxArray.Unbind();
        }

        Shader shader;
	private:
        uint32_t activeTexture = 0;
        uint32_t textureID = 0;
        VertexBuffer skyboxArray;
        IndexBuffer skyboxIndicies;
	};
}