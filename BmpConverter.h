#ifndef BMP_CONVERTER_H
#define BMP_CONVERTER_H

#include "BmpImage.h"
#include "managing_structs.h"

namespace bmp {
    class BmpConverter {
    protected:
        BmpImage*& bmp_image;

        virtual uint32_t calculate_offset() const = 0;
        virtual void change_headers() const = 0;
    public:
        explicit BmpConverter(BmpImage*& bmp_image) : bmp_image(bmp_image) {}

        virtual ~BmpConverter() = default;

        virtual void convert() = 0;
    };

    class BmpConverterRgbToIndexed8Bit final : public BmpConverter {
        BmpHeader& file_header;
        BmpInfoHeader& info_header;
        std::vector<uint8_t>& data;
        Palette& palette;

        uint32_t calculate_offset() const override {
            return sizeof(BmpHeader) + sizeof(BmpInfoHeader) + palette.colors.size() * sizeof(Color);
        }

        void change_headers() const override {
            const int width = info_header.width;
            const int height = std::abs(info_header.height);
            const int row_stride = (width + 3) & ~3;

            info_header.size_image = row_stride * height;

            info_header.bit_count = 8;
            file_header.offset = calculate_offset();
        }
    public:

        explicit BmpConverterRgbToIndexed8Bit(
            BmpImage*& image,
            BmpHeader& file_header,
            BmpInfoHeader& info_header,
            std::vector<uint8_t>& data,
            Palette& palette) : BmpConverter(image),
        file_header(file_header), info_header(info_header), data(data), palette(palette) {}

        void convert() override {
            if (bmp_image == nullptr) {
                throw std::invalid_argument("BmpConverterRgbToIndexed8bit: image is null");
            }

            delete bmp_image;
            bmp_image = new IndexedBmpImage(file_header, info_header, data , palette);

            std::vector<uint8_t> new_data;

            for (int i = 0; i + 2 < data.size(); i += 3) {
                const uint8_t b = data[i];
                const uint8_t g = data[i + 1];
                const uint8_t r = data[i + 2];

                auto new_color = static_cast<uint8_t>(0.3 * r + 0.59 * g + 0.11 * b);

                new_data.push_back(new_color);
            }

            data.swap(new_data);

            change_headers();
        }
    };

    class BmpConverterIndexed8BitToMonochrome final : public BmpConverter {
        BmpHeader& file_header;
        BmpInfoHeader& info_header;
        std::vector<uint8_t>& data;
        Palette& palette;
        const int p;

        void change_headers() const override {
            const int bits_per_row = info_header.width;
            const int row_stride = (bits_per_row + 7) / 8 + 3 & ~3;

            info_header.size_image = row_stride * info_header.height;

            info_header.bit_count = 1;
            file_header.offset = calculate_offset();
        }

        uint32_t calculate_offset() const override {
            return sizeof(BmpHeader) + sizeof(BmpInfoHeader) + palette.colors.size() * sizeof(Color);
        }

        void change_palette() const {
            palette.colors.clear();

            palette.colors.push_back(
                {static_cast<uint8_t>(0), static_cast<uint8_t>(0),
                    static_cast<uint8_t>(0), static_cast<uint8_t>(0)}
            );
            palette.colors.push_back(
                {static_cast<uint8_t>(255), static_cast<uint8_t>(255),
                    static_cast<uint8_t>(255), static_cast<uint8_t>(0)}
            );
        }
    public:

        explicit BmpConverterIndexed8BitToMonochrome(
            BmpImage*& bmp_image,
            BmpHeader& file_header,
            BmpInfoHeader& info_header,
            std::vector<uint8_t>& data,
            Palette& palette,
            const int p = 127
        ) :
        BmpConverter(bmp_image),
        file_header(file_header),
        info_header(info_header),
        data(data),
        palette(palette),
        p(p) {}

        void convert() override {
            change_palette();
            change_headers();

            const int width = info_header.width;
            const int height = std::abs(info_header.height);
            const int bits_per_row = width;
            const int row_stride = ((bits_per_row + 31) / 32) * 4; // выравнивание до ближайших 4 байт
            std::vector<uint8_t> new_data(height * row_stride, 0);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; x += 8) {
                    uint8_t byte = 0;
                    for (int bit = 0; bit < 8 && x + bit < width; ++bit) {
                        int pixel_index = y * width + x + bit;
                        if (data[pixel_index] >= p) {
                            byte |= 1 << (7 - bit);
                        }
                    }

                    int byte_index_in_row = x / 8;
                    new_data[y * row_stride + byte_index_in_row] = byte;
                }
            }

            data.swap(new_data);
        }

    };
}


#endif
