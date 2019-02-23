#pragma once

#include <memory>
#include <vector>
#include <list>

#include "VertexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Edge.h"

struct Bucket
{
    float y_min;
    float y_max;
    float current_x;
    float inv_dir;
};

struct Vertex
{
    Vertex(const float x_, const float y_) { x = x_;  y = y_;  }
    float x;
    float y;
};

using EdgeTable = std::vector<Bucket*>;
using VertexPtr = std::shared_ptr<Vertex>;
using VertexList = std::list<VertexPtr>;
using VertexListIterator = VertexList::iterator;

class Polygon
{
public:
	Polygon(float r = 1.0f, float g = 1.0f, float b = 1.0f);
    Polygon(Polygon&& p);
    ~Polygon() = default;

	void addPoint(float x, float y);
	void onImGuiRender();
	void onRender(const glm::mat4& vp, Shader* shader);
    void onRenderFill(const glm::mat4& vp, Shader* shader);
	void onUpdate();
    void sutherlandOgdmann(const std::shared_ptr<Polygon>& polygon, const std::shared_ptr<Polygon>& window);
    void ear_clipping(std::vector<std::shared_ptr<Polygon>>& vector);
    void computeBoundingBox(std::shared_ptr<Polygon>& polygon);
    void subdivise();
    int size() const { return mVertexSize_; }
    
private:
    // - clipping
    void line_intersection (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float& xi, float& yi);
    void clip(float x1, float y1, float x2, float y2);
    // filling
    void fill_LCA();
    void fill_edge_table(EdgeTable& et);
    void sort_edge_table(EdgeTable& et) const;
    void sort_active_edge_table(EdgeTable& aet) const;
    void update_active_edge(EdgeTable& et, EdgeTable& aet, EdgeTable& garbage, const int y) const;
    void compute_line_coordinates(EdgeTable& aet, std::vector<float>& lines, const int y) const;
    void update_x_bucket(EdgeTable& aet) const;
    void clean_edge_table(EdgeTable& et) const;
    void update_edges();
    // ear clipping
    void create_vertex_list(VertexList &list);
    void init_ear_clipping(VertexList& vertex_list, VertexList& convex_list, VertexList& reflex_list, VertexList& ear_list);
    bool is_ear(VertexListIterator& it, VertexList& vertex_list, VertexList& reflex_list);
    bool is_in_list(VertexPtr& v, VertexList& vertex_list);
    VertexListIterator get_in_list(VertexPtr& v, VertexList& vertex_list);
    void update_vertex(VertexPtr& v, VertexList& vertex_list, VertexList& convex_list, VertexList& reflex_list, VertexList& ear_list);
    bool is_reflex(const VertexPtr& v, const VertexPtr& prev, const VertexPtr& next);
    void move_to_list(VertexPtr& v, VertexList& src, VertexList& dest);
    void erase_from_list(VertexPtr& v, VertexList& vertex_list);

    float minY_;
    float maxY_;

	std::unique_ptr<VertexArray> mVertexArray_;
	std::unique_ptr<VertexBuffer> mVertexBuffer_;
    std::vector<std::unique_ptr<Edge>> mEdges_;

	std::vector<float> mMousePoints_;
	int mVertexSize_;

	float mColor_[4];
	glm::vec3 mTranslation_;
};

