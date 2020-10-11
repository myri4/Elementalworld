#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>

#include <Utilitiess/Log.h>


namespace gl{
	enum class ShaderStatus {
		OK, FILE_NOT_SUCCESFULLY_READ
	};
class Shader{
public:
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	
	Shader (){}
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr){Create(vertexPath, fragmentPath, geometryPath);}
	ShaderStatus Create(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
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
		catch (std::ifstream::failure& e) { return ShaderStatus::FILE_NOT_SUCCESFULLY_READ; }
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
		return ShaderStatus::OK;
		}
		return ShaderStatus::OK;
	}
	ShaderStatus Create(const char* filepath) {
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
			else
				return ShaderStatus::FILE_NOT_SUCCESFULLY_READ;
			stream.close();

			std::string vertexCode;
			std::string fragmentCode;
			std::string geometryCode;
			vertexCode =   shaderStream[(int)ShaderType::VERTEX].str();
			fragmentCode = shaderStream[(int)ShaderType::FRAGMENT].str();
			geometryCode = shaderStream[(int)ShaderType::GEOMETRY].str();

			uint32_t vertex = CompileShader(vertexCode.c_str(), "vertex");
			uint32_t fragment = CompileShader(fragmentCode.c_str(), "fragment");
			uint32_t geometry;
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

			return ShaderStatus::OK;
		}
		return ShaderStatus::OK;
	}
	//void stringSource(const char*){}
	// activate the shader
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(m_RendererID);
	}
	void unUse()
	{
		glUseProgram(0);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const char* name, bool value)
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const char* name, int value)
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const char* name, float value)
	{
		glUniform1f(glGetUniformLocation(m_RendererID, name), value);
	}
	// ------------------------------------------------------------------------
	void setNum(const char* name, float value)
	{
		glUniform1f(glGetUniformLocation(m_RendererID, name), value);
	}
	// ------------------------------------------------------------------------
	void setNum(const char* name, int value)
	{
		glUniform1i(glGetUniformLocation(m_RendererID, name), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const char* name, const glm::vec2 &value)
	{
		glUniform2fv(glGetUniformLocation(m_RendererID, name), 1, glm::value_ptr(value));
	}
	void setVec2(const char* name, float x, float y)
	{
		glUniform2f(glGetUniformLocation(m_RendererID, name), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const char* name, const glm::vec3 &value)
	{
		glUniform3fv(glGetUniformLocation(m_RendererID, name), 1, glm::value_ptr(value));
	}
	void setVec3(const char* name, float x, float y, float z)
	{
		glUniform3f(glGetUniformLocation(m_RendererID, name), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const char* name, const glm::vec4 &value)
	{
		glUniform4fv(glGetUniformLocation(m_RendererID, name), 1, glm::value_ptr(value));
	}
	void setVec4(const char* name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(m_RendererID, name), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const char* name, const glm::mat2 &mat)
	{
		glUniformMatrix2fv(glGetUniformLocation(m_RendererID, name), 1, GL_FALSE, glm::value_ptr(mat));
	}
	// ------------------------------------------------------------------------
	void setMat3(const char* name, const glm::mat3 &mat)
	{
		glUniformMatrix3fv(glGetUniformLocation(m_RendererID, name), 1, GL_FALSE, glm::value_ptr(mat));
	}
	// ------------------------------------------------------------------------
	void setMat4(const char* name, const glm::mat4 &mat)
	{
		glUniformMatrix4fv(glGetUniformLocation(m_RendererID, name), 1, GL_FALSE, glm::value_ptr(mat));
	}
	void SetArray(const char* name, size_t size, int arr[]) {
		glUniform1iv(glGetUniformLocation(m_RendererID, name), size, arr);
	}

	void SetArray(const char* name, size_t size, float arr[]) {
		glUniform1fv(glGetUniformLocation(m_RendererID, name), size, arr);
	}
	uint32_t GetRendererID() {return m_RendererID;}
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
private:
	void checkCompileErrors(uint32_t shader, const char* type)
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
	void Compile() {

	}

	uint32_t ShaderTypeFromString(const char* type)
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
	uint32_t m_RendererID = 0;
};
void HandleErrors(ShaderStatus func) {
	if(func == ShaderStatus::FILE_NOT_SUCCESFULLY_READ) WC_ERROR("SHADER::FILE_NOT_SUCCESFULLY_READ");
}
}