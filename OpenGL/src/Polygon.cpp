#include "Polygon.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <GL/glew.h>

Polygon::Polygon()
	:mVertexSize_(0), mColor_{ 1.0f, 0.0f, 0.0f, 1.0f }, mTranslation_(0.0f, 0.0f, 0.0f)
{
	mVertexArray_ = std::make_unique<VertexArray>();
	mVertexBuffer_ = std::make_unique<VertexBuffer>(nullptr, 0);
	VertexBufferLayout layout;
	layout.push<float>(2);
	mVertexArray_->addBuffer(*mVertexBuffer_, layout);

	mShader_ = std::make_unique<Shader>("res/shaders/Basic.shader");
	mShader_->bind();
	mShader_->setUniform4F("u_Color", mColor_[0], mColor_[1], mColor_[2], mColor_[3]);
}

Polygon::~Polygon()
{
	if (mPosArray_)
		delete[] mPosArray_;
}

void Polygon::addPoint(float x, float y)
{
	mMousePoints_.push_back(x);
	mMousePoints_.push_back(y);

	mVertexSize_ = mMousePoints_.size() / 2;
	//mPosArray_ = mMousePoints_.data();
	onUpdate();
}

void Polygon::onImGuiRender()
{
	ImGui::SliderFloat2("Translation", &mTranslation_.x, 0.0f, 640.0f);
	ImGui::ColorEdit4("Color", mColor_);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Polygon::onRender(glm::mat4 vp)
{
	Renderer renderer;

	glm::mat4 model = glm::translate(glm::mat4(1.0f), mTranslation_);
	glm::mat4 mvp = model * vp;
	mShader_->bind();
	mShader_->setUniformMat4F("u_MVP", mvp);
	mShader_->setUniform4F("u_Color", mColor_[0], mColor_[1], mColor_[2], mColor_[3]);
	renderer.draw(*mVertexArray_, mVertexSize_, *mShader_);
}

void Polygon::onUpdate()
{
	std::cout << mMousePoints_.size() << std::endl;
	mVertexBuffer_->edit(mMousePoints_.data(), mMousePoints_.size() * sizeof(float));
}
