#ifndef MANAGING_STRUCTS
#define MANAGING_STRUCTS

#include <cstdint>
#include <vector>

typedef uint8_t byte;

#pragma pack(push, 1)

struct BmpHeader {
    uint16_t file_type {0x4d42};
    uint32_t file_size {0};
    uint16_t reserve1 {0};
    uint16_t reserve2 {0};
    uint32_t offset {0};
};

struct BmpInfoHeader {
    uint32_t size {0};
    int32_t width {0};
    int32_t height {0};

    uint16_t planes {1};
    uint16_t bit_count {0};
    uint32_t compression {0};
    uint32_t size_image {0};
    int32_t x_pixels_per_meter {0};
    int32_t y_pixels_per_meter {0};
    uint32_t colors_used {0};
    uint32_t colors_important {0};
};

struct BmpColorHeader {
    uint32_t red_mask {0x00ff0000};
    uint32_t green_mask {0x0000ff00};
    uint32_t blue_mask {0x000000ff};
    uint32_t alpha_mask {0xff000000};
    uint32_t color_space_type {0x73524742};
    uint32_t unused[16] {};
};

struct Color {
    byte blue;
    byte green;
    byte red;
    byte reserved;
};

struct Palette {

    std::vector<Color> colors;

    void set_bit_count(const uint16_t bit_count) {
        colors.resize(1 << bit_count);
    }

    void make_grayscale() {
        colors.clear();
        for (int i = 0; i < 256; ++i) {
            colors.push_back({ static_cast<byte>(i), static_cast<byte>(i), static_cast<byte>(i), 0 });
        }
    }
};


#pragma pack(pop)

#endif