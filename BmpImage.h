#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include "managing_structs.h"
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cmath>

namespace  bmp {
    class BmpImage {
    protected:
        BmpHeader& file_header;
        BmpInfoHeader& info_header;

        std::vector<uint8_t>& data;

        void check_type_of_file_header() const {
            if (file_header.file_type != 0x4d42) {
                throw std::runtime_error("Unrecognized type of file");
            }
        }

    public:
        BmpImage(
            BmpHeader& file_header,
            BmpInfoHeader& info_header,
            std::vector<uint8_t>& file_data
        ) :
            file_header(file_header), info_header(info_header), data(file_data) {}

        virtual ~BmpImage() = default;

        [[nodiscard]] BmpHeader& get_file_header() const { return file_header; }

        [[nodiscard]] BmpInfoHeader& get_info_header() const { return info_header; }

        [[nodiscard]] std::vector<uint8_t>& get_data() const { return data; }

        virtual void read_headers(std::ifstream& file) {
            file.read(reinterpret_cast<char *>(&file_header), sizeof(file_header));
            check_type_of_file_header();
            file.read(reinterpret_cast<char *>(&info_header), sizeof(info_header));
        }

        void read_data(std::ifstream& file) const {
            const int bits_per_row = info_header.width * info_header.bit_count;
            int row_stride = (bits_per_row + 7) / 8;
            row_stride = (row_stride + 3) & ~3; // Align to 4 bytes

            data.resize(row_stride * abs(info_header.height));  // Handle negative height

            // Read from bottom up if height is positive
            if (info_header.height > 0) {
                for (int y = info_header.height - 1; y >= 0; --y) {
                    file.read(reinterpret_cast<char*>(data.data() + row_stride * y), row_stride);
                }
            } else {
                // Top-down image (negative height)
                file.read(reinterpret_cast<char*>(data.data()), row_stride * abs(info_header.height));
            }
        }

        virtual void write_headers(std::ofstream& file) const {
            file.write(reinterpret_cast<const char *>(&file_header), sizeof(file_header));
            file.write(reinterpret_cast<const char *>(&info_header), sizeof(info_header));
        }

        void write_data(std::ofstream& file) const {
            int bits_per_row = info_header.width * info_header.bit_count;
            int row_stride = (bits_per_row + 7) / 8;
            row_stride = (row_stride + 3) & ~3;

            // Write from bottom up if height is positive
            if (info_header.height > 0) {
                for (int y = info_header.height - 1; y >= 0; --y) {
                    file.write(reinterpret_cast<const char*>(data.data() + row_stride * y), row_stride);
                }
            } else {
                // Top-down image (negative height)
                file.write(reinterpret_cast<const char*>(data.data()), row_stride * abs(info_header.height));
            }
        }

        [[nodiscard]] virtual std::unordered_map<uint8_t, int> get_color_histogram() const = 0;

        virtual void change_brightness(int brightness) = 0;

        virtual void transform_to_negative() = 0;
        virtual void transform_to_negative(int p) = 0;

        virtual void increase_contrast(uint8_t q1, uint8_t q2) = 0;
        virtual void decrease_contrast(uint8_t q1, uint8_t q2) = 0;

        virtual void gamma_correct(int gamma) = 0;

        virtual void create_blank() = 0;

        virtual void draw_pixel_black(uint32_t index) = 0;

        //[[nodiscard]] uint8_t get_byte_value(const uint index) const {
          //  return data[index];
        //}
    };

    class RgbBmpImage final : public BmpImage {
    public:
        RgbBmpImage(
            BmpHeader& file_header,
            BmpInfoHeader& info_header,
            std::vector<uint8_t>& file_data
        ): BmpImage(file_header, info_header, file_data) {}

        void read_headers(std::ifstream& file) override {
            BmpImage::read_headers(file);
        }

        void write_headers(std::ofstream &file) const override {
            BmpImage::write_headers(file);
        }

        [[nodiscard]] std::unordered_map<uint8_t, int> get_color_histogram() const override {
            std::unordered_map<uint8_t, int> histogram;

            for (auto byte : data) {
                histogram[byte]++;
            }

            return histogram;
        }

        void change_brightness(const int brightness) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                if (byte + brightness > 255 || byte + brightness < 0) {
                    new_data.push_back(byte);
                    continue;
                }

                new_data.push_back(byte + brightness);
            }

            data.swap(new_data);
        }

        void transform_to_negative() override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                const auto to_add = 255 - byte;
                new_data.push_back(to_add);
            }

            data.swap(new_data);
        }

        void transform_to_negative(const int p) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                if (byte < p) {
                    new_data.push_back(byte);
                    continue;
                }

                new_data.push_back(255 - byte);
            }
            data.swap(new_data);
        }

        void increase_contrast(const uint8_t q1, const uint8_t q2) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                const auto to_add = static_cast<uint8_t>((byte - q1) * 255 / (q2 - q1));
                new_data.push_back(to_add);
            }
            data.swap(new_data);
        }

        void decrease_contrast(const uint8_t q1, const uint8_t q2) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                const auto to_add = static_cast<uint8_t>(q1 + byte * (q2 - q1) / 255);
                new_data.push_back(to_add);
            }
            data.swap(new_data);
        }

        void gamma_correct(const int gamma) override {
            std::vector<uint8_t> new_data;

            for (const auto byte : data) {
                const auto to_add = static_cast<uint8_t>(255 * pow(byte / 255.0, gamma));
                new_data.push_back(to_add);
            }

            data.swap(new_data);
        }

        void create_blank() override {
            const int32_t width = info_header.width = 1080;
            const int32_t height = info_header.height = 720;
            const uint32_t bits = info_header.bit_count = 24;

            const uint32_t row_stride = ((width * bits + 31) / 32) * 4;
            info_header.size_image = row_stride * height;

            info_header.size = sizeof(BmpInfoHeader);

            data.resize(info_header.size_image);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    size_t index = y * row_stride + x * 3;
                    data[index + 0] = 255; // Blue
                    data[index + 1] = 255; // Green
                    data[index + 2] = 255; // Red
                }
            }

            file_header.file_size = sizeof(BmpHeader) + sizeof(BmpInfoHeader) + info_header.size_image;
            file_header.offset = sizeof(BmpHeader) + sizeof(BmpInfoHeader);
        }

        void draw_pixel_black(const uint32_t index) override {
            data[index] = 0;
            data[index + 1] = 0;
            data[index + 2] = 0;
        }
    };

    class ArgbBmpImage final : public BmpImage {
        BmpColorHeader& color_header;

        void check_color_header() const {
            const BmpColorHeader sample;

            const bool is_alpha_failed = sample.alpha_mask != color_header.alpha_mask;
            const bool is_red_failed = sample.red_mask != color_header.red_mask;
            const bool is_green_failed = sample.green_mask != color_header.green_mask;
            const bool is_blue_failed = sample.blue_mask != color_header.blue_mask;
            const bool is_color_mask_failed = is_alpha_failed || is_red_failed || is_green_failed ||
                is_blue_failed;

            if (is_color_mask_failed) {
                throw std::runtime_error("Wrong color mask format: expected BGRA");
            }

            const bool is_color_space_type_failed =
                sample.color_space_type != color_header.color_space_type;

            if (is_color_space_type_failed) {
                throw std::runtime_error("Wrong color space type format: expected sRGB");
            }
        }

    public:
        ArgbBmpImage(
            BmpHeader& file_header,
            BmpInfoHeader& info_header,
            std::vector<uint8_t>& file_data,
            BmpColorHeader& color_header
            )
        : BmpImage(file_header, info_header, file_data), color_header(color_header) {}

        void read_headers(std::ifstream& file) override {
            BmpImage::read_headers(file);
            file.read(reinterpret_cast<char *>(&color_header), sizeof(color_header));
            check_color_header();
        }

        void write_headers(std::ofstream &file) const override {
            BmpImage::write_headers(file);
            file.write(reinterpret_cast<const char *>(&color_header), sizeof(color_header));
        }

        [[nodiscard]] std::unordered_map<uint8_t, int> get_color_histogram() const override {return {};}
        void change_brightness(const int brightness) override {}
        void transform_to_negative() override {}
        void transform_to_negative(const int p) override {}
        void increase_contrast(const uint8_t q1, const uint8_t q2) override {}
        void decrease_contrast(const uint8_t q1, const uint8_t q2) override {}
        void gamma_correct(const int gamma) override {}

        void create_blank() override {}

        void draw_pixel_black(const uint32_t index) override {}
    };

    class IndexedBmpImage final : public BmpImage {
        Palette& palette;

    public:
        IndexedBmpImage(
            BmpHeader& file_header,
            BmpInfoHeader& info_header,
            std::vector<uint8_t>& file_data,
            Palette& palette
        ) : BmpImage(file_header, info_header, file_data), palette(palette) {}

        void read_headers(std::ifstream& file) override {
            BmpImage::read_headers(file);
            palette.set_bit_count(info_header.bit_count);
            file.read(reinterpret_cast<char *>(palette.colors.data()), palette.colors.size() * sizeof(Color));
        }

        void write_headers(std::ofstream &file) const override {
            BmpImage::write_headers(file);
            file.write(reinterpret_cast<char *>(palette.colors.data()), palette.colors.size() * sizeof(Color));
        }

        [[nodiscard]] std::unordered_map<uint8_t, int> get_color_histogram() const override {
            std::unordered_map<uint8_t, int> histogram;

            for (auto byte : data) {
                histogram[byte]++;
            }

            return histogram;
        }

        void change_brightness(const int brightness) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                if (byte + brightness > 255 || byte + brightness < 0) {
                    new_data.push_back(byte);
                    continue;
                }

                new_data.push_back(byte + brightness);
            }

            data.swap(new_data);
        }

        void transform_to_negative() override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                const auto to_add = 255 - byte;
                new_data.push_back(to_add);
            }

            data.swap(new_data);
        }

        void transform_to_negative(const int p) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                if (byte < p) {
                    new_data.push_back(byte);
                    continue;
                }

                new_data.push_back(255 - byte);
            }
            data.swap(new_data);
        }

        void increase_contrast(const uint8_t q1, const uint8_t q2) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                const auto to_add = static_cast<uint8_t>((byte - q1) * 255 / (q2 - q1));
                new_data.push_back(to_add);
            }
            data.swap(new_data);
        }

        void decrease_contrast(const uint8_t q1, const uint8_t q2) override {
            std::vector<uint8_t> new_data;
            for (const auto byte : data) {
                const auto to_add = static_cast<uint8_t>(q1 + byte * (q2 - q1) / 255);
                new_data.push_back(to_add);
            }
            data.swap(new_data);
        }

        void gamma_correct(const int gamma) override {
            std::vector<uint8_t> new_data;

            for (const auto byte : data) {
                const auto to_add = static_cast<uint8_t>(255 * pow(byte / 255.0, gamma));
                new_data.push_back(to_add);
            }

            data.swap(new_data);
        }

        void create_blank() override {}

        void draw_pixel_black(const uint32_t index) override {}
    };
}

#endif
