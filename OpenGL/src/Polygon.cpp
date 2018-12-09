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

void Polygon::sutherlandOgdmann(const std::unique_ptr<Polygon>& polygon, const std::unique_ptr<Polygon>& window)
{
    // - reset current points
    mMousePoints_.clear();
    mVertexSize_ = 0;

    // - clone polygon points
    for (auto i = 0; i < polygon->mVertexSize_; ++i)
        addPoint(polygon->mMousePoints_[i * 2], polygon->mMousePoints_[i * 2 + 1]);

    // - don't try to clip if the window is a line
    if (window->mVertexSize_ < 3)
        return;

    // - sutherland-hodgman algorithm
    for (auto i = 0; i < window->mVertexSize_; ++i)
    {
        int j = (i + 1) % window->mVertexSize_;

        float x1 = window->mMousePoints_[i * 2];
        float y1 = window->mMousePoints_[i * 2 + 1];
        float x2 = window->mMousePoints_[j * 2];
        float y2 = window->mMousePoints_[j * 2 + 1];

        clip(x1, y1, x2, y2);
    }
}

void Polygon::line_intersection(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float& xi, float& yi)
{
    // - compute x intersection
    float num = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
    float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    xi = num / den;

    // - compute y intersection
    num = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);
    yi = num / den;
}

// (x1, y1) (x2, y2) defines the clipping line
void Polygon::clip(float x1, float y1, float x2, float y2)
{
    std::vector<float> new_points;
    int new_size = 0;

    // - temporary variables used to get intersection between edges and clipping line
    float x_inter, y_inter;

    // i : current point, j : next point
    // ij = current edge of the polygon
    for (auto i = 0; i < mVertexSize_; ++i)
    {
        int j = (i + 1) % mVertexSize_;
        
        float xi = mMousePoints_[i * 2];
        float yi = mMousePoints_[i * 2 + 1];
        float xj = mMousePoints_[j * 2];
        float yj = mMousePoints_[j * 2 + 1];

        // - compute the position (inside / outside of the clipper line) of i and j
        float i_pos = (x2 - x1) * (yi - y1) - (y2 - y1) * (xi - x1);
        float j_pos = (x2 - x1) * (yj - y1) - (y2 - y1) * (xj - x1);

        // - both points inside
        if (i_pos < 0 && j_pos < 0)
        {
            // - add j to the new vertices list
            new_points.push_back(xj);
            new_points.push_back(yj);
            new_size++;
        }

        // - only first point outside
        else if (i_pos >= 0 && j_pos < 0)
        {
            // - add intersection point and j to the new vertices list
            line_intersection(x1, y1, x2, y2, xi, yi, xj, yj, x_inter, y_inter);

            new_points.push_back(x_inter);
            new_points.push_back(y_inter);
            new_points.push_back(xj);
            new_points.push_back(yj);

            new_size += 2;
        }

        // - only second point outside
        else if (i_pos < 0 && j_pos >= 0)
        {
            // - add intersection point to the new vertices list
            line_intersection(x1, y1, x2, y2, xi, yi, xj, yj, x_inter, y_inter);

            new_points.push_back(x_inter);
            new_points.push_back(y_inter);
            new_size++;
        }

        // - if both points are outisde, nothing to do
    }

    // - clear current points and clone result
    mMousePoints_.clear();
    mVertexSize_ = 0;

    for (auto i = 0; i < new_size; ++i)
        addPoint(new_points[i * 2], new_points[i * 2 + 1]);
}
