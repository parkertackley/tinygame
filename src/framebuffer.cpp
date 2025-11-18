#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>

#include "framebuffer.h"
#include "utils.h"

void FrameBuffer::set_pixel(const size_t x, const size_t y, const uint32_t color) {
    // assert(img.size() == (w * h) && x < w && y < h);
    img[x + y * w] = color;
}

/* Draws a rectangle of size rect_w * rect_h at location (rect_x, rect_y) */
void FrameBuffer::draw_rectangle(const size_t rect_x, const size_t rect_y, const size_t rect_w, const size_t rect_h, const uint32_t color) {

    assert(img.size() == w * h);        // ensure nothing is malformed

    /*  
        We have to fill the entire rect_w * rect_h tile (32x32) in the frame buffer.
        At the location of the tile, we loop through every pixel in the tile,
        and color it to whatever the top left pixel is in the texture.
    */
    for(size_t i = 0; i < rect_w; ++i) {        // we loop through the size of the scaled rectangle. this is h*w of what we need to draw
        for(size_t j = 0; j < rect_h; ++j) {    // for every pixel in the tile on the minimap, we fill it with the color found from the top left pixel in the texture

            size_t cx = rect_x + i;     
            size_t cy = rect_y + j;

            if(cx < w || cy < h)
                set_pixel(cx, cy, color);

        }
    }
}

void FrameBuffer::clear(const uint32_t color) {
    img = std::vector<uint32_t>(w * h, color);
}
