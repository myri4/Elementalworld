#ifndef LIGHT_HPP
#define LIGHT_HPP
#include <gl/Shaders.hpp>

namespace gl {
    struct Light {
        glm::vec4 vector;

        float constant;
        float linear;
        float quadratic;

        float strenght;

        void Apply(const gl::Shader &shader, const std::string& value) {
            shader.use();
            shader.setVec4(std::string(value + ".vector").c_str(), vector);
            shader.setFloat(std::string(value + ".constant").c_str(), constant);
            shader.setFloat(std::string(value + ".linear").c_str(), linear);
            shader.setFloat(std::string(value + ".quadratic").c_str(), quadratic);
            shader.setFloat(std::string(value + ".strenght").c_str(), strenght);
        }
    };
}
#endif