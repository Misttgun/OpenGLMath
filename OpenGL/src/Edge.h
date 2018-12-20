#pragma once
#include <iostream>
class Edge
{
    public:
        Edge() = default;
        Edge(float x1, float y1, float x2, float y2);
        ~Edge() = default;
        inline float minY() const { return y1_ < y2_ ? y1_ : y2_; }
        inline float maxY() const { return y1_ > y2_ ? y1_ : y2_; }
        inline float minX() const { return x1_ < x2_ ? x1_ : x2_; }
        inline float maxX() const { return x1_ > x2_ ? x1_ : x2_; }
        inline float getInvDir() const { return invDir_; }

        float x1_;
        float y1_;
        float x2_;
        float y2_;
        float invDir_;
};

