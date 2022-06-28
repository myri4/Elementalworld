#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>

#include <Utils/Log.h>

namespace wcUtil {

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
		uint32_t bindingAttribs = 0;
	public:
		Shader() = default;
		bool depthTest = false;

		void Create(const std::string& vertexPath, const std::string& fragmentPath) {
			if (!m_RendererID) {
				uint32_t vertex = wcUtil::CompileShader(wcUtil::readFile(vertexPath), GL_VERTEX_SHADER);
				uint32_t fragment = wcUtil::CompileShader(wcUtil::readFile(fragmentPath), GL_FRAGMENT_SHADER);
				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, vertex);
				glAttachShader(m_RendererID, fragment);
				glLinkProgram(m_RendererID);

				int success;
				char infoLog[1024];
				glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(m_RendererID, 1024, nullptr, infoLog);
					WC_ERROR("PROGRAM_LINKING_ERROR: \n{0}", infoLog);
				}
				// delete the shaders as they're linked into our program now and no longer necessary
				glDeleteShader(vertex);
				glDeleteShader(fragment);
			}

			glCreateVertexArrays(1, &bindingAttribs);
		}

		void SetVertexBuffer(const GLuint& VBO, const GLuint& stride, const GLuint& offset = 0, const GLuint& binding = 0) {
			glVertexArrayVertexBuffer(bindingAttribs, binding, VBO, offset, stride);
		}

		void SetIndexBuffer(const GLuint& EBO) {
			glVertexArrayElementBuffer(bindingAttribs, EBO);
		}

		void VertexAttribPointer(const GLuint& index, const int& size, const GLuint& offset, const GLenum& type = GL_FLOAT, const bool& normalized = false) {
			glEnableVertexArrayAttrib(bindingAttribs, index);
			glVertexArrayAttribFormat(bindingAttribs, index, size, type, normalized, offset);
			glVertexArrayAttribBinding(bindingAttribs, index, 0);
		}

		void use() const { 
			glUseProgram(m_RendererID);
			glBindVertexArray(bindingAttribs);
			if (depthTest) 
				glEnable(GL_DEPTH_TEST);
			else 
				glDisable(GL_DEPTH_TEST);
		}

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }
	};

	class ComputeShader {
		uint32_t m_RendererID = 0;
	public:
		ComputeShader() = default;

		void Create(const std::string& path) {
			if (!m_RendererID) {
				uint32_t compute = wcUtil::CompileShader(wcUtil::readFile(path), GL_COMPUTE_SHADER);

				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, compute);
				glLinkProgram(m_RendererID);

				glDeleteShader(compute);
			}
		}

		void use() const { glUseProgram(m_RendererID); }

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