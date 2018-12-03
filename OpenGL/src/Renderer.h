#pragma once

#include "VertexArray.h"
#include "Shader.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GL_CALL(x) GlClearError();\
	x;\
	ASSERT(GlLogCall(#x, __FILE__, __LINE__))

void GlClearError();
bool GlLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
	void clear() const;
	void draw(const VertexArray& va, unsigned int count, const Shader& shader) const;
};