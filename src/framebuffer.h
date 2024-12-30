#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstdlib>
#include <vector>

struct FrameBuffer {
    size_t w, h;                    // image dimentions
    std::vector<uint32_t> img;      // storage container

    void clear(const uint32_t color);
    void set_pixel(const size_t x, const size_t y, const uint32_t color);
    void draw_rectangle(const size_t rect_x, const size_t rect_y, const size_t rect_w, const size_t rect_h, const uint32_t pack_color);
    void draw_rectangle(const size_t x, const size_t y, const size_t h, const uint32_t color);
};

#endif
