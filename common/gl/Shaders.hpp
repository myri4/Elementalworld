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
	void checkCompileErrors(const uint32_t& shader, const char* type) {
		int success;
		char infoLog[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
				WC_ERROR("SHADER_COMPILATION_ERROR of type: PROGRAM\n{0}", infoLog);
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
				WC_ERROR("PROGRAM_LINKING_ERROR of type: {0}\n{1}", type, infoLog);
			}
		}
	}

	uint32_t ShaderTypeFromString(const char* type) {
		if (type == "vertex") return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;
		if (type == "compute")	return GL_COMPUTE_SHADER;
		if (type == "geometry")	return GL_GEOMETRY_SHADER;

		return 0;
	}

	uint32_t CompileShader(const char* code, const char* type) {
		uint32_t shader;
		shader = glCreateShader(ShaderTypeFromString(type));
		glShaderSource(shader, 1, &code, nullptr);
		glCompileShader(shader);
		return shader;
	}

	void CheckForIncludes(std::string& line, std::stringstream& stream) {
		if (line.find("#include") != std::string::npos && line.find("//") == std::string::npos) {
			line.erase(0, 10);
			line.erase(line.size() - 1, 1);
			std::ifstream file(line);
			if (file.is_open())	while (std::getline(file, line))CheckForIncludes(line, stream);
			else WC_ERROR("Can`t find file at location: {0}", line.c_str());
			file.close();
		}
		else stream << line << '\n';
	}
}

namespace gl {
	class Shader {
	public:
		// constructor generates the shader on the fly
		// ------------------------------------------------------------------------

		Shader() {}
		~Shader() { Destroy(); }

		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) { Create(vertexPath, fragmentPath, geometryPath); }

		void Create(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
			if (!m_RendererID) {
				std::ifstream file(vertexPath);
				std::string line;
				std::stringstream vertexFile;
				std::stringstream fragmentFile;
				std::stringstream geometryFile;
				if (file.is_open()) while (std::getline(file, line)) wcUtil::CheckForIncludes(line, vertexFile); else WC_ERROR("Can`t find file in location: {0}", vertexPath);
				file.close();
				file.open(fragmentPath);
				if (file.is_open()) while (std::getline(file, line)) wcUtil::CheckForIncludes(line, fragmentFile); else WC_ERROR("Can`t find file in location: {0}", fragmentPath);

				file.close();
				if (geometryPath != nullptr)
				{
					file.open(geometryPath);
					if (file.is_open()) while (std::getline(file, line)) wcUtil::CheckForIncludes(line, geometryFile); else WC_ERROR("Can`t find file in location: {0}", geometryPath);
				}
				uint32_t vertex = wcUtil::CompileShader(vertexFile.str().c_str(), "vertex");
				uint32_t fragment = wcUtil::CompileShader(fragmentFile.str().c_str(), "fragment");
				wcUtil::checkCompileErrors(vertex, "VERTEX");
				wcUtil::checkCompileErrors(fragment, "FRAGMENT");
				uint32_t geometry = 0;
				if (geometryPath != nullptr) {
					geometry = wcUtil::CompileShader(geometryFile.str().c_str(), "geometry");
					wcUtil::checkCompileErrors(geometry, "geometry");
				}

				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, vertex);
				glAttachShader(m_RendererID, fragment);
				if (geometryPath != nullptr) glAttachShader(m_RendererID, geometry);
				glLinkProgram(m_RendererID);
				wcUtil::checkCompileErrors(m_RendererID, "PROGRAM");
				// delete the shaders as they're linked into our program now and no longer necessary
				glDeleteShader(vertex);
				glDeleteShader(fragment);
				if (geometryPath != nullptr) glDeleteShader(geometry);
			}
		}

		void Create(const char* filepath) {
			if (!m_RendererID) {
				std::string line;
				std::ifstream stream;
				std::stringstream shaderStream[3];

				int type = -1;

				// open files
				stream.open(filepath);
				if (stream.is_open()) {
					while (getline(stream, line)) {

						if (line.find("#shader") != std::string::npos || line.find("#type") != std::string::npos && line.find("//") == std::string::npos) {
							if (line.find("vertex") != std::string::npos)	 type = 0; // VERTEX
							else if (line.find("fragment") != std::string::npos) type = 1; // FRAGMENT
							else if (line.find("geometry") != std::string::npos) type = 2; // GEOMETRY
						}
						else wcUtil::CheckForIncludes(line, shaderStream[type]);
					}
				}
				else WC_ERROR("The shader was not parsed correctly! Check your filepath or your shader file! {0}", filepath);
				stream.close();

				std::string vertexCode = shaderStream[0].str();
				std::string fragmentCode = shaderStream[1].str();
				std::string geometryCode = shaderStream[2].str();

				uint32_t vertex = wcUtil::CompileShader(vertexCode.c_str(), "vertex");
				uint32_t fragment = wcUtil::CompileShader(fragmentCode.c_str(), "fragment");
				uint32_t geometry = 0;
				if (!geometryCode.empty())geometry = wcUtil::CompileShader(geometryCode.c_str(), "geometry");

				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, vertex);
				glAttachShader(m_RendererID, fragment);
				if (!geometryCode.empty())
					glAttachShader(m_RendererID, geometry);
				glLinkProgram(m_RendererID);
				wcUtil::checkCompileErrors(m_RendererID, "PROGRAM");
				// delete the shaders as they're linked into our program now and no longer necessary
				glDeleteShader(vertex);
				glDeleteShader(fragment);
				if (!geometryCode.empty()) glDeleteShader(geometry);
			}
		}
		// activate the shader
		// ------------------------------------------------------------------------
		void use() const
		{
			glUseProgram(m_RendererID);
		}

		static void unUse()
		{
			glUseProgram(0);
		}

		inline operator uint32_t& () { return m_RendererID; }
		inline operator const uint32_t& () const { return m_RendererID; }

		void Destroy() {
			glDeleteProgram(m_RendererID);
			m_RendererID = 0;
		}
		// ------------------------------------------------------------------------
	private:
		uint32_t m_RendererID = 0;
	};

	class ComputeShader {
	public:

		ComputeShader() {}

		void Create(const char* path) {
			if (!m_RendererID) {
				std::ifstream file(path);
				std::string line;
				std::stringstream shaderFile;
				if (file.is_open()) while (std::getline(file, line)) shaderFile << line << '\n';
				else WC_ERROR("Can`t find file in location: {0}", path);
				file.close();

				uint32_t compute = wcUtil::CompileShader(shaderFile.str().c_str(), "compute");
				wcUtil::checkCompileErrors(compute, "COMPUTE");

				// shader Program
				m_RendererID = glCreateProgram();
				glAttachShader(m_RendererID, compute);
				glLinkProgram(m_RendererID);
				wcUtil::checkCompileErrors(m_RendererID, "PROGRAM");
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

		static void unUse()
		{
			glUseProgram(0);
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
	private:
		uint32_t m_RendererID = 0;
	};
}