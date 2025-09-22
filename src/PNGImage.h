#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <cstdio>
#include "util.h"
#include <png.h>

namespace {
struct PngReadStream {
    const unsigned char* data;
    size_t offset;
};

void read_from_buffer(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count_to_read) {
    PngReadStream* stream = static_cast<PngReadStream*>(png_get_io_ptr(png_ptr));
    memcpy(out_bytes, stream->data + stream->offset, byte_count_to_read);
    stream->offset += byte_count_to_read;
}
}

class PNGImage {
private:
    bool m_is_valid = false;

public:
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t channels = 0;
    std::string file_name;
    std::vector<unsigned char> pixels;

    explicit PNGImage(const std::string& relative_path) {
        file_name = relative_path;
        std::vector<unsigned char> png_data;
        try {
            png_data = read_file_bytes("resources/" + relative_path);
        } catch (const std::runtime_error& e) {
            fprintf(stderr, "png_image: %s\n", e.what());
            return;
        }

        if (png_data.size() < 8 || png_sig_cmp(png_data.data(), 0, 8) != 0) {
            fprintf(stderr, "png_image: Not a valid PNG file: %s\n", relative_path.c_str());
            return;
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr) {
            fprintf(stderr, "png_image: png_create_read_struct failed.\n");
            return;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            fprintf(stderr, "png_image: png_create_info_struct failed.\n");
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            return;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            fprintf(stderr, "png_image: Error during PNG read.\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return;
        }

        PngReadStream stream{png_data.data(), 8};
        png_set_read_fn(png_ptr, &stream, read_from_buffer);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        if (bit_depth == 16) png_set_strip_16(png_ptr);
        if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
        if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
        }
        if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
            png_set_gray_to_rgb(png_ptr);
        }

        png_read_update_info(png_ptr, info_ptr);
        
        size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        channels = row_bytes / width;
        pixels.resize(row_bytes * height);

        std::vector<png_bytep> row_pointers(height);
        for (uint32_t i = 0; i < height; ++i) {
            row_pointers[i] = pixels.data() + (i * row_bytes);
        }

        png_read_image(png_ptr, row_pointers.data());
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

        m_is_valid = true;
    }

    bool is_valid() const {
        return m_is_valid;
    }
};
