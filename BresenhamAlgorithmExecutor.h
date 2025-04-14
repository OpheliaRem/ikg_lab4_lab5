#ifndef BRESENHAM_ALGORITHM_EXECUTOR_H
#define BRESENHAM_ALGORITHM_EXECUTOR_H

#include "Drawer.h"
#include <cmath>
#include <vector>

namespace curve_algorithms {

class BresenhamAlgorithmExecutor {
protected:
    drawing::Drawer* drawer;
public:
    virtual ~BresenhamAlgorithmExecutor() = default;

    explicit BresenhamAlgorithmExecutor(drawing::Drawer* drawer) : drawer(drawer) {}

    virtual void execute() = 0;
};

class LineSegmentBresenhamAlgorithmExecutor final: public BresenhamAlgorithmExecutor {
    uint32_t i0;
    uint32_t i1;
    uint32_t j0;
    uint32_t j1;
    std::string filename;

    void draw_point(const uint32_t x, const uint32_t y) const {
        drawer->draw(drawing::Point(x, y));
    }
public:
    LineSegmentBresenhamAlgorithmExecutor(
        drawing::Drawer* drawer,
        const uint32_t i0,
        const uint32_t i1,
        const uint32_t j0,
        const uint32_t j1,
        std::string filename
    ) : BresenhamAlgorithmExecutor(drawer), i0(i0), i1(i1), j0(j0), j1(j1), filename(std::move(filename)) {}

    void execute() override {
        int c;
        int d_i = static_cast<int>(i1) - i0;
        int d_j = static_cast<int>(j1) - j0;

        const int8_t sign_1 = sign(d_i);
        const int8_t sign_2 = sign(d_j);

        d_i = abs(d_i);
        d_j = abs(d_j);

        if (d_j > d_i) {
            std::swap(d_i, d_j);
            c = 1;
        } else {
            c = 0;
        }

        int e  = 2 * d_j - d_i;

        uint32_t i = i0;
        uint32_t j = j0;
        for (int k = 0; k < d_i; ++k) {
            draw_point(i ,j);
            while (e >= 0) {
                if (c == 1) {
                    i += sign_1;
                } else {
                    j += sign_2;
                }
                e -= 2 * d_i;
            }
            if (c == 1) {
                j += sign_2;
            } else {
                i += sign_1;
            }
            e += 2 * d_j;
        }
        drawer->save(filename);
    }

    static int8_t sign(const int32_t x) {
        return (x > 0) - (x < 0);
    }
};

}

#endif
