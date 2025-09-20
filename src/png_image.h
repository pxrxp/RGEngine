#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <png.h>

class png_image {
private:
    bool m_is_valid = false;

public:
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<unsigned char> pixels;

    // Loads a PNG from a file path into 8-bit RGBA format.
    // Errors are printed to stderr. Check is_valid() after creation.
    explicit png_image(const std::string& file_path) {
        FILE* fp = fopen(file_path.c_str(), "rb");
        if (!fp) {
            fprintf(stderr, "png_image: Failed to open file: %s\n", file_path.c_str());
            return;
        }

        unsigned char header[8];
        if (fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8) != 0) {
            fprintf(stderr, "png_image: Not a valid PNG file: %s\n", file_path.c_str());
            fclose(fp);
            return;
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr) {
            fprintf(stderr, "png_image: png_create_read_struct failed.\n");
            fclose(fp);
            return;
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            fprintf(stderr, "png_image: png_create_info_struct failed.\n");
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            fclose(fp);
            return;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            fprintf(stderr, "png_image: Error during PNG read.\n");
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            fclose(fp);
            return;
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        // --- Transformations to get standard 8-bit RGBA ---
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
        
        // --- Read image data ---
        size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        pixels.resize(row_bytes * height);

        std::vector<png_bytep> row_pointers(height);
        for (uint32_t i = 0; i < height; ++i) {
            row_pointers[i] = pixels.data() + (i * row_bytes);
        }

        png_read_image(png_ptr, row_pointers.data());

        // --- Cleanup ---
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(fp);

        m_is_valid = true; // Success!
    }

    // Returns true if the image was loaded successfully.
    bool is_valid() const {
        return m_is_valid;
    }
};
