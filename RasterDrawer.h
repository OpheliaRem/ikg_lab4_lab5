#ifndef RASTER_DRAWER_H
#define RASTER_DRAWER_H

#include "BresenhamAlgorithmExecutor.h"
#include "FillingAlgorithmExecutor.h"


class RasterDrawer {
public:
    static void draw_line(const std::string& filename, const uint i0, const uint j0, const uint i1, const uint j1) {
        drawing::Drawer* drawer = new drawing::BmpDrawer();
        curve_algorithms::BresenhamAlgorithmExecutor* executor =
            new curve_algorithms::LineSegmentBresenhamAlgorithmExecutor{
                drawer,
                i0,
                j0,
                i1,
                j1,
                filename,
        };

        executor->execute();

        delete drawer;
        delete executor;
    }

    static void draw_circle(const std::string& filename, const uint cx, const uint cy, const uint r) {
        drawing::Drawer* drawer = new drawing::BmpDrawer();
        curve_algorithms::BresenhamAlgorithmExecutor* executor =
            new curve_algorithms::CircleBresenhamAlgorithmExecutor{
            drawer,
            filename,
            cx,
            cy,
            r
        };

        executor->execute();

        delete drawer;
        delete executor;
    }

    static void fill(const std::string& filename, const uint x, const uint y) {
        const FillingAlgorithmExecutor executor(
            filename,
            x,
            y
        );

        executor.execute();
    }
};



#endif
