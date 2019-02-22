#include "Utils.h"

Utils* Utils::_instance = nullptr;

Utils* Utils::get()
{
    if (_instance == nullptr)
        _instance = new Utils();

    return _instance;
}

bool Utils::in_triangle(const Vector& a, const Vector &b, const Vector& c, const Vector &p) const
{
    Vector v0 = c - a;
    Vector v1 = b - a;
    Vector v2 = p - a;

    float v00 = v0.dot(v0);
    float v01 = v0.dot(v1);
    float v02 = v0.dot(v2);
    float v11 = v1.dot(v1);
    float v12 = v1.dot(v2);

    float inv_denom = 1.0f / (v00 * v11 - v01 * v01);
    float u = (v11 * v02 - v01 * v12) * inv_denom;
    float v = (v00 * v12 - v01 * v02) * inv_denom;

    return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}