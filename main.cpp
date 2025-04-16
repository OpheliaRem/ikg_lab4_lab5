#include "Bmp.h"
#include "BmpImage.h"
#include "RasterDrawer.h"
#include "managing_structs.h"
#include "MapToCsvFileHandler.h"
#include <filesystem>
#include <vector>
#include <functional>

std::string expand_home_directory(const std::string& path) {
    if (path[0] == '~') {
        if (const char* home = std::getenv("HOME")) {
            return std::string(home) + path.substr(1);
        }
    }
    return path;
}

void lab4() {
    const std::filesystem::path input_path{expand_home_directory("~/me/labs/ikg/lab4/input_data/")};

    int i = 0;
    for (const auto& entry : std::filesystem::directory_iterator(input_path)) {
        const auto handler = new bmp::BmpHandler(entry.path());

        handler->make_noise(50);

        handler->write(
    expand_home_directory(
    "~/me/labs/ikg/lab4/output_data/lab4_file"
            ) + std::to_string(i) + ".bmp"
        );

        ++i;

        delete handler;
    }
}

void lab5() {
    bmp::BmpHandler first_handler(expand_home_directory("~/me/labs/ikg/lab4/input_data/sample0_RGB.bmp"));

    Palette palette;
    palette.make_grayscale();

    first_handler.to_8bit(palette);

    first_handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
    const auto source = expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp");
    //const auto source = expand_home_directory("~/me/labs/ikg/lab4/input_data/sample0_RGB.bmp");

    std::vector<std::function<void()>> actions;
    actions.emplace_back([source] {
        const bmp::BmpHandler handler(source);
        const auto map = handler.get_color_histogram();
        MapToCsvFileHandler::to_csv(
        expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_histogram.csv"),
            map,
            ';'
        );
    });

    actions.emplace_back([source] {
        const bmp::BmpHandler handler(source);
        handler.change_brightness(-25);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_bright.bmp"));
    });

    actions.emplace_back([source] {
        const bmp::BmpHandler handler(source);
        handler.negative_transform(32);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_negative.bmp"));
    });

    actions.emplace_back([source] {
        const bmp::BmpHandler handler(source);
        handler.increase_contrast(0, 100);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_inc_contr.bmp"));
    });

    actions.emplace_back([source] {
        const bmp::BmpHandler handler(source);
        handler.decrease_contrast(32, 128);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_dec_contr.bmp"));
    });

    actions.emplace_back([source] {
        bmp::BmpHandler handler(source);
        handler.to_monochrome(190);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_monochrome.bmp"));
    });

    actions.emplace_back([source] {
        const bmp::BmpHandler handler(source);
        handler.gamma_correct(6);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_gamma.bmp"));
    });

    for (const auto& action : actions) {
        action();
    }
}

void lab7() {

    RasterDrawer::draw_line(
        expand_home_directory("~/me/labs/ikg/lab4/output_data/lab7_file_line_segment.bmp"),
        1 << 9, 1 << 9,
        1 << 5, 1 << 5
    );

    RasterDrawer::draw_circle(
        expand_home_directory("~/me/labs/ikg/lab4/output_data/lab7_file_circle.bmp"),
        720 / 2,
        1080 / 2,
        720 / 2 - 20
    );

    RasterDrawer::fill(expand_home_directory(
        "~/me/labs/ikg/lab4/output_data/lab7_file_circle.bmp"),
        720 / 2,
        1080 / 2
    );
}

int main() {
    lab4();
    lab5();
    lab7();

    return 0;
}