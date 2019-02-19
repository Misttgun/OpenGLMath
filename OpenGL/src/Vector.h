#pragma once

#include "Polygon.h"

class Vector
{
    public: 
        Vector(const Vertex &v1, const Vertex &v2);
        float dot(const Vector& v) const;
        void rotate_90();
        void normalized();
        float get_magnitude() const;
        
    private:
        float _x;
        float _y;
};