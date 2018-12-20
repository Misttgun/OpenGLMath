#pragma once

#include <memory>
#include <vector>

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

using EdgeTable = std::vector<Bucket*>;

class Polygon
{
public:
	Polygon(float r = 1.0f, float g = 1.0f, float b = 1.0f);
	~Polygon() = default;

	void addPoint(float x, float y);
	void onImGuiRender();
	void onRender(const glm::mat4& vp, Shader* shader);
    void onRenderFill(const glm::mat4& vp, Shader* shader);
	void onUpdate();
    void sutherlandOgdmann(const std::unique_ptr<Polygon>& polygon, const std::unique_ptr<Polygon>& window);
    void computeBoundingBox(std::unique_ptr<Polygon>& polygon);
    
private:
    void line_intersection (float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float& xi, float& yi);
    void clip(float x1, float y1, float x2, float y2);
    void fill_LCA();
    void fill_edge_table(EdgeTable& et);
    void sort_edge_table(EdgeTable& et) const;
    void sort_active_edge_table(EdgeTable& aet) const;
    void update_active_edge(EdgeTable& et, EdgeTable& aet, EdgeTable& garbage, const int y) const;
    void compute_line_coordinates(EdgeTable& aet, std::vector<float>& lines, const int y) const;
    void update_x_bucket(EdgeTable& aet) const;
    void clean_edge_table(EdgeTable& et) const;

    float minY_;
    float maxY_;

	std::unique_ptr<VertexArray> mVertexArray_;
	std::unique_ptr<VertexBuffer> mVertexBuffer_;

	std::vector<float> mMousePoints_;
    std::vector<std::unique_ptr<Edge>> mEdges_;
	int mVertexSize_;

	float mColor_[4];
	glm::vec3 mTranslation_;
};

