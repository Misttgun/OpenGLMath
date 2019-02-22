#pragma once

#include "Polygon.h"

class Vector
{
    public: 
        Vector(const Vertex& v);
        Vector(const float& v1, const float &v2);
        Vector(const Vertex &v1, const Vertex &v2);
        float dot(const Vector& v) const;
        void rotate_90();
        void normalized();
        float get_magnitude() const;
        Vector operator- (const Vector& v1) const;
        
    private:
        float _x;
        float _y;
};