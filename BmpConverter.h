#ifndef BMP_CONVERTER_H
#define BMP_CONVERTER_H

#include "BmpImage.h"
#include "managing_structs.h"

namespace bmp {
    class BmpConverter {
    protected:
        BmpImage*& bmp_image;
    public:
        explicit BmpConverter(BmpImage*& bmp_image) : bmp_image(bmp_image) {}

        virtual ~BmpConverter() {}

        virtual void convert() = 0;
    };

    class BmpConverterRgbToIndexed8Bit final : public BmpConverter {
        BmpHeader& file_header;
        BmpInfoHeader& info_header;
        std::vector<uint8_t>& data;
        Palette& palette;

        uint32_t calculate_offset() const {
            return sizeof(BmpHeader) + sizeof(BmpInfoHeader) + palette.colors.size();
        }

        void change_file_headers() const {
            info_header.bit_count = 8;
            info_header.size_image = data.size();
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
                throw std::invalid_argument("BmpConverterRgbToGrayscale: image is null");
            }

            delete bmp_image;
            bmp_image = new IndexedBmpImage(file_header, info_header, data , palette);

            std::vector<uint8_t> new_data;

            for (int i = 0; i + 2 < data.size(); i += 3) {
                const uint8_t r = data[i];
                const uint8_t g = data[i + 1];
                const uint8_t b = data[i + 2];

                auto new_color = static_cast<uint8_t>(0.3 * r + 0.59 * g + 0.11 * b);

                new_data.push_back(new_color);
            }

            data.swap(new_data);

            change_file_headers();
        }
    };

    class BmpConverterIndexed8BitToMonochrome final : public BmpConverter {
        std::vector<uint8_t>& data;
        const int p;
    public:

        explicit BmpConverterIndexed8BitToMonochrome(
            BmpImage*& bmp_image,
            std::vector<uint8_t>& data,
            const int p = 127
        ) : BmpConverter(bmp_image), data(data), p(p) {}

        void convert() override {
            std::vector<uint8_t> new_data;

            for (const auto pixel : data) {
                const auto to_add = pixel < p ? 0 : 255;
                new_data.push_back(to_add);
            }

            data.swap(new_data);
        }
    };
}


#endif
