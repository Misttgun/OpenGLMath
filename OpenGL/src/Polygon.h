#pragma once

#include <memory>
#include <vector>

#include "VertexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"


class Polygon
{
public:
	Polygon();
	~Polygon();

	void addPoint(float x, float y);
	void onImGuiRender();
	void onRender(glm::mat4 vp);
	void onUpdate();

private:
	std::unique_ptr<VertexArray> mVertexArray_;
	std::unique_ptr<VertexBuffer> mVertexBuffer_;
	std::unique_ptr<Shader> mShader_;

	std::vector<float> mMousePoints_;
	float* mPosArray_;
	int mVertexSize_;

	float mColor_[4];
	glm::vec3 mTranslation_;
};

