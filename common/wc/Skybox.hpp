#pragma once

#include <gl/Buffer.hpp>
#include <gl/Shaders.hpp>
#include <gl/VertexArray.hpp>
#include <gl/CubeMap.hpp>
#include <sol/sol.hpp>

namespace gl {

	class Skybox{
	public:
        Skybox() {}
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

            float size = playerFarPlane / 2.f;

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
            skyboxVertexBuffer.Create(vertices, sizeof(vertices), 0);
            skyBoxArray.VertexAttribPointer(0, 3, 0);
            skyBoxArray.AddVertexBuffer(skyboxVertexBuffer, 3 * sizeof(float));

            const char* faces[6];
            std::string sfaces[6];
                                               
            if (skyboxState["right"].valid())  { sfaces[0] = skyboxState["right"];  faces[0] = sfaces[0].c_str(); }
            if (skyboxState["left"].valid())   { sfaces[1] = skyboxState["left"];   faces[1] = sfaces[1].c_str(); }
            if (skyboxState["top"].valid())    { sfaces[2] = skyboxState["top"];    faces[2] = sfaces[2].c_str(); }
            if (skyboxState["bottom"].valid()) { sfaces[3] = skyboxState["bottom"]; faces[3] = sfaces[3].c_str(); }
            if (skyboxState["front"].valid())  { sfaces[4] = skyboxState["front"];  faces[4] = sfaces[4].c_str(); }
            if (skyboxState["back"].valid())   { sfaces[5] = skyboxState["back"];   faces[5] = sfaces[5].c_str(); }
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
            skyboxIndicies.Create(indices, sizeof(indices), 0);
            skyBoxArray.AddIndexBuffer(skyboxIndicies);
        }

        void Draw(){
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            shader.use();
            // skybox cube
            skyBoxArray.Bind();
            skyboxTexture.Bind();
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            glDepthFunc(GL_LESS); // set depth function back to default
        }

        void Update(const float& deltaTime, glm::vec3& fogColor) {
            // skybox cube
            angle += deltaTime * rotateSpeed;
            angle = glm::mod(angle, 360.f);

            glm::vec3 skyColor = dayColor;
            glm::vec3 voidColor = dayVoidColor;
            if (angle > 345.f) { 
                voidColor = glm::mix(sunsetColor, nightColor, (360.f - angle) / 15.f);
                glm::vec3 nColor = glm::mix(dayColor, nightColor, (360.f - angle) / 15.f);

                skyColor = nColor;
            }
            else if (angle > startSunrise && angle < sunriseMid) voidColor = glm::mix(sunsetColor, dayVoidColor, (sunriseMid - angle) / 15.f);
            else if (angle > sunriseMid && angle < endSunrise) {

                glm::vec3 nSunsetColor = glm::mix(sunsetColor, nightColor, 1.f - (endSunrise - angle) / 30.f);
                glm::vec3 nColor = glm::mix(dayColor, nightColor, 1.f - (endSunrise - angle) / 30.f);

                skyColor = nColor;
                voidColor = nSunsetColor;
            }
            else if (angle < 30.f) voidColor = glm::mix(sunsetColor, dayVoidColor, (angle / 30.f));

            if (angle > endSunrise && angle < 345.f) {
                voidColor = nightColor;
                skyColor = nightColor;
            }

            fogColor = voidColor;
            shader.setMat4(0, glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)));
            shader.setVec3(1, skyColor);
            shader.setVec3(2, voidColor);
        }

        float rotateSpeed = 1.f * 6.f; // one cycle is one unit (in minutes)
        float angle = 0.f;
        Shader shader;
	private:
        const float startSunrise = 165.f;
        const float sunriseMid = 180.f;
        const float endSunrise = 210.f;

        const glm::vec3 dayColor = glm::vec3(115.f, 211.f, 255.f) / 255.f;
        const glm::vec3 dayVoidColor = glm::vec3(0.f, 0.5f, 0.75f);
        const glm::vec3 sunsetColor = glm::vec3(255, 178, 79) / 255.f;
        const glm::vec3 nightColor = glm::vec3(0.f, 0.f, 4.f / 255.f);
        Cubemap skyboxTexture;
        VertexBuffer skyboxVertexBuffer;
        VertexArray skyBoxArray;
        IndexBuffer skyboxIndicies;
	};
}