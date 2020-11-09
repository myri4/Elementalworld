#pragma once
#include <gl/Shaders.hpp>

namespace gl {
    class Light {
    public:
        glm::vec4 vector;

        float constant;
        float linear;
        float quadratic;

        float strenght;

        glm::vec3 color;

        void Apply(const gl::Shader &shader, const std::string& value) {
            shader.use();
            shader.setVec4(std::string(value + ".vector").c_str(), vector);
            shader.setFloat(std::string(value + ".constant").c_str(), constant);
            shader.setFloat(std::string(value + ".linear").c_str(), linear);
            shader.setFloat(std::string(value + ".quadratic").c_str(), quadratic);
            shader.setFloat(std::string(value + ".strenght").c_str(), strenght);
            shader.setVec3(std::string(value + ".color").c_str(), color);
        }
    };
}