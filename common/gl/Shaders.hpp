#pragma once

#include <glad/glad.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>

#include <Utils/Log.hpp>

namespace wcUtil {
	void checkCompileErrors(const uint32_t& shader) {
		int success;
		char infoLog[1024];
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
			WC_ERROR("PROGRAM_LINKING_ERROR: \n{1}", infoLog);
		}
	}

	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			WC_ERROR("Failed to open file at location {0}!", filename);
			return std::vector<char>();
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	uint32_t CompileShader(const std::vector<char>& code, const uint32_t& type) {
		uint32_t shader = glCreateShader(type);
		glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, reinterpret_cast<const uint32_t*>(code.data()), code.size());
		glSpecializeShader(shader, "main", 0, nullptr, nullptr);
		return shader;
	}
}

namespace gl {
	class Shader {
		uint32_t m_RendererID = 0;
	public:
		Shader() {}
		~Shader() { Destroy(); }

		void Create(const char* vertexPath, const char* fragmentPath) {
			if (!m_RendererID) {
				uint32_t vertex = wcUtil::CompileShader(wcUtil::readFile(vertexPath), GL_VERTEX_SHADER);
				uint32_t fragment = wcUtil::CompileShader(wcUtil::readFile(fragmentPath), GL_FRAGMENT_SHADER);
				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, vertex);
				glAttachShader(m_RendererID, fragment);
				glLinkProgram(m_RendererID);
				wcUtil::checkCompileErrors(m_RendererID);
				// delete the shaders as they're linked into our program now and no longer necessary
				glDeleteShader(vertex);
				glDeleteShader(fragment);
			}
		}
		// activate the shader
		// ------------------------------------------------------------------------
		void use() const
		{
			glUseProgram(m_RendererID);
		}

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }

		void Destroy() {
			glDeleteProgram(m_RendererID);
			m_RendererID = 0;
		}
	};

	class ComputeShader {
		uint32_t m_RendererID = 0;
	public:
		ComputeShader() {}

		void Create(const char* path) {
			if (!m_RendererID) {
				std::vector<char> code = wcUtil::readFile(path);

				uint32_t compute = wcUtil::CompileShader(code, GL_COMPUTE_SHADER);

				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, compute);
				glLinkProgram(m_RendererID);
				wcUtil::checkCompileErrors(m_RendererID);
				// delete the shaders as they're linked into our program now and no longer necessary
				glDeleteShader(compute);
			}
		}
		// activate the shader
		// ------------------------------------------------------------------------
		void use() const
		{
			glUseProgram(m_RendererID);
		}

		void Dispatch(const GLuint& num_groups_x, const GLuint& num_groups_y, const GLuint& num_groups_z) {
			glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		}

		void Dispatch(const glm::vec3& num_groups) {
			glDispatchCompute(num_groups.x, num_groups.y, num_groups.z);
		}

		void Dispatch(const glm::vec2& num_groups) {
			glDispatchCompute(num_groups.x, num_groups.y, 1);
		}

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }
	};
}