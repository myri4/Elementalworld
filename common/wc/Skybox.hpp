#pragma once

#include <gl/Buffer.hpp>
#include <gl/Shaders.hpp>
#include <gl/VertexArray.hpp>
#include <Renderer/Renderer.hpp>

namespace gl {

	class Skybox{
	public:
        Skybox() {}
        void Create(const char* file) {

            float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                // positions 
                -1.0f, -1.0f,
                -1.0f,  1.0f,
                 1.0f, -1.0f,

                 1.0f, -1.0f,
                -1.0f,  1.0f,
                 1.0f,  1.0f,
            };

            skyboxVertexBuffer.Create(quadVertices, sizeof(quadVertices), 0);
            skyBoxArray.Create();
            skyBoxArray.VertexAttribPointer(0, 2, 0);
            skyBoxArray.AddVertexBuffer(skyboxVertexBuffer, sizeof(float) * 2);

            shader.Create(file);
        }

        void Draw(const float& deltaTime){
            shader.use();
            skyBoxArray.Bind();

            wc::Renderer::DrawArrays(6);

            // skybox cube
            angle += deltaTime * rotateSpeed;
            angle = glm::mod(angle, 360.f);
        }

        float rotateSpeed = 1.f * 6.f; // one cycle is one unit (in minutes)
        float angle = 0.f;
	private:
        Shader shader;
        VertexBuffer skyboxVertexBuffer;
        VertexArray skyBoxArray;
	};
}