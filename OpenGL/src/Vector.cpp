#include <cmath>

#include "Vector.h"


Vector::Vector(const Vertex &v1, const Vertex &v2)
{
    _x = v2.x - v1.x;
    _y = v2.y - v1.y;
}

float Vector::dot(const Vector& v) const
{
    return _x * v._x + _y * v._y;
}

void Vector::rotate_90()
{
    std::swap(_x, _y);
    _x *= -1;
}


void Vector::normalized()
{
    const float magnitude = get_magnitude();

    _x /= magnitude;
    _y /= magnitude;
}

float Vector::get_magnitude() const
{
    return sqrt(_x*_x + _y * _y);
}