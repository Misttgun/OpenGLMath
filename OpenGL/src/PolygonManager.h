#pragma once

#include <memory>
#include <vector>

#include "Polygon.h"

class PolygonManager
{
    public:
        ~PolygonManager() { delete _instance; };

        static PolygonManager* get();

        void add_polygon();
        void add_window();
        std::shared_ptr<Polygon> get_current_polygon();
        std::shared_ptr<Polygon> get_current_result();
        std::shared_ptr<Polygon> get_current_window();
        void on_render(const glm::mat4& vp, Shader* shader);
        void on_render_fill(const glm::mat4& vp, Shader* shader);
        void compute_bounding_box();
        void sutherland_ogdmann();
        void delete_current_polygon();
        void delete_current_window();
    
    private:
        PolygonManager() = default;

        static PolygonManager* _instance;

        int _current_polygon_index = -1;
        int _current_window_index = -1;

        std::vector<std::shared_ptr<Polygon>> _polygons;
        std::vector<std::shared_ptr<Polygon>> _windows;
        std::vector<std::shared_ptr<Polygon>> _bounding_boxes;
        std::vector<std::shared_ptr<Polygon>> _results;
};