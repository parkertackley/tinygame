#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "utils.h"
#include "textures.h"

Texture::Texture(const std::string filename) : img_w(0), img_h(0), count(0), size(0), img() {
    int nchannels = -1, w, h;

    // an array where each of pixel takes up 4 bytes. pixmap[0-3] are the rgba values of pixel 0,0, pixmap[4-7] are pixel 0,1, etc.
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);     // 0 tells stbi_load to use howevery many channels are in the orininal image
    if(!pixmap) {
        std::cerr << "Error: can not load the textures!" << std::endl;
        return;
    }

    // 4 channels = 32 bit image
    if(4 != nchannels) {
        std::cerr << "Error: the texture must be a 32 bit image!" << std::endl;
        stbi_image_free(pixmap);
        return;
    }

    if(w != h * int(w / h)) {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally!" << std::endl;
        stbi_image_free(pixmap);
        return;
    }

    count = w / h;      // # textures
    size = w / count;   // size of each
    img_w = w;      // width of the image
    img_h = h;      // height of the image

    img = std::vector<uint32_t>(w * h);     // this will store all the pixels of the image, each pixel is 32 bits/8 bytes
    for(int j = 0; j < h; ++j) {
        for(int i = 0; i < w; ++i) {
            uint8_t r = pixmap[(i + j * w) * 4 + 0];
            uint8_t g = pixmap[(i + j * w) * 4 + 1];
            uint8_t b = pixmap[(i + j * w) * 4 + 2];
            uint8_t a = pixmap[(i + j * w) * 4 + 3];
            img[i + j * w] = pack_color(r, g, b, a);
        }
    }

    stbi_image_free(pixmap);        // free the opened image

}

uint32_t &Texture::get(const size_t i, const size_t j, const size_t idx) {
    assert(i < size && j < size && idx < count);
    return img[i + idx * size + j * img_w];
}

std::vector<uint32_t> Texture::get_scaled_column(const size_t texture_id, const size_t tex_coord, const size_t column_height) {
    assert(tex_coord < size && texture_id < count);
    std::vector<uint32_t> column(column_height);
    for(size_t y = 0; y < column_height; ++y) {
        column[y] = get(tex_coord, (y * size) / column_height, texture_id);
    }
    return column;
}
