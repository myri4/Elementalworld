#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <gl/Shaders.hpp>

namespace gl {
struct Material {
	glm::vec3 ambient = { 0,0,0 };
	float shininess = 0.f;
	Material() {}
	Material(const Material& material) : ambient(material.ambient), shininess(material.shininess) {}
	Material(const glm::vec3& ambient, const float& shininess) : ambient(ambient), shininess(shininess) {}
	~Material() {}

	void Apply(const gl::Shader &shader, const std::string& value) {
		shader.use();
		shader.setVec3(std::string(value + ".ambient").c_str(), ambient);
		shader.setFloat(std::string(value + ".shininess").c_str(), shininess);
	}
};
}
#endif