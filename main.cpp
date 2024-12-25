#include <iostream>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cassert>
#include "stb_image.h"

#define STD_IMAGE_IMPLEMENTATION

uint32_t pack_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) {
    return (a << 24) + (b << 16) + (g << 8) + r;
}

bool load_texture(const std::string filename, std::vector<uint32_t> &texture, size_t text_size, size_t &text_cnt) {
    int nchannels = -1, w, h;
    unsigned char *pixmap = stbi_load(filename.c_str(), &w, &h, &nchannels, 0);
    if(!pixmap) {
        std::cerr << "Error: can not load the textures!" <<std::endl;
        return false;
    }

    if(4 != nchannels) {
        std::cerr << "Error: the texture must be a 32 bit image!" << std::endl;
        return false;
    }

    text_cnt = w / h;
    text_size = w / text_cnt;
    if(w != h * int(text_cnt)) {
        std::cerr << "Error: the texture file must contain N square textures packed horizontally!" << std::endl;
        stbi_image_free(pixmap);
        return false;
    }

    texture = std::vector<uint32_t>(w * h);
    for(int j = 0; j < h; ++j) {
        for(int i = 0; i < w; ++i) {
            uint8_t r = pixmap[(i + j * w) * 4 + 0];
            uint8_t g = pixmap[(i + j * w) * 4 + 1];
            uint8_t b = pixmap[(i + j * w) * 4 + 2];
            uint8_t a = pixmap[(i + j * w) * 4 + 3];
            texture[i + j * w] = pack_color(r, g, b, a);
        }
    }

    stbi_image_free(pixmap);
    return true;

}

void unpack_color(const uint32_t &color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = (color >> 0) & 255;
    g = (color >> 8) & 255;
    b = (color >> 16) & 255;
    a = (color >> 24) & 255;
}

void drop_ppm_image(const std::string filename, const std::vector<uint32_t> &image, const size_t w, const  size_t h) {
    assert(image.size() == w * h);

    std::ofstream ofs(filename, std::ios::binary);
    ofs << "P6\n" << w << " " << h << "\n255\n";

    for(size_t i = 0; i < h * w; ++i) {
        uint8_t r, g, b, a;
        unpack_color(image[i], r, g, b, a);
        ofs << static_cast<char>(r) << static_cast<char>(g) << static_cast<char>(b);
    }

    ofs.close();

}

void draw_rectangle(std::vector<uint32_t> &img, const size_t img_w, const size_t img_h, const size_t x, const size_t y, const size_t w, const size_t h, const uint32_t color) {
    assert(img.size() == img_w * img_h);
    for(size_t i = 0; i < w; ++i) {
        for(size_t j = 0; j < h; ++j) {
            size_t cx = x + i;
            size_t cy = y + j;
            if(cx >= img_w || cy >= img_h)
                continue;
            img[cx + cy * img_w] = color;
        }
    }
}

int main() {
    const size_t win_w = 1024;
    const size_t win_h = 512;
    std::vector<uint32_t> framebuffer(win_w * win_h, pack_color(255, 255, 255));

    // setting the map boundaries 
    const size_t map_w = 16, map_h = 16;
    const char map[] = "0000222222220000"\
                       "0              0"\
                       "0      11111   0"\
                       "0     0        0"\
                       "0     0  1110000"\
                       "0     3        0"\
                       "0   10000      0"\
                       "0   0   11100  0"\
                       "0   0   0      0"\
                       "0   0   1  00000"\
                       "0       1      0"\
                       "0       1      0"\
                       "0       0      0"\
                       "0 0000000      0"\
                       "0              0"\
                       "0000222222220000";
    assert(sizeof(map) == map_w * map_h + 1); // assert the map size is == to the calculation, +1 for null terminator in a string

    // player details
    float player_x = 3.4556;
    float player_y = 2.345;
    float player_a = 1.523;
    const float fov = M_PI / 3.0;

    const size_t ncolors = 10;
    std::vector<uint32_t> colors(ncolors);
    for(size_t i = 0; i < ncolors; ++i) {
        colors[i] = pack_color(rand()%255, rand()%255, rand()%255);
    }

    std::vector<uint32_t> walltext; // texture for walls
    size_t walltext_size; // texture dimesnsions
    size_t walltext_cnt;
    if(!load_texture("../walltext.png", walltext, walltext_size, walltext_cnt)) {
        std::cerr << "Failed to load wall textures" << std::endl;
        return -1;
    }

    // draws map
    const size_t rect_w = win_w / (map_w * 2);
    const size_t rect_h = win_h / map_h;

    for(size_t j = 0; j < map_h; ++j) {
        for(size_t i = 0; i < map_w; ++i) {
            if(map[i + j * map_w] == ' ')
                continue;
            size_t rect_x = i * rect_w;
            size_t rect_y = j * rect_h;
            size_t icolor = map[i + j * map_w] - '0';
            assert(icolor < ncolors);
            draw_rectangle(framebuffer, win_w, win_h, rect_x, rect_y, rect_w, rect_h, colors[icolor]);
        }
    }

        // raycasting loop
    for(size_t i = 0; i < win_w / 2; ++i) {
        float angle = player_a - fov / 2 + fov * i / float(win_w / 2);

        for(float t = 0; t < 20; t += 0.01) {
            float cx = player_x + t * cos(angle);
            float cy = player_y + t * sin(angle);

            size_t pix_x = cx * rect_w;
            size_t pix_y = cy * rect_h;
            framebuffer[pix_x + pix_y * win_w] = pack_color(160, 160, 160);

            if(map[int(cx) + int(cy) * map_w] != ' ') {
                size_t icolor = map[int(cx) + int(cy) * map_w] - '0';
                assert(icolor < ncolors);
                size_t column_height = win_h / (t * cos(angle - player_a)); // removes fish eye
                draw_rectangle(framebuffer, win_w, win_h, win_w / 2 + i, win_h / 2 - column_height / 2, 1, column_height, pack_color(0, 255, 255));
                break;
            }
            const size_t textid = 4;
            for(size_t i = 0; i < walltext_size; ++i) {
                for(size_t j = 0; j < walltext_size; ++j) {
                    framebuffer[i + j * win_w] = walltext[i + textid * walltext_size + j * walltext_size * walltext_cnt];
                }
            }
        }
    }

    drop_ppm_image("./out.ppm", framebuffer, win_w, win_h);

    return 0;

}
