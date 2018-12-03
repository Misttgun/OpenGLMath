#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include <GL/glew.h>

Shader::Shader(const std::string& filepath)
	: mFilePath_(filepath), mRendererId_(0)

{
	ShaderProgramSource source = parseShader(filepath);
	mRendererId_ = createShader(source.vertexSource, source.fragmentSource);
}

Shader::~Shader()
{
	GL_CALL(glDeleteProgram(mRendererId_));
}

ShaderProgramSource Shader::parseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source)
{
	GL_CALL(unsigned int id = glCreateShader(type));
	const char* src = source.c_str();
	GL_CALL(glShaderSource(id, 1, &src, nullptr));
	GL_CALL(glCompileShader(id));

	int result;
	GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));
		GL_CALL(glGetShaderInfoLog(id, length, &length, message));
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
		std::cout << message << std::endl;
		GL_CALL(glDeleteShader(id));
		return 0;
	}

	return id;
}

unsigned int Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	GL_CALL(unsigned int program = glCreateProgram());
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GL_CALL(glAttachShader(program, vs));
	GL_CALL(glAttachShader(program, fs));
	GL_CALL(glLinkProgram(program));
	GL_CALL(glValidateProgram(program));

	GL_CALL(glDeleteShader(vs));
	GL_CALL(glDeleteShader(fs));

	return program;
}

void Shader::bind() const
{
	GL_CALL(glUseProgram(mRendererId_));
}

void Shader::unbind() const
{
	GL_CALL(glUseProgram(0));
}

void Shader::setUniform1I(const std::string& name, int value)
{
	GL_CALL(glUniform1i(getUniformLocation(name), value));
}

void Shader::setUniform1F(const std::string& name, float value)
{
	GL_CALL(glUniform1f(getUniformLocation(name), value));
}

void Shader::setUniform4F(const std::string& name, float v0, float v1, float v2, float v3)
{
	GL_CALL(glUniform4f(getUniformLocation(name), v0, v1, v2, v3));
}

void Shader::setUniformMat4F(const std::string& name, const glm::mat4& matrix)
{
	GL_CALL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

int Shader::getUniformLocation(const std::string& name)
{
	if (mUniformLocationCache_.find(name) != mUniformLocationCache_.end())
		return mUniformLocationCache_[name];

	GL_CALL(int location = glGetUniformLocation(mRendererId_, name.c_str()));
	if (location == -1)
		std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;

	mUniformLocationCache_[name] = location;
	return location;
}
