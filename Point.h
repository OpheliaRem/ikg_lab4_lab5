#ifndef POINT_H
#define POINT_H

namespace drawing {
    struct Point {
        uint32_t x;
        uint32_t y;

        Point() : x(0), y(0) {}

        Point(const uint32_t x, const uint32_t y) : x(x), y(y) {}

        bool operator<(const Point &other) const {
            return x * x + y * y <= other.x * other.x + other.y * other.y;
        }
    };
};

#endif
