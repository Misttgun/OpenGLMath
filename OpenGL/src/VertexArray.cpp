#include "VertexArray.h"

#include "VertexBufferLayout.h"
#include "Renderer.h"
#include <GL/glew.h>

VertexArray::VertexArray()
{
	GL_CALL(glGenVertexArrays(1, &mRendererId_));
}

VertexArray::~VertexArray()
{
	GL_CALL(glDeleteVertexArrays(1, &mRendererId_));
}

void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	bind();
	vb.bind();
	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GL_CALL(glEnableVertexAttribArray(i));
		GL_CALL(glVertexAttribPointer(i, element.count, element.type,
			element.normalized, layout.getStride(), reinterpret_cast<const void*>(offset)));
		offset += element.count * VertexBufferElement::getSizeOfType(element.type);
	}
}

void VertexArray::bind() const
{
	GL_CALL(glBindVertexArray(mRendererId_));
}

void VertexArray::unbind() const
{
	GL_CALL(glBindVertexArray(0));
}
