#pragma once

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

struct ShaderProgramSource
{
	std::string vertexSource;
	std::string fragmentSource;
};

class Shader
{
private:
	std::string mFilePath_;
	unsigned int mRendererId_;
	std::unordered_map<std::string, int> mUniformLocationCache_;
public:
	Shader(const std::string& filepath);
	~Shader();

	void bind() const;
	void unbind() const;

	// Set uniforms
	void setUniform1I(const std::string& name, int value);
	void setUniform1F(const std::string& name, float value);
	void setUniform4F(const std::string& name, float v0, float v1, float v2, float v3);
	void setUniformMat4F(const std::string& name, const glm::mat4& matrix);
private:
	ShaderProgramSource parseShader(const std::string& filepath);
	unsigned int compileShader(unsigned int type, const std::string& source);
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);

	int getUniformLocation(const std::string& name);
};
