#include "VertexBuffer.h"

#include "Renderer.h"
#include <GL/glew.h>

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
	GL_CALL(glGenBuffers(1, &mRendererId_));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mRendererId_));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &mRendererId_));
}

void VertexBuffer::bind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mRendererId_));
}

void VertexBuffer::unbind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::edit(const void * data, unsigned int size)
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mRendererId_));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
