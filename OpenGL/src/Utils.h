#pragma once

#include "Vector.h"

class Utils
{
    public:
        static Utils* get();
        bool in_triangle(const Vector& a, const Vector &b, const Vector& c, const Vector &p) const;

    private:
        Utils() = default;
        static Utils* _instance;
};