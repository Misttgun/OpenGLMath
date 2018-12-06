#pragma once

#include <memory>
#include <vector>

#include "VertexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"


class Polygon
{
public:
	Polygon(float r = 1.0f, float g = 1.0f, float b = 1.0f);
	~Polygon() = default;

	void addPoint(float x, float y);
	void onImGuiRender();
	void onRender(glm::mat4 vp, Shader* shader);
	void onUpdate();

private:
	std::unique_ptr<VertexArray> mVertexArray_;
	std::unique_ptr<VertexBuffer> mVertexBuffer_;

	std::vector<float> mMousePoints_;
	int mVertexSize_;

	float mColor_[4];
	glm::vec3 mTranslation_;
};

