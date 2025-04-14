#ifndef DRAWER_H
#define DRAWER_H

#include <vector>
#include "Bmp.h"
#include "ImageType.h"

namespace drawing {

struct Point {
    uint32_t x;
    uint32_t y;

    Point(const uint32_t x, const uint32_t y) : x(x), y(y) {}
};

class Drawer {
public:
    virtual ~Drawer() = default;
    virtual void draw(std::vector<Point> points) = 0;
    virtual void draw(const Point& point) = 0;
    virtual void save(const std::string& filename) const = 0;
};

class BmpDrawer final : public Drawer {
    bmp::BmpHandler* handler;

public:
    BmpDrawer() {
        handler = new bmp::BmpHandler(RGB);
    }

    ~BmpDrawer() override {
        delete handler;
    }

    void draw(std::vector<Point> points) override {}

    void draw(const Point& point) override {
        handler->draw_pixel_black(point.x * 3 * static_cast<uint32_t>(handler->get_image_width()) + point.y * 3);
    }

    void save(const std::string& filename) const override {
        handler->write(filename);
    }
};

}

#endif
