#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>

#include <Utils/Log.hpp>

namespace gl{
class Shader{
public:
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	
	Shader () {}
	~Shader() { Destroy(); }
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr){ Create(vertexPath, fragmentPath, geometryPath); }
	void Create(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
		if (!m_RendererID) {			
		// 1. retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			// if geometry shader path is present, also load a geometry shader
			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure& e) { WC_ERROR("The shader was not found! Check your filepath!"); }
		// 1. compile shaders
		uint32_t vertex = CompileShader(vertexCode.c_str(), "vertex");
		uint32_t fragment = CompileShader(fragmentCode.c_str(), "fragment");
		checkCompileErrors(vertex, "VERTEX");
		checkCompileErrors(fragment, "FRAGMENT");
		// if geometry shader is given, compile geometry shader
		uint32_t geometry;
		if (geometryPath != nullptr) {
			geometry = CompileShader(geometryCode.c_str(), "geometry");
			checkCompileErrors(geometry, "geometry");
		}

		
		// shader Program
		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, vertex);
		glAttachShader(m_RendererID, fragment);
		if (geometryPath != nullptr)
			glAttachShader(m_RendererID, geometry);
		glLinkProgram(m_RendererID);
		checkCompileErrors(m_RendererID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);
		}
	}
	void Create(const char* filepath) {
		if (!m_RendererID) {
			std::string line;
			std::ifstream stream;
			std::stringstream shaderStream[3];
			enum class ShaderType {
				NONE = -1, VERTEX, FRAGMENT, GEOMETRY
			}type;
			
			// open files
			stream.open(filepath);
			if (stream.is_open()) {
				while (std::getline(stream, line)) {
					if (line.find("#shader") != std::string::npos || line.find("#type") != std::string::npos) {
					     if (line.find("vertex") != std::string::npos)	 type = ShaderType::VERTEX;
					else if (line.find("fragment") != std::string::npos) type = ShaderType::FRAGMENT;
					else if (line.find("geometry") != std::string::npos) type = ShaderType::GEOMETRY;
						
					}
					else 
						shaderStream[(int)type] << line << '\n';
				}
			}
			else WC_ERROR("The shader was not parsed correctly! Check your filepath or your shader file!");
			stream.close();

			std::string vertexCode;
			std::string fragmentCode;
			std::string geometryCode;
			vertexCode =   shaderStream[(int)ShaderType::VERTEX].str();
			fragmentCode = shaderStream[(int)ShaderType::FRAGMENT].str();
			geometryCode = shaderStream[(int)ShaderType::GEOMETRY].str();

			uint32_t vertex = CompileShader(vertexCode.c_str(), "vertex");
			uint32_t fragment = CompileShader(fragmentCode.c_str(), "fragment");
			uint32_t geometry = 0;
			if(!geometryCode.empty())geometry = CompileShader(geometryCode.c_str(), "geometry");

			// shader Program
			m_RendererID = glCreateProgram();
			glAttachShader(m_RendererID, vertex);
			glAttachShader(m_RendererID, fragment);
			if (!geometryCode.empty())
			glAttachShader(m_RendererID, geometry);
			glLinkProgram(m_RendererID);
			checkCompileErrors(m_RendererID, "PROGRAM");
			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			if (!geometryCode.empty())
			glDeleteShader(geometry);
		}
	}
	//void stringSource(const char*){}
	// activate the shader
	// ------------------------------------------------------------------------
	void use() const
	{
		int id;
		glGetIntegerv(GL_CURRENT_PROGRAM, &id);
		if (id != m_RendererID)	glUseProgram(m_RendererID);
	}
	void unUse() const
	{
		glUseProgram(0);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const char* name, const bool& value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform1i(loc, (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const char* name, const int& value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform1i(loc, value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const char* name, const float& value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform1f(loc, value);
	}
	// ------------------------------------------------------------------------
	void setNum(const char* name, const float& value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform1f(loc, value);
	}
	// ------------------------------------------------------------------------
	void setNum(const char* name, const int& value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform1i(loc, value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const char* name, const glm::vec2 &value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform2fv(loc, 1, glm::value_ptr(value));
	}
	void setVec2(const char* name, const float& x, const float& y) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform2f(loc, x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const char* name, const glm::vec3 &value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform3fv(loc, 1, glm::value_ptr(value));
	}
	void setVec3(const char* name, const float& x, const float& y, const float& z) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform3f(loc, x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const char* name, const glm::vec4 &value) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform4fv(loc, 1, glm::value_ptr(value));
	}
	void setVec4(const char* name, const float& x, const float& y, const float& z, const float& w) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform4f(loc, x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const char* name, const glm::mat2 &mat) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniformMatrix2fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
	}
	// ------------------------------------------------------------------------
	void setMat3(const char* name, const glm::mat3 &mat) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
	}
	// ------------------------------------------------------------------------
	void setMat4(const char* name, const glm::mat4 &mat) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
	}
	void SetArray(const char* name, const size_t& size, const int arr[]) const
	{
		auto loc = GetUnifLoc(name);
		if (loc != -1)
		glUniform1iv(loc, size, arr);
	}

	void SetArray(const char* name, size_t size, const float arr[]) const
	{
		auto loc = GetUnifLoc(name);
		if(loc != -1)
		glUniform1fv(loc, size, arr);
	}
	uint32_t GetRendererID() const {return m_RendererID;}

	void Destroy() { glDeleteProgram(m_RendererID); }
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
private:
	uint32_t m_RendererID = 0;

	mutable std::unordered_map<std::string, int> m_UniformCache;

	int GetUnifLoc(const std::string& name) const
	{
		if (m_UniformCache.find(name) != m_UniformCache.end())
			return m_UniformCache[name];

		int location = glGetUniformLocation(m_RendererID, name.c_str());
		m_UniformCache[name] = location;
		return location;
	}

	void checkCompileErrors(const uint32_t& shader, const char* type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
				WC_ERROR("SHADER_COMPILATION_ERROR of type: {0}\n{1}", type, infoLog);
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
				WC_ERROR("PROGRAM_LINKING_ERROR of type: {0}\n{1}", type, infoLog);
			}
		}
	}

	uint32_t ShaderTypeFromString(const char* type) const
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		if (type == "geometry")
			return GL_GEOMETRY_SHADER;

		return 0;
	}

	uint32_t CompileShader(const char* code, const char* type) {
		uint32_t shader;
		shader = glCreateShader(ShaderTypeFromString(type));
		glShaderSource(shader, 1, &code, nullptr);
		glCompileShader(shader);
		return shader;
	}
};
}
#endif