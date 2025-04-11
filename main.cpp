#include "Bmp.h"
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
    bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/input_data/sample0_RGB.bmp"));

    Palette palette;
    palette.make_grayscale();

    handler.rgb_to_indexed_image_8bit(palette);
    handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));

    std::vector<std::function<void()>> actions;
    actions.emplace_back([] {
        bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
        const auto map = handler.get_color_histogram();
        MapToCsvFileHandler::to_csv(
        expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_histogram.csv"),
            map,
            ','
        );
    });

    actions.emplace_back([] {
        bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
        handler.change_brightness(-25);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_bright.bmp"));
    });

    actions.emplace_back([] {
        bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
        handler.negative_transform();
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_negative.bmp"));
    });

    actions.emplace_back([] {
        bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
        handler.indexed_8bit_to_monochrome(200);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_monochrome.bmp"));
    });

    actions.emplace_back([] {
        bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
        handler.increase_contrast(13, 89);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_inc_contr.bmp"));
    });

    actions.emplace_back([] {
        bmp::BmpHandler handler(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file.bmp"));
        handler.decrease_contrast(13, 89);
        handler.write(expand_home_directory("~/me/labs/ikg/lab4/output_data/lab5_file_dec_contr.bmp"));
    });

    for (const auto& action : actions) {
        action();
    }
}

int main() {
    lab4();
    lab5();

    return 0;
}