#ifndef BMP_CLASS
#define BMP_CLASS

#include "managing_structs.h"
#include "BmpImage.h"
#include "BmpConverter.h"
#include "ImageType.h"
#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>

namespace bmp {

    class BmpHandler {

        BmpHeader file_header;
        BmpInfoHeader info_header;
        BmpColorHeader color_header;
        Palette palette;

        std::vector<uint8_t> data;

        BmpImage* bmp_image;

        BmpConverter* bmp_converter;

        BmpImage* define_image_type(const std::string& filename) {
            std::ifstream file(filename, std::ios::binary);

            if (!file) {
                throw std::runtime_error("Could not open file " + filename);
            }

            read_headers(file);
            file.close();

            switch (info_header.bit_count) {
                case 1:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case 2:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case 4:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case 8:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case 16:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case 24:
                    return new RgbBmpImage(file_header, info_header, data);
                case 32:
                    return new ArgbBmpImage(file_header, info_header, data, color_header);
                default:
                    throw std::runtime_error("Unrecognized bit_count");
            }
        }

        BmpImage* define_image_type(const ImageType type) {
            switch (type) {
                case INDEXED_1BIT:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case INDEXED_2BIT:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case INDEXED_4BIT:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case INDEXED_8BIT:
                    return new IndexedBmpImage(file_header, info_header, data, palette);
                case RGB:
                    return new RgbBmpImage(file_header, info_header, data);
                case RGBA:
                    return new ArgbBmpImage(file_header, info_header, data, color_header);
                default:
                    throw std::runtime_error("Unrecognized type of image");
            }
        }

        void read(const std::string& filename) {

            std::ifstream file{filename, std::ios::binary};

            if (!file) {
                throw std::runtime_error("Could not open file " + filename);
            }

            bmp_image->read_headers(file);

            file.seekg(file_header.offset, std::ifstream::beg);

            bmp_image->read_data(file);
        }

        void read_headers(std::ifstream& file) {
            file.read(reinterpret_cast<char *>(&file_header), sizeof(file_header));
            file.read(reinterpret_cast<char *>(&info_header), sizeof(info_header));
        }

    public:

        explicit BmpHandler(const std::string& filename) : bmp_image(nullptr), bmp_converter(nullptr) {
            bmp_image = define_image_type(filename);
            read(filename);
        }

        explicit BmpHandler(const ImageType type) : bmp_converter(nullptr) {
            bmp_image = define_image_type(type);
            bmp_image->create_blank();
        }

        ~BmpHandler() {
            delete bmp_image;
        }

        BmpHandler(const BmpHandler&) = delete;
        BmpHandler& operator=(const BmpHandler&) = delete;

        void write(const std::string& filename) const {
            std::ofstream file {filename, std::ios::binary};

            if (!file) {
                throw std::runtime_error("Unable to open the file");
            }

            bmp_image->write_headers(file);

            file.seekp(file_header.offset, std::ofstream::beg);

            bmp_image->write_data(file);
        }

        void change_pattern(const std::vector<uint8_t>& bytes, const int index) {
            for (int i = index; i < index + bytes.size() && i < data.size(); ++i) {
                data[i] = bytes[i - index];
            }
        }

        void draw_pixel_black(const uint32_t index) const {
            return bmp_image->draw_pixel_black(index);
        }

        [[nodiscard]] uint64_t get_number_of_pixels() const {
            return info_header.width * info_header.height;
        }

        [[nodiscard]] int get_image_size_in_bytes() const {
            return info_header.width * info_header.height * info_header.bit_count / 8;
        }

        [[nodiscard]] int32_t get_image_width() const {
            return info_header.width;
        }

        [[nodiscard]] int32_t get_image_height() const {
            return info_header.height;
        }

        void make_noise(const int percent_of_picture_to_change) {
            const float part_of_picture_to_change = static_cast<float>(percent_of_picture_to_change) / 100.0f;
            const int size = static_cast<int>(part_of_picture_to_change * static_cast<float>(get_image_size_in_bytes()));
            std::vector<uint8_t> bytes;
            bytes.resize(size);

            for (int i = 0; i < size; ++i) {
                bytes[i] = rand() % 255;
            }

            change_pattern(bytes, 0);
        }

        void to_8bit(Palette palette) {
            if (const auto rgb_image{dynamic_cast<RgbBmpImage*>(bmp_image)}; !rgb_image) {
                throw std::runtime_error("Could not create RGB image");
            }

            this->palette = std::move(palette);
            bmp_converter = new BmpConverterRgbToIndexed8Bit(
                bmp_image,
                file_header,
                info_header,
                data,
                this->palette
            );

            bmp_converter->convert();

            delete bmp_converter;
            bmp_converter = nullptr;
        }

        void to_monochrome(const int p = 127) {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};

            if (!indexed_image) {
                Palette palette;
                palette.make_grayscale();
                to_8bit(palette);
            }

            bmp_converter = new BmpConverterIndexed8BitToMonochrome(
                bmp_image,
                file_header,
                info_header,
                data,
                this->palette,
                p
            );

            bmp_converter->convert();

            delete bmp_converter;
            bmp_converter = nullptr;
        }

        [[nodiscard]] std::unordered_map<uint8_t, int> get_color_histogram() const {
            return bmp_image->get_color_histogram();
        }

        void change_brightness(const int brightness) const {
            return bmp_image->change_brightness(brightness);
        }

        void negative_transform() const {
            return bmp_image->transform_to_negative();
        }

        void negative_transform(const int p) const {
            return bmp_image->transform_to_negative(p);
        }

        void increase_contrast(const uint8_t q1, const uint8_t q2) const {
            return bmp_image->increase_contrast(q1, q2);
        }

        void decrease_contrast(const uint8_t q1, const uint8_t q2) const {
            return bmp_image->decrease_contrast(q1, q2);
        }

        void gamma_correct(const int gamma) const {
            return bmp_image->gamma_correct(gamma);
        }
    };
}

#endif