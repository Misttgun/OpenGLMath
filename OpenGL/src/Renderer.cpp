#include "Renderer.h"

#include <iostream>
#include <GL/glew.h>

void GlClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool GlLogCall(const char* function, const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;

		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}

		std::cout << "[OpenGL Error] " << error << ": "
			<< function << " " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

void Renderer::clear() const
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::draw(const VertexArray& va, const unsigned int count, const Shader& shader) const
{
	shader.bind();
	va.bind();
	GL_CALL(glDrawArrays(GL_LINE_LOOP, 0, count));
}


void Renderer::draw_line(const VertexArray& va, const unsigned int count, const Shader& shader) const
{
    shader.bind();
    va.bind();
    GL_CALL(glDrawArrays(GL_LINES, 0, count));
}
