#ifndef DRAWER_H
#define DRAWER_H

#include <vector>
#include "Bmp.h"
#include "ImageType.h"
#include "Point.h"

namespace drawing {

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

    explicit BmpDrawer(bmp::BmpHandler* handler) : handler(handler) {}

    explicit BmpDrawer(const std::string& filename) {
        handler = new bmp::BmpHandler(filename);
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

    [[nodiscard]] const bmp::BmpHandler& get_handler() const { return *handler; }
};

}

#endif
