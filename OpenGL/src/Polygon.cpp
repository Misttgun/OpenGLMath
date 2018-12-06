#include "Polygon.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

Polygon::Polygon(float r, float g, float b)
	:mVertexSize_(0), mColor_{ r, g, b, 1.0f }, mTranslation_(0, 0, 0)
{
	mVertexArray_ = std::make_unique<VertexArray>();
	mVertexBuffer_ = std::make_unique<VertexBuffer>(nullptr, 0);
	VertexBufferLayout layout;
	layout.push<float>(2);
	mVertexArray_->addBuffer(*mVertexBuffer_, layout);
}

void Polygon::addPoint(float x, float y)
{
	mMousePoints_.push_back(x);
	mMousePoints_.push_back(y);

	mVertexSize_ = mMousePoints_.size() / 2;
	onUpdate();
}

void Polygon::onImGuiRender()
{
	ImGui::SliderFloat("TranslationX", &mTranslation_.x, 0.0f, 640.0f);
	ImGui::SliderFloat("TranslationY", &mTranslation_.y, 0.0f, 640.0f);
	ImGui::ColorEdit4("Color", mColor_);
	if (ImGui::Button("Clear"))
	{
		mMousePoints_.clear();
		mVertexSize_ = 0;
		onUpdate();
	}
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Polygon::onRender(const glm::mat4& vp, Shader* shader)
{
	Renderer renderer;

	glm::mat4 model = glm::translate(glm::mat4(1.0f), mTranslation_);
	glm::mat4 mvp = vp * model;
	shader->bind();
	shader->setUniformMat4F("u_MVP", mvp);
	shader->setUniform4F("u_Color", mColor_[0], mColor_[1], mColor_[2], mColor_[3]);
	renderer.draw(*mVertexArray_, mVertexSize_, *shader);
}

void Polygon::onUpdate()
{
	std::cout << mMousePoints_.size() << std::endl;
	mVertexBuffer_->edit(mMousePoints_.data(), mMousePoints_.size() * sizeof(float));
}
