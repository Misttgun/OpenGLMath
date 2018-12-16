#include <utility>

#include "Edge.h"

Edge::Edge(float x1, float y1, float x2, float y2)
{
    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;

    // - vertices must be ordered left to right
    if (x1 > x2)
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
}