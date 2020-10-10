#pragma once
#include <gl/Shaders.h>

namespace gl {
	class Light {
	public:
		glm::vec4 vector;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		float constant;
		float linear;
		float quadratic;

		glm::vec3 color;
		Light() {}
		Light(const Light& light) : ambient(light.ambient), diffuse(light.diffuse), specular(light.specular) {}
		Light(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float& constant, const float& linear) : ambient(ambient), diffuse(diffuse), specular(specular){}
		~Light() {}

		void Apply(gl::Shader& shader, const std::string& value) {
			shader.use();
			shader.setVec3(std::string(value + ".ambient").c_str(), ambient);
			shader.setVec3(std::string(value + ".diffuse").c_str(), diffuse);
			shader.setVec3(std::string(value + ".specular").c_str(), specular);
		}
	};
}