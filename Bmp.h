#ifndef BMP_CLASS
#define BMP_CLASS

#include "managing_structs.h"
#include "BmpImage.h"
#include "BmpConverter.h"
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

        void read(const std::string& filename) {

            std::ifstream file{filename, std::ios::binary};

            if (!file) {
                throw std::runtime_error("Could not open file " + filename);
            }

            bmp_image = define_image_type(filename);

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
            read(filename);
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

        void change_pattern(const u_int8_t* bytes, const int index, const size_t quantity) {
            for (int i = index; i < index + quantity; ++i) {
                data[i] = bytes[i - index];
            }
        }

        uint64_t get_number_of_pixels() const {
            return info_header.width * info_header.height;
        }

        int get_image_size_in_bytes() const {
            return info_header.width * info_header.height * info_header.bit_count / 8;
        }

        void make_noise(const int percent_of_picture_to_change) {
            const float part_of_picture_to_change = static_cast<float>(percent_of_picture_to_change) / 100.0f;
            const int size = static_cast<int>(part_of_picture_to_change * static_cast<float>(get_image_size_in_bytes()));
            const auto bytes = new uint8_t[size];

            for (int i = 0; i < size; ++i) {
                bytes[i] = rand() % 255;
            }

            change_pattern(bytes, 0, size);
            delete[] bytes;
        }

        void rgb_to_indexed_image_8bit(Palette palette) {
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

        std::unordered_map<uint8_t, int> get_color_histogram() const {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};

            if (!indexed_image) {
                throw std::runtime_error("Could not create indexed image");
            }

            std::unordered_map<uint8_t, int> histogram;

            for (auto pixel : data) {
                histogram[pixel]++;
            }

            return histogram;
        }

        void change_brightness(const int brightness) {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};

            if (!indexed_image) {
                throw std::runtime_error("Could not create indexed image");
            }

            std::vector<uint8_t> new_data;
            for (const auto pixel : data) {
                if (pixel + brightness > 255 || pixel + brightness < 0) {
                    new_data.push_back(pixel);
                    continue;
                }

                new_data.push_back(pixel + brightness);
            }

            data.swap(new_data);
        }

        void negative_transform() {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};

            if (!indexed_image) {
                throw std::runtime_error("Could not create indexed image");
            }

            std::vector<uint8_t> new_data;
            for (const auto pixel : data) {
                new_data.push_back(255 - pixel);
            }

            data.swap(new_data);
        }

        void negative_transform(const int p) {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};
            if (!indexed_image) {
                throw std::runtime_error("Could not create indexed image");
            }

            std::vector<uint8_t> new_data;
            for (const auto pixel : data) {
                if (pixel < p) {
                    new_data.push_back(pixel);
                    continue;
                }

                new_data.push_back(255 - pixel);
            }
            data.swap(new_data);
        }

        void indexed_8bit_to_monochrome(const int p = 127) {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};

            if (!indexed_image || info_header.bit_count != 8) {
                throw std::runtime_error("Could not create indexed image");
            }

            bmp_converter = new BmpConverterIndexed8BitToMonochrome(
                bmp_image,
                data,
                p
            );

            bmp_converter->convert();

            delete bmp_converter;
            bmp_converter = nullptr;
        }

        void increase_contrast(const uint8_t q1, const uint8_t q2) {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};
            if (!indexed_image) {
                throw std::runtime_error("Could not create indexed image");
            }

            std::vector<uint8_t> new_data;
            for (const auto pixel : data) {
                const auto to_add = (pixel - q1) * 255 / (q2 - q1);
                new_data.push_back(to_add);
            }
            data.swap(new_data);
        }

        void decrease_contrast(const uint8_t q1, const uint8_t q2) {
            const auto indexed_image{dynamic_cast<IndexedBmpImage*>(bmp_image)};
            if (!indexed_image) {
                throw std::runtime_error("Could not create indexed image");
            }

            std::vector<uint8_t> new_data;
            for (const auto pixel : data) {
                const auto to_add = q1 + pixel * (q2 - q1) / 255;
                new_data.push_back(to_add);
            }
            data.swap(new_data);
        }
    };
}

#endif