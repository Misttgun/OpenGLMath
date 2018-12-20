#include <cmath>
#include "Polygon.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <algorithm>

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

    // - clear and recreate edges (can be optimized : remove last entry and create 2 
    // new edge : [last, current], [current, first]
    mEdges_.clear();
    for (int i = 0; i < mVertexSize_; i++)
	{
        int next = (i + 1) % mVertexSize_;

        mEdges_.push_back(std::make_unique<Edge>(mMousePoints_[i * 2], mMousePoints_[i * 2 + 1], mMousePoints_[next * 2], mMousePoints_[next * 2 + 1]));
	}

    // - sort edges by y_min
    
    std::sort(mEdges_.begin(), mEdges_.end(), [](const std::unique_ptr<Edge>& lhs, const std::unique_ptr<Edge>& rhs)
    {
        if (lhs->minY() == rhs->minY())
            return lhs->getInvDir() < rhs->getInvDir();

        return lhs->minY() < rhs->minY();
    });
	
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
	mVertexBuffer_->edit(mMousePoints_.data(), mMousePoints_.size() * sizeof(float));
}

void Polygon::sutherlandOgdmann(const std::unique_ptr<Polygon>& polygon, const std::unique_ptr<Polygon>& window)
{
    // - reset current points
    mMousePoints_.clear();
    mVertexSize_ = 0;

    // - clone polygon points
    for (auto i = 0; i < polygon->mVertexSize_; ++i)
    {
        // - compute translated coordinates of the polygon
        glm::vec3 coor(polygon->mMousePoints_[i * 2], polygon->mMousePoints_[i * 2 + 1], 0.0f);
        glm::vec3 res = polygon->mTranslation_ + coor;

        addPoint(res.x, res.y);
    }

    // - don't try to clip if the window is a line
    if (window->mVertexSize_ < 3)
        return;

    // - sutherland-hodgman algorithm
    for (auto i = 0; i < window->mVertexSize_; ++i)
    {
        int j = (i + 1) % window->mVertexSize_;

        glm::vec3 tr = window->mTranslation_;

        float x1 = window->mMousePoints_[i * 2] + tr.x;
        float y1 = window->mMousePoints_[i * 2 + 1] + tr.y;
        float x2 = window->mMousePoints_[j * 2] + tr.x;
        float y2 = window->mMousePoints_[j * 2 + 1] + tr.y;

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

            new_points.push_back(floor(x_inter));
            new_points.push_back(floor(y_inter));
            new_points.push_back(floor(xj));
            new_points.push_back(floor(yj));

            new_size += 2;
        }

        // - only second point outside
        else if (i_pos < 0 && j_pos >= 0)
        {
            // - add intersection point to the new vertices list
            line_intersection(x1, y1, x2, y2, xi, yi, xj, yj, x_inter, y_inter);

            new_points.push_back(floor(x_inter));
            new_points.push_back(floor(y_inter));
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

void Polygon::computeBoundingBox(std::unique_ptr<Polygon>& polygon)
{
    // - clear previous vertices
    polygon->mVertexSize_ = 0;
    polygon->mMousePoints_.clear();

    // - return if the polygon is not created yet
    if (mVertexSize_ < 3)
        return;

    int i = 0;

    float x_min, y_min, x_max, y_max;

    for (auto point : mMousePoints_)
    {
        // - init x
        if (i == 0)
        {
            x_min = point;
            x_max = point;
        }

        // - init y
        else if (i == 1)
        {
            y_min = point;
            y_max = point;
        }

        // - update values
        else
        {
            // - update x
            if (i % 2 == 0)
            {
                if (point > x_max)
                    x_max = point;

                if (point < x_min)
                    x_min = point;
            }

            // - update y
            else
            {
                if (point > y_max)
                    y_max = point;

                if (point < y_min)
                    y_min = point;
            }
        }

        i++;
    }

    // - fill the bounding box anti clockwise
    polygon->addPoint(x_min, y_min);
    polygon->addPoint(x_min, y_max);
    polygon->addPoint(x_max, y_max);
    polygon->addPoint(x_max, y_min);

    minY_ = floor(y_min);
    maxY_ = ceil(y_max);
}

// the function is still in developpment and a bit dirty, 
// it will be subdivided in smaller method later
void Polygon::fill(Shader* shader)
{
    Renderer renderer;

    auto vertexArray = std::make_unique<VertexArray>();
    auto vertexBuffer = std::make_unique<VertexBuffer>(nullptr, 0);
    VertexBufferLayout layout;
    layout.push<float>(2);
    vertexArray->addBuffer(*vertexBuffer, layout);

    if (mEdges_.size() < 3)
        return;

    // - create SI structure
    std::vector<Bucket*> si;

    for (int i = minY_; i <= maxY_; i++)
    {
        bool change = false;
        bool first = true;

        // - first iteration : will the list change ?
        for (auto&& edge : mEdges_)
        {
            if (static_cast<int>(edge->minY()) == i )
            {
                change = true;
                break;
            }
        }

        // - if there is no new edge in the list, move to next line
        if (!change)
            continue;


        // - we add or remove a segment from the list
        for (auto&& edge : mEdges_)
        {
            
            // - edge is not intercepted by current line
            if (edge->minY() > i || edge->maxY() <= i)
                continue;

            // - create a new bucket for the edge
            Bucket* b = new Bucket;
            b->y_min = edge->minY();
            b->y_max = edge->maxY();
            b->current_x = edge->y1_ <= edge->y2_ ? edge->x1_ : edge->x2_;
            b->inv_dir = edge->getInvDir();
            b->next = nullptr;

            // - we add the first edge in the new entry
            if (first)
            {
                si.push_back(b);
                first = false;
            }

            else
            {
                si.back()->next = b;
            }
        }
    }

    // USE THE SI AS AN INPUT FOR THE LCA ALGORITHM
    Bucket* LCA = nullptr;
    Bucket* tmp = nullptr;
    Bucket* next = nullptr;
    int si_index = 0;
    std::vector<float> lines;

    for (int i = minY_; i < maxY_; i++)
    {
        bool change = false;

        // - first iteration : will the list change ?
        for (auto&& edge : mEdges_)
        {
            if (static_cast<int>(edge->minY()) == i)
            {
                change = true;
                break;
            }
        }

        // go to the next entry of the SI
        if (change && si_index < si.size())
        {
            LCA = si[si_index];
            si_index++;
        }

        // - create line from the current entry
        tmp = LCA;
        next = LCA->next;
        
        while (tmp != nullptr && next != nullptr)
        {
            lines.push_back(float((tmp->current_x) + (float(tmp->inv_dir) * (i - float(tmp->y_min)))));
            lines.push_back(static_cast<float>(i));
            lines.push_back(float(next->current_x) + (float(next->inv_dir) * (i - float(next->y_min))));
            lines.push_back(i);
            
            tmp = next;
            next = tmp->next;
        }
    }
    vertexBuffer->edit(lines.data(), lines.size() * sizeof(float));
    renderer.draw_line(*vertexArray, lines.size() * 4, *shader);

    // print && clear the SI
    int i = 0;
    for (auto s : si)
    {
        // delete buckets
        Bucket* tmp = s;
        Bucket* next = s->next;

        while (tmp != nullptr)
        {
            delete tmp;
            tmp = next;

            if (next)
                next = next->next;
        }
        i++;
    }
}

void Polygon::fill_LCA(Shader* shader)
{
    Renderer renderer;

    auto vertexArray = std::make_unique<VertexArray>();
    auto vertexBuffer = std::make_unique<VertexBuffer>(nullptr, 0);
    VertexBufferLayout layout;
    layout.push<float>(2);
    vertexArray->addBuffer(*vertexBuffer, layout);

    if (mEdges_.size() < 3)
        return;

    // - création de l'edge table
    std::vector<Bucket*> edge_table;

    for (auto&& edge: mEdges_)
    {
        Bucket* bucket = new Bucket;
        bucket->y_min = edge->minY();
        bucket->y_max = edge->maxY();
        bucket->current_x = edge->y1_ < edge->y2_ ? edge->x1_ : edge->x2_;
        bucket->inv_dir = edge->getInvDir();
        bucket->next = nullptr;

        edge_table.push_back(bucket);
    }

    // - create de l'active edge table
    std::vector<Bucket*> active_edge_table;

    // - tri de l'edge table en fonction de y_min, x_min et w
    std::sort(edge_table.begin(), edge_table.end(), [](Bucket* lhs, Bucket* rhs)
    {
        if (lhs->y_min == rhs->y_min)
        {
          if (lhs->current_x == rhs->current_x)
              return lhs->inv_dir < rhs->inv_dir;

          return lhs->current_x < rhs->current_x;
        }

        return lhs->y_min < rhs->y_min;
    });

    // - set start y
    int y = minY_;

    std::vector<Bucket*> garbage;
    std::vector<float> lines;

    do
    {
        // - insert new edge in active edge list
        auto insert = std::begin(edge_table);
        while (insert != std::end(edge_table)) 
        {
            if ((*insert)->y_min == y)
            {
                active_edge_table.push_back((*insert));
                insert = edge_table.erase(insert);
            }

            else
                ++insert;
        }

        // - remove "terminated" edge from active edge list
        auto remove = std::begin(active_edge_table);
        while (remove != std::end(active_edge_table))
        {
            if ((*remove)->y_max == y)
            {
                garbage.push_back((*remove));
                remove = active_edge_table.erase(remove);
            }

            else
                ++remove;
        }

        // - sort active edge table by x (may be not necessary)
        std::sort(active_edge_table.begin(), active_edge_table.end(), [](Bucket* lhs, Bucket* rhs)
        {
            return lhs->current_x < rhs->current_x;
        });

        // - add lines
        int i = 0;
        for (auto it = active_edge_table.begin(); it != active_edge_table.end(); it++)
        {
            auto next = std::next(it, 1);
            if (i < active_edge_table.size() - 1 && (i % 2 == 0))
            {
                lines.push_back(float(ceil((*it)->current_x)));
                lines.push_back(float(y));
                lines.push_back(float(floor((*next)->current_x)));
                lines.push_back(float(y));
            }

            i++;
        }

        y++;

        for (auto edge: active_edge_table)
            edge->current_x += edge->inv_dir;
        
    } while ((!edge_table.empty() || !active_edge_table.empty()));

    // draw lines
    vertexBuffer->edit(lines.data(), lines.size() * sizeof(float));
    renderer.draw_line(*vertexArray, lines.size() * 4, *shader);

    // - clean garbage
    for (auto edge : garbage)
        delete edge;
}