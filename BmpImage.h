#ifndef BMP_IMAGE_H
#define BMP_IMAGE_H

#include "managing_structs.h"
#include <fstream>
#include <vector>

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

        virtual ~BmpImage() {}

        BmpHeader& get_file_header() const { return file_header; }

        BmpInfoHeader& get_info_header() const { return info_header; }

        std::vector<uint8_t>& get_data() const { return data; }

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
            file.write((char*)palette.colors.data(), palette.colors.size() * sizeof(Color));
        }

    };
}

#endif
