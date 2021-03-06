#include <cmath>
#include "Polygon.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"
#include "Utils.h"

#include "imgui/imgui.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Vector.h"
#include <algorithm>
#include "PolygonManager.h"

// to use M_PI
const float M_PI = 3.14159265358979f;

Polygon::Polygon(float r, float g, float b, unsigned int id)
    :mVertexSize_(0), mColor_{ r, g, b, 1.0f }, mTranslation_(0, 0, 0), mEdges_(), id_(id)
{
    mVertexArray_ = std::make_unique<VertexArray>();
    mVertexBuffer_ = std::make_unique<VertexBuffer>(nullptr, 0);
    VertexBufferLayout layout;
    layout.push<float>(2);
    mVertexArray_->addBuffer(*mVertexBuffer_, layout);
    minY_ = -1;
    maxY_ = -1;
}

Polygon::Polygon(Polygon&& p) : mVertexArray_(std::move(p.mVertexArray_)), mEdges_(),
mVertexBuffer_(std::move(p.mVertexBuffer_)), mVertexSize_(p.mVertexSize_), mTranslation_(p.mTranslation_)
{
    for (int i = 0; i < p.mEdges_.size(); i++)
        mEdges_.push_back(std::move(p.mEdges_[i]));

    for (int i = 0; i < 4; i++)
        mColor_[i] = p.mColor_[i];

    minY_ = p.minY_;
    maxY_ = p.maxY_;
    id_ = p.id_;
}

void Polygon::addPoint(float x, float y)
{
    mMousePoints_.push_back(x);
    mMousePoints_.push_back(y);

    mVertexSize_ = mMousePoints_.size() / 2;

    update_edges();
}

void Polygon::onImGuiRenderPolygon()
{
    std::string id = std::to_string(id_);
    std::string tr_x_id = "X translation_" + id;
    std::string tr_y_id = "Y translation_" + id;
    std::string color_id = "Color_" + id;
    std::string clear_id =  "Clear_" + id;

    ImGui::SliderFloat(tr_x_id.c_str(), &mTranslation_.x, 0.0f, 640.0f);
    ImGui::SliderFloat(tr_y_id.c_str(), &mTranslation_.y, 0.0f, 640.0f);
    ImGui::ColorEdit4(color_id.c_str(), mColor_);
    
    if (ImGui::Button(clear_id.c_str()))
    {
        mMousePoints_.clear();
        mVertexSize_ = 0;
        onUpdate();
        PolygonManager::get()->delete_polygon(this);
    }
}

void Polygon::onImGuiRenderWindow()
{
    std::string id = std::to_string(id_);
    std::string tr_x_id = "X translation_" + id;
    std::string tr_y_id = "Y translation_" + id;
    std::string color_id = "Color_" + id;
    std::string clear_id = "Clear_" + id;

    ImGui::SliderFloat(tr_x_id.c_str(), &mTranslation_.x, 0.0f, 640.0f);
    ImGui::SliderFloat(tr_y_id.c_str(), &mTranslation_.y, 0.0f, 640.0f);
    ImGui::ColorEdit4(color_id.c_str(), mColor_);

    if (ImGui::Button(clear_id.c_str()))
    {
        mMousePoints_.clear();
        mVertexSize_ = 0;
        onUpdate();
        PolygonManager::get()->delete_window(this);
    }
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
    // - on va cr�er un bucket par c�t� du polygone
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
    // - on ajoute les c�t�s nouvellement intercept� dans la liste
    // des c�t�s actifs
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

    // - on d�place les c�t�s qui ne sont plus actifs dans un container
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
    // - tri� selon l'ordre de priorit� suivant : y_min > current_x > inv_dir
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

        // - on ajoute les lignes entre les c�t�s d'index pair et impair adjacents
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
    // - d�truit les buckets cr��s pour la frame actuelle
    for (auto edge : et)
        delete edge;
}




void Polygon::ear_clipping(std::vector<std::shared_ptr<Polygon>> &vector)
{
    // - init variables
    std::vector<std::shared_ptr<Polygon>> triangulations;
    VertexList vertex, convex_list, reflex_list, ear_list;
    init_ear_clipping(vertex, convex_list, reflex_list, ear_list);

    float offset_x = this->mTranslation_[0];
    float offset_y = this->mTranslation_[1];

    if (vertex.size() == 3)
    {
        vector.push_back(std::make_shared<Polygon>(Polygon(1.0f, 0.5f, 1.0f)));
        for (const auto& v : vertex)   
            vector.back()->addPoint(v->x + offset_x, v->y + offset_y);
        vector.back()->update_edges();
        return;
    }

    // - iterate over created list
    while (!ear_list.empty() && vertex.size() > 1)
    {
        VertexPtr current_ear = ear_list.front();
        VertexListIterator it = std::find(vertex.begin(), vertex.end(), current_ear);

        // - get adjacent Vertex
        auto prev = it == vertex.begin() ? --vertex.end() : std::prev(it, 1);
        auto next = it == --vertex.end() ? vertex.begin() : std::next(it, 1);

        // - add in polygon
        vector.push_back(std::make_shared<Polygon>(Polygon(1.0f, 0.5f, 1.0f)));
        vector.back()->addPoint((*prev)->x + offset_x, (*prev)->y + offset_y);
        vector.back()->addPoint((*it)->x + offset_x, (*it)->y + offset_y);
        vector.back()->addPoint((*next)->x + offset_x, (*next)->y + offset_y);
        vector.back()->update_edges();

        // update lists
        ear_list.pop_front();
        erase_from_list(*it, vertex);

        // start by next, because nea ear are add to front
        update_vertex(*next, vertex, convex_list, reflex_list, ear_list);
        update_vertex(*prev, vertex, convex_list, reflex_list, ear_list);
    }
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

        if (is_reflex(*it, prev, next))
            reflex_list.push_back(*it);
        else
        {
            convex_list.push_back(*it);
        }
        i++;
    }

    // setup ears
    i = 0;
    for (auto it = vertex_list.begin(); it != vertex_list.end(); ++it)
    {
        // - ignore reflex vertex 
        if (is_in_list(*it, reflex_list))
            continue;

        if (is_ear(it, vertex_list, reflex_list))
            ear_list.push_back(*it);
    }
}

bool Polygon::is_reflex(const VertexPtr& v, const VertexPtr& prev, const VertexPtr& next)
{
    Vector v1(*v, *prev);
    Vector v2(*v, *next);

    v1.normalized();
    v1.rotate_90();
    v2.normalized();

    return (v1.dot(v2) < 0);
}

void Polygon::update_vertex(VertexPtr& v, VertexList& vertex_list, VertexList& convex_list, VertexList& reflex_list, VertexList& ear_list)
{
    // check if convex vertex was an ear
    if (is_in_list(v, convex_list) && is_in_list(v, ear_list))
    {
        auto it = get_in_list(v, vertex_list);
        
        // vertex is no longer an ear
        if (!is_ear(it, vertex_list, reflex_list))
            erase_from_list(v, ear_list);

        return;
    }

    // reflex vertex can become convex and become and ear
    if (is_in_list(v, reflex_list))
    {
        auto it = get_in_list(v, vertex_list);
        const auto prev = it == vertex_list.begin() ? *(--vertex_list.end()) : *(std::prev(it, 1));
        const auto next = it == --vertex_list.end() ? *vertex_list.begin() : *(std::next(it, 1));

        // still reflex, nothing change
        if (is_reflex(v, prev, next))
            return;


        move_to_list(v, reflex_list, convex_list);

        if (is_ear(it, vertex_list, reflex_list))
            ear_list.push_front(v);
    }
}

bool Polygon::is_ear(VertexListIterator& it, VertexList& vertex_list, VertexList& reflex_list)
{
    const auto prev = it == vertex_list.begin() ? *(--vertex_list.end()) : *(std::prev(it, 1));
    const auto next = it == --vertex_list.end() ? *vertex_list.begin() : *(std::next(it, 1));

    bool has_reflex_inside = false;

    for (const auto& vertex : reflex_list)
    {
        // - ignore adjacent vertex
        if ((vertex == prev) || (vertex == (*it)) || (vertex == next))
            continue;

        if (Utils::get()->in_triangle(Vector(*prev), Vector(*it->get()), Vector(*next), Vector(*vertex)))
        {
            has_reflex_inside = true;
            break;
        }
    }
    
    return !has_reflex_inside;
}

bool Polygon::is_in_list(VertexPtr& v, VertexList& vertex_list)
{
    const auto it = std::find(vertex_list.begin(), vertex_list.end(), v);
    
    return it != vertex_list.end();
}

VertexListIterator Polygon::get_in_list(VertexPtr& v, VertexList& vertex_list)
{
    const auto it = std::find(vertex_list.begin(), vertex_list.end(), v);

    return it;
}

void Polygon::erase_from_list(VertexPtr& v, VertexList& vertex_list)
{
    const auto it = get_in_list(v, vertex_list);

    vertex_list.erase(it);
}

void Polygon::move_to_list(VertexPtr& v, VertexList& src, VertexList& dest)
{
    const auto it = get_in_list(v, src);

    dest.push_front(v);
    src.erase(it);
}

void Polygon::fractalise()
{
    const int size = mMousePoints_.size();
    mMousePoints_.resize(2 * size, 0.f);
    const int new_size = mMousePoints_.size();

    // - make space for new values
    for (int i = new_size - 3; i > 1; i-= 4)
    {
        mMousePoints_[i] = mMousePoints_[i / 2 + 1];
        mMousePoints_[i - 1] = mMousePoints_[i / 2];
    }

    // - double add barycenter of current edges to the list of vertex
    for (int i = 2; i < new_size - 1; i+= 4)
    {
        float x1, y1, x2, y2;
        if (i != new_size - 2)
        {
            x1 = mMousePoints_[i - 2];
            x2 = mMousePoints_[i + 2];
            y1 = mMousePoints_[i - 1];
            y2 = mMousePoints_[i + 3];
        }

        else
        {
            x1 = mMousePoints_[i - 2];
            x2 = mMousePoints_[0];
            y1 = mMousePoints_[i - 1];
            y2 = mMousePoints_[1];
        }

        Vertex v1(x1, y1);
        Vertex v2(x2, y2);
        Vector v(v1, v2);
        Vector n(-v.y(), v.x());
        n.normalized();
        n = n * (v.get_magnitude() / 2.0f);

        mMousePoints_[i] = (v1.x + v2.x) / 2.0f + n.x();
        mMousePoints_[i + 1] = (v1.y + v2.y) / 2.0f + n.y();
    }

    mVertexSize_ = mMousePoints_.size() / 2;

    update_edges();
}

void Polygon::subdivise()
{
    const int size = mMousePoints_.size();
    mMousePoints_.resize(2 * size, 0.f);
    const int new_size = mMousePoints_.size();

    // - make space for new values
    for (int i = new_size - 3; i > 1; i -= 4)
    {
        mMousePoints_[i] = mMousePoints_[i / 2 + 1];
        mMousePoints_[i - 1] = mMousePoints_[i / 2];
    }

    // - double add barycenter of current edges to the list of vertex
    for (int i = 2; i < new_size - 1; i += 4)
    {
        float x1, y1, x2, y2;
        if (i != new_size - 2)
        {
            x1 = mMousePoints_[i - 2];
            x2 = mMousePoints_[i + 2];
            y1 = mMousePoints_[i - 1];
            y2 = mMousePoints_[i + 3];
        }

        else
        {
            x1 = mMousePoints_[i - 2];
            x2 = mMousePoints_[0];
            y1 = mMousePoints_[i - 1];
            y2 = mMousePoints_[1];
        }

        mMousePoints_[i] = (x1 + x2) / 2.0f;
        mMousePoints_[i + 1] = (y1 + y2) / 2.0f;
    }

    std::vector<float> new_vertex(new_size);

    for (int i = 0; i < new_size - 2; i++)
        new_vertex[i] = (mMousePoints_[i] + mMousePoints_[i + 2]) / 2.0f;

    new_vertex[new_size - 2] = (mMousePoints_[new_size - 2] + mMousePoints_[0]) / 2.0f;
    new_vertex[new_size - 1] = (mMousePoints_[new_size - 1] + mMousePoints_[1]) / 2.0f;

    mVertexSize_ = mMousePoints_.size() / 2;
    mMousePoints_ = new_vertex;

    update_edges();
}

void Polygon::update_edges()
{
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
