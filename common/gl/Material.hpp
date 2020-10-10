#pragma once
#include <gl/Shaders.h>

namespace gl {
class Material {
public:
	glm::vec3 ambient = { 0,0,0 };
	glm::vec3 diffuse = { 0,0,0 };
	glm::vec3 specular = { 0,0,0 };
	float shininess = 0.f;
	Material() {}
	Material(const Material& material) : ambient(material.ambient), diffuse(material.diffuse), specular(material.specular), shininess(material.shininess) {}
	Material(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float& shininess) : ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}
	~Material() {}

	void Apply(gl::Shader& shader, const std::string& value) {
		shader.use();
		shader.setVec3(std::string(value + ".ambient").c_str(), ambient);
		shader.setVec3(std::string(value + ".diffuse").c_str(), diffuse);
		shader.setVec3(std::string(value + ".specular").c_str(), specular);
		shader.setFloat(std::string(value + ".shininess").c_str(), shininess);
	}
};
}