#ifndef FILLING_ALGORITHM_EXECUTOR_H
#define FILLING_ALGORITHM_EXECUTOR_H

#include <string>
#include <stack>
#include <functional>
#include <set>
#include "Drawer.h"
#include <vector>

class FillingAlgorithmExecutor {
    std::string canvas_file;
    drawing::Point point{drawing::Point(0, 0)};
    drawing::Drawer* drawer;

public:

    FillingAlgorithmExecutor(std::string canvas_file, const uint x, const uint y) :
    canvas_file(std::move(canvas_file)) {
        drawer = new drawing::BmpDrawer(this->canvas_file);
        point.x = x;
        point.y = y;
    }

    ~FillingAlgorithmExecutor() {
        delete drawer;
    }

    void execute() const {
        std::stack<drawing::Point> stack;
        std::set<drawing::Point> initialized;

        stack.push(point);

        while (!stack.empty()) {
            auto current = stack.top();
            stack.pop();

            drawer->draw(current);

            initialized.insert(current);

            std::function border_condition = [this] (const drawing::Point& point) -> bool {
                const auto bmp_drawer = dynamic_cast<drawing::BmpDrawer*>(drawer);
                if (!bmp_drawer) {
                    throw std::runtime_error("Unknown way to define borders: unknown file type");
                }

                const bmp::BmpHandler& handler = bmp_drawer->get_handler();
                const auto colors = handler.get_color_value(point);

                if (colors.size() != 3) {
                    throw std::runtime_error("Non-RGB image: impossible to parse");
                }

                return colors[0] == 0 && colors[1] == 0 && colors[2] == 0;
            };

            std::vector<drawing::Point> neighbours;
            neighbours.emplace_back(current.x + 1, current.y);
            neighbours.emplace_back(current.x, current.y + 1);
            neighbours.emplace_back(current.x - 1, current.y);
            neighbours.emplace_back(current.x, current.y - 1);

            for (auto neighbour : neighbours) {
                if (!initialized.contains(neighbour) && !border_condition(neighbour)) {
                    stack.push(neighbour);
                }
            }
        }

        drawer->save(canvas_file.substr(0, canvas_file.length() - 4) + "_filled.bmp");
    }
};



#endif
