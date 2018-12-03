#pragma once

#include "Test.h"

#include "VertexBuffer.h"

#include <memory>
#include "VertexArray.h"
#include "Shader.h"

namespace test {

	class TestPolygon : public Test
	{
	public:
		TestPolygon();
		~TestPolygon();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	private:
		std::unique_ptr<VertexArray> mVao_;
		std::unique_ptr<VertexBuffer> mVertexBuffer_;
		std::unique_ptr<Shader> mShader_;

		glm::mat4 mProj_, mView_;
		glm::vec3 mTranslation_;
		float mColor_[4];
	};

}
