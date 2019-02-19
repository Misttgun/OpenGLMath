#include <cmath>
#include "Polygon.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

#include "imgui/imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Vector.h"
#include <algorithm>

// to use M_PI
const float M_PI = 3.14159265358979f;

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

Polygon::Polygon(Polygon&& p) : mVertexArray_(std::move(p.mVertexArray_)),
    mVertexBuffer_(std::move(p.mVertexBuffer_)), mVertexSize_(p.mVertexSize_), mTranslation_(p.mTranslation_)
    
{
    for (int i = 0; i < p.mEdges_.size(); i++)
        mEdges_.push_back(std::move(p.mEdges_[i]));

    for (int i = 0; i < 4; i++)
        mColor_[i] = p.mColor_[i];
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

void Polygon::onRenderFill(const glm::mat4& vp, Shader* shader)
{
    Renderer renderer;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), mTranslation_);
    glm::mat4 mvp = vp * model;
    shader->bind();
    shader->setUniformMat4F("u_MVP", mvp);
    shader->setUniform4F("u_Color", mColor_[0], mColor_[1], mColor_[2], mColor_[3]);
    fill_LCA();
    renderer.draw_line(*mVertexArray_, mVertexSize_, *shader);
}

void Polygon::onUpdate()
{
	mVertexBuffer_->edit(mMousePoints_.data(), mMousePoints_.size() * sizeof(float));
}

void Polygon::sutherlandOgdmann(const std::shared_ptr<Polygon>& polygon, const std::shared_ptr<Polygon>& window)
{
    // - reset current points
    mMousePoints_.clear();
    mEdges_.clear();
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
    {
        return;
    }
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
    mEdges_.clear();
    mMousePoints_.clear();
    mVertexSize_ = 0;

    for (auto i = 0; i < new_size; ++i)
        addPoint(floor(new_points[i * 2]), floor(new_points[i * 2 + 1]));
}

void Polygon::computeBoundingBox(std::shared_ptr<Polygon>& polygon)
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

    minY_ = y_min;

    // - fill the bounding box anti clockwise
    polygon->addPoint(x_min, y_min);
    polygon->addPoint(x_min, y_max);
    polygon->addPoint(x_max, y_max);
    polygon->addPoint(x_max, y_min);
}

void Polygon::fill_LCA()
{
    // - on ne peut pas remplir un point ou une ligne
    if (mEdges_.size() < 3)
        return;

    for (auto&& edge : mEdges_)
    {
        minY_ = std::min(minY_, edge->minY());
        maxY_ = std::max(maxY_, edge->maxY());
    }

    EdgeTable edge_table;
    EdgeTable active_edge_table;
    EdgeTable garbage;
    std::vector<float> lines;
    int y = minY_;

    fill_edge_table(edge_table);
    sort_edge_table(edge_table);

    do
    {
        update_active_edge(edge_table, active_edge_table, garbage, y);
        sort_active_edge_table(active_edge_table);
        compute_line_coordinates(active_edge_table, lines, y);
        update_x_bucket(active_edge_table);
        y++;
    } while ((!edge_table.empty() || !active_edge_table.empty()));

    mVertexBuffer_->edit(lines.data(), lines.size() * sizeof(float));
    mVertexSize_ = lines.size() / 2;

    clean_edge_table(garbage);
}

void Polygon::fill_edge_table(EdgeTable& et)
{
    // - on va créer un bucket par côté du polygone
    for (auto&& edge : mEdges_)
    {
        auto* bucket = new Bucket;
        bucket->y_min = edge->minY();
        bucket->y_max = edge->maxY();
        bucket->current_x = edge->y1_ < edge->y2_ ? edge->x1_ : edge->x2_;
        bucket->inv_dir = edge->getInvDir();

        et.push_back(bucket);
    }
}

void Polygon::update_active_edge(EdgeTable& et, EdgeTable& aet, EdgeTable& garbage, const int y) const
{
    // - on ajoute les côtés nouvellement intercepté dans la liste
    // des côtés actifs
    auto insert = std::begin(et);
    while (insert != std::end(et))
    {
        if ((*insert)->y_min == y)
        {
            aet.push_back((*insert));
            insert = et.erase(insert);
        }

        else
            ++insert;
    }

    // - on déplace les côtés qui ne sont plus actifs dans un container
    // trash
    auto remove = std::begin(aet);
    while (remove != std::end(aet))
    {
        if ((*remove)->y_max == y)
        {
            garbage.push_back((*remove));
            remove = aet.erase(remove);
        }

        else
            ++remove;
    }
}

void Polygon::sort_edge_table(EdgeTable& et) const
{
    // - trié selon l'ordre de priorité suivant : y_min > current_x > inv_dir
    std::sort(et.begin(), et.end(), [](Bucket* lhs, Bucket* rhs)
    {
        if (lhs->y_min == rhs->y_min)
        {
            if (lhs->current_x == rhs->current_x)
                return lhs->inv_dir < rhs->inv_dir;

            return lhs->current_x < rhs->current_x;
        }

        return lhs->y_min < rhs->y_min;
    });
}

void Polygon::sort_active_edge_table(EdgeTable& aet) const
{
    std::sort(aet.begin(), aet.end(), [](Bucket* lhs, Bucket* rhs)
    {
        return lhs->current_x < rhs->current_x;
    });
}

void Polygon::compute_line_coordinates(EdgeTable& aet, std::vector<float>& lines, const int y) const
{
    int i = 0;
    int second_last_index = aet.size() - 1;

    for (auto it = aet.begin(); it != aet.end(); ++it)
    {
        auto next = std::next(it, 1);

        // - on ajoute les lignes entre les côtés d'index pair et impair adjacents
        if (i < second_last_index && (i % 2 == 0))
        {
            lines.push_back(float(ceil((*it)->current_x)));
            lines.push_back(float(y));
            lines.push_back(float(floor((*next)->current_x)));
            lines.push_back(float(y));
        }
        i++;
    }
}

void Polygon::update_x_bucket(EdgeTable& aet) const
{
    for (auto edge : aet)
        edge->current_x += edge->inv_dir;
}

void Polygon::clean_edge_table(EdgeTable& et) const
{
    // - détruit les buckets créés pour la frame actuelle
    for (auto edge : et)
        delete edge;
}

void Polygon::ear_clipping()
{
    // - init variables
    std::vector<std::shared_ptr<Polygon>> triangulations;
    std::list<std::shared_ptr<Vertex>> vertex, convex_list, reflex_list, ear_list;
    init_ear_clipping(vertex, convex_list, reflex_list, ear_list);

    // - iterate over created list

}

void Polygon::create_vertex_list(VertexList &list)
{
    for (int i = 0; i < mVertexSize_; i++)
        list.push_back(std::make_shared<Vertex>(Vertex(mMousePoints_[2 * i], mMousePoints_[2 * i + 1])));
}

void Polygon::init_ear_clipping(VertexList& vertex_list, VertexList& convex_list, VertexList& reflex_list, VertexList& ear_list)
{
    // - store vertex as a list (seems to be more optimized for this algorithm)
    create_vertex_list(vertex_list);

    int i = 0;
    const int last_index = vertex_list.size() - 1;

    // set up convex and reflex list
    for (auto it = vertex_list.begin(); it != vertex_list.end(); ++it) 
    {
        const auto prev = i == 0 ? *(--vertex_list.end()) : *(std::prev(it, 1));
        const auto next = i == last_index ? *(vertex_list.begin()) : *(std::next(it, 1));

        Vector v1(*it->get(), *prev);
        Vector v2(*it->get(), *next);
        v1.normalized();
        v1.rotate_90();
        v2.normalized();

        if (v1.dot(v2) < 0)
            reflex_list.push_back(*it);
        else
            convex_list.push_back(*it);

        i++;
    }

    // setup ears

    /*
    std::cout << "REFLEX VERTEX COUNT : " << reflex_list.size() << std::endl;
    std::cout << "CONVEX VERTEX COUNT : " << convex_list.size() << std::endl;
    */
}
