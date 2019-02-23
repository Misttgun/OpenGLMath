#include "PolygonManager.h"

PolygonManager* PolygonManager::_instance = nullptr;

PolygonManager* PolygonManager::get()
{
    if (!_instance)
        _instance = new PolygonManager();

    return _instance;
}

void PolygonManager::add_polygon()
{
    _polygons.push_back(std::make_shared<Polygon>(Polygon(1.0f, 0.0f, 0.0f)));
    _results.push_back(std::make_shared<Polygon>(Polygon(0.0f, 1.0f, 0.0f)));
    _bounding_boxes.push_back(std::make_shared<Polygon>(Polygon(1.0f, 1.0f, 0.0f)));
    _current_polygon_index++;
}

void PolygonManager::add_window()
{
    _windows.push_back(std::make_shared<Polygon>(Polygon(0.0f, 0.0f, 1.0f)));
    _current_window_index++;
}

void PolygonManager::on_im_gui_render_polygons()
{
    for (const auto& polygon : _polygons)
        polygon->onImGuiRender();
}

void PolygonManager::on_im_gui_render_windows()
{
    for (const auto& window : _windows)
        window->onImGuiRender();
}

std::shared_ptr<Polygon> PolygonManager::get_current_polygon()
{
    if (_current_polygon_index == -1)
        return nullptr;

    return _polygons.at(_current_polygon_index);
}

std::shared_ptr<Polygon> PolygonManager::get_current_result()
{
    if (_current_polygon_index == -1)
        return nullptr;

    return _results.at(_current_polygon_index);
}

std::shared_ptr<Polygon> PolygonManager::get_current_window()
{
    if (_current_window_index == -1)
        return nullptr;

    return _windows.at(_current_window_index);
}

std::vector<std::shared_ptr<Polygon>>& PolygonManager::get_triangles()
{
    return _windows_triangles;
}

void PolygonManager::on_render(const glm::mat4& vp, Shader* shader)
{
    for (const auto& polygon : _polygons)
        polygon->onRender(vp, shader);


    for (const auto& window : _windows)
        window->onRender(vp, shader);

    /*
    for (const auto& triangle : _windows_triangles)
        triangle->onRender(vp, shader);
    
    for (const auto& bounding_box : _bounding_boxes)
        bounding_box->onRender(vp, shader);
    */

    for (const auto& result : _results)
            result->onRender(vp, shader);    
}

void PolygonManager::sutherland_ogdmann()
{
    if (_current_window_index == -1)
        return;

    int i = 0;

    _results.resize(_windows_triangles.size() * _polygons.size());

    for (const auto& polygon : _polygons)
    {
        for (const auto& triangle : _windows_triangles)
        {
            if (_results[i] == nullptr)
                _results[i] = std::make_shared<Polygon>(Polygon(0.0f, 1.0f, 0.0f));
            _results[i]->sutherlandOgdmann(polygon, triangle);
            i++;
        }
    }
}

void PolygonManager::compute_bounding_box()
{
    int i = 0;
    const int size = _windows_triangles.size() * _polygons.size();

    _bounding_boxes.resize(size);

    if (size == 0)
        return;

    for (const auto& result : _results)
    {
        if (_bounding_boxes[i] == nullptr)
            _bounding_boxes[i] = std::make_shared<Polygon>(Polygon(1.0f, 1.0f, 0.0f));
        result->computeBoundingBox(_bounding_boxes[i]);
        i++;
    }
}


void PolygonManager::on_render_fill(const glm::mat4& vp, Shader* shader)
{
    for (const auto& result : _results)
        result->onRenderFill(vp, shader);
}


void PolygonManager::delete_current_polygon()
{
    if (_current_polygon_index == -1)
        return;

    _polygons.pop_back();
    _current_polygon_index--;
}

void PolygonManager::delete_current_window()
{
    if (_current_window_index == -1)
        return;

    _windows.pop_back();
    _current_window_index--;
}
