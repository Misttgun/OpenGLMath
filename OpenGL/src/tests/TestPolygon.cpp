#include "TestPolygon.h"

#include "Renderer.h"
#include "imgui/imgui.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GL/glew.h>
#include "VertexBufferLayout.h"

namespace test {

	TestPolygon::TestPolygon()
		: mProj_(glm::ortho(0.0f, 640.0f, 0.0f, 640.0f, -1.0f, 1.0f))
		, mView_(glm::translate(glm::mat4(1.0f)
			, glm::vec3(0, 0, 0))), mTranslation_(50, 50, 0)
		, mColor_{ 1.0f, 0.0f, 0.0f, 1.0f }
	{
		float positions[] = {
			-50.0f, -50.0f, // 0
			 50.0f, -50.0f, // 1
			 50.0f,  50.0f, // 2
			-50.0f,  50.0f  // 3
		};

		mVao_ = std::make_unique<VertexArray>();

		mVertexBuffer_ = std::make_unique<VertexBuffer>(positions, 4 * 2 * sizeof(float));
		VertexBufferLayout layout;
		layout.push<float>(2);
		mVao_->addBuffer(*mVertexBuffer_, layout);

		mShader_ = std::make_unique<Shader>("res/shaders/Basic.shader");
		mShader_->bind();
		mShader_->setUniform4F("u_Color", mColor_[0], mColor_[1], mColor_[2], mColor_[3]);
	}

	TestPolygon::~TestPolygon()
	{}

	void TestPolygon::OnUpdate(float deltaTime)
	{
	}

	void TestPolygon::OnRender()
	{
		GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

		Renderer renderer;

		glm::mat4 model = glm::translate(glm::mat4(1.0f), mTranslation_);
		glm::mat4 mvp = mProj_ * mView_ * model;
		mShader_->bind();
		mShader_->setUniformMat4F("u_MVP", mvp);
		mShader_->setUniform4F("u_Color", mColor_[0], mColor_[1], mColor_[2], mColor_[3]);
		renderer.draw(*mVao_, 4, *mShader_);
	}

	void TestPolygon::OnImGuiRender()
	{
		ImGui::SliderFloat2("Translation", &mTranslation_.x, 0.0f, 640.0f);
		ImGui::ColorEdit4("Color", mColor_);
	}
}