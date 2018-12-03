#pragma once

#include <vector>
#include "Renderer.h"
#include <GL/glew.h>

struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int getSizeOfType(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT:			return 4;
			case GL_UNSIGNED_INT:	return 4;
			case GL_UNSIGNED_BYTE:	return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> mElements_;
	unsigned int mStride_;
public:
	VertexBufferLayout()
		: mStride_(0) {}

	template<typename T>
	void push(unsigned int count)
	{
		static_assert(false);
	}

	template<>
	void push<float>(unsigned int count)
	{
		mElements_.push_back({ GL_FLOAT, count, GL_FALSE });
		mStride_ += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
	}

	template<>
	void push<unsigned int>(unsigned int count)
	{
		mElements_.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		mStride_ += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	void push<unsigned char>(unsigned int count)
	{
		mElements_.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		mStride_ += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
	}

	inline std::vector<VertexBufferElement> getElements() const { return mElements_; }
	inline unsigned int getStride() const { return mStride_; }
};