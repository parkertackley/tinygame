#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <iostream>
#include <iostream>
#include <cstdint>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>

#include "map.h"
#include "utils.h"
#include "player.h"
#include "framebuffer.h"
#include "sprite.h"
#include "textures.h"

int wall_x_texcoord(const float hitx, const float hity, Texture &tex_walls) {
    float x = hitx - floor(hitx + 0.5);         // we give the location in the 2d map of where the ray hits the wall
    float y = hity - floor(hity + 0.5);         // we convert this to a value in the range [-0.5, 0.5] to know where in the wall texture the ray has hit
    int tex = x * tex_walls.size;               // tells us what x coord we hit, gives us the column of the texture

    if(std::abs(y) > std::abs(x))               // determine if we hit a vertical or a horizontal wall
        tex = y * tex_walls.size;

    if(tex < 0)                                 // make sure tex is always between 0 and tex_walls.size - 1
        tex += tex_walls.size;

    assert(tex >= 0 && tex < (int)tex_walls.size);

    return tex;     // return the column in the texture that we hit
}   

void map_show_sprite(Sprite &sprite, FrameBuffer &fb, Map &map) {
    const size_t rect_w = fb.w / (map.w * 2);
    const size_t rect_h = fb.h / map.h;
    fb.draw_rectangle(sprite.x * rect_w - 3, sprite.y * rect_h - 3, 6, 6, pack_color(255, 0, 0));

}

void draw_sprite(Sprite &sprite, std::vector<float> &depth_buffer, FrameBuffer &fb, Player &player, Texture &tex_sprites) {
    // dis from player to the sprite in radians
    float sprite_dir = atan2(sprite.y - player.y, sprite.x - player.x);

    // remove unnecessary perios from the relative direction
    while(sprite_dir - player.a > M_PI) 
        sprite_dir -= 2 * M_PI;
    while(sprite_dir - player.a < -M_PI)
        sprite_dir += 2 * M_PI;

    float sprite_dist = std::sqrt(pow(player.x - sprite.x, 2) + pow(player.y - sprite.y, 2));    // distance from the player to the sprite
    size_t sprite_screen_size = std::min(1000, static_cast<int>(fb.h / sprite.player_dist));   // screen sprite dist
    int h_offset = (sprite_dir - player.a) / player.fov * (fb.w / 2) + (fb.w / 2) / 2 - tex_sprites.size / 2;
    int v_offset = fb.h / 2 - sprite_screen_size / 2;

    for(size_t i = 0; i < sprite_screen_size; ++i) {
        if(depth_buffer[h_offset + i] < sprite.player_dist)
            continue;
        for(size_t j = 0; j < sprite_screen_size; ++j) {
            if(v_offset + int(j) < 0 || v_offset + j >= fb.h)
                continue;
            uint32_t color = tex_sprites.get(i * tex_sprites.size / sprite_screen_size, j * tex_sprites.size / sprite_screen_size, sprite.tex_id);
            uint8_t r, g, b, a;
            unpack_color(color, r, g, b, a);
            if(a > 128)
                fb.set_pixel(fb.w / 2 + h_offset + i, v_offset + j, color);
        }
    }
}

void render(FrameBuffer &fb, Map &map, Player &player, std::vector<Sprite> &sprites, Texture &tex_walls, Texture &tex_monst) {
    fb.clear(pack_color(255, 255, 255));    // clear the frame, fill with white


    /* 
        This first block is only for the minimap on the left side of the screen.
        Since the map is a 16x16 grid, we have to scale accordingly to fit the 1024x512 pixel screen.
    */
    // since the map is 16x16, we have to scale up the map. this finds out how many pixels each tile of the map has to be to fill up the frame buffer
    const size_t rect_w = fb.w / (map.w * 2);       // 1024 / (16 * 2) = 32 pixels wide in the buffer (we multiply by 2 b/c the map will fill up half the frame buffer)
    const size_t rect_h = fb.h / map.h;             // 512 / 16 = 32 pixels tall in the buffer

    // loop through the map size and width O(n^2)
    for(size_t j = 0; j < map.h; ++j) {
        for(size_t i = 0; i < map.w; ++i) {
            if(map.is_empty(i, j))              // if the space is blank, no texture
                continue;
            size_t rect_x = i * rect_w;         // scales the tile to up to what the frame buffer needs
            size_t rect_y = j * rect_h;         // this tells us where in the frame buffer we put the texture color
            size_t texid = map.get(i, j);       // get the value of the map index
            assert(texid < tex_walls.count);    // make sure were not accessing an out of bounds texture
            fb.draw_rectangle(rect_x, rect_y, rect_w, rect_h, tex_walls.get(0, 0, texid));      // get the top left pixel of the texture
        }
    }

    /* -------------------------------------------------------------------------------------------------------------------------------------------------------------------- */

    /* 
        This will handle the actual rendering. We do all of the calculations according to the top down map.
        If a ray touches a wall/texture, we have to render the texture in the "3D world"
        We want to send rays from the players left most view, to their right most view
        <-(player.a - fov/2) ------------- player.a ------------- (player.a + fov/2)->
        player.a - fov/2 = the left most ray
        player.a + fov/2 = the right most ray
        player.fov * i / (fb.w / 2) = the center ray
    */
    std::vector<float> depth_buffer(fb.w / 2, 1e3);     // will store the distance from the player to the first wall hit by the ith ray

    for(size_t i = 0; i < fb.w / 2; ++i) {      // we pan across the view from left to right
        float angle = (player.a - player.fov / 2) + (player.fov * i / float(fb.w / 2));     // (calculate the angle of the current ray in the frame
                //  =           left ray          +             the ith ray

        for(float t = 0; t < 20; t += 0.01) {    // the "distance" traveled along the ray
            float x = player.x + t * cos(angle);        
            float y = player.y + t * sin(angle);
            fb.set_pixel(x * rect_w, y * rect_h, pack_color(160, 160, 160));        // draws the visibility cone

            if(map.is_empty(x, y))
                continue;

            size_t texid = map.get(x, y);               // ray touches wayy so draw the vert column
            assert(texid < tex_walls.count);
            
            /* 
                t is the straight line distance from the player to the wall.
                    if we use this to render the texture, well get a fisheye effect
                the cos(difference) removes the fish eye
                    we get the distance between the current ray and the players view direction

            */
            float dist = t * cos(angle - player.a);     
            
            depth_buffer[i] = dist;
            
            size_t column_height = fb.h / dist;
            
            int x_texcoord = wall_x_texcoord(x, y, tex_walls);      // get the x coordinate of where the ray hit a texture
            
            std::vector<uint32_t> column = tex_walls.get_scaled_column(texid, x_texcoord, column_height);       

            int pix_x = i + fb.w / 2;

            for(size_t j = 0; j < column_height; ++j) {
                int pix_y = j + fb.h / 2 - column_height / 2;       // get the y value from the center of the screen
                if(pix_y >= 0 && pix_y < (int)fb.h) {
                    fb.set_pixel(pix_x, pix_y, column[j]);
                }
            }
            break;
        }   //ray marching loop
    }       // fov ray sweeping

    for(size_t i = 0; i < sprites.size(); ++i) {    // loop through all of the sprites
        sprites[i].player_dist = std::sqrt(pow(player.x - sprites[i].x, 2) + pow(player.y - sprites[i].y, 2));      // since all the sprites have a distance of 0 from the player
    }                                                                                                               // we update this by calculating the hypotenuse

    std::sort(sprites.begin(), sprites.end());  // sort the list of sprites from closest to furthest

    for(size_t i = 0; i < sprites.size(); ++i) {    // draw the sprites
        map_show_sprite(sprites[i], fb, map);
        draw_sprite(sprites[i], depth_buffer, fb, player, tex_monst);
    }

}

int main() {
    std::filesystem::create_directory("../frames");

    FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024 * 512, pack_color(255, 255, 255))};        // makes a frame buffer object with w, h, image of 1024*512 pixels, each of color white
    Player player {3.456, 2.345, 1.523, M_PI / 3.0};        // creates a player object x, y, view direction, fov
    Map map;                                                // map object
    Texture tex_walls("../textures/walltext.png");      // we create a texture object that holds all the pixel values and data about the texture png
    Texture tex_monst("../textures/monsters.png");      // we create a texture object that holds all the pixel values and data about the texture png
    if(!tex_walls.count || !tex_monst.count) {      // need both textures to load
        std::cerr << "Failed to load textures!" << std::endl;
        return -1;
    }

    std::vector<Sprite> sprites{{3.523, 3.812, 2, 0}, {1.834, 8.756, 2, 0}, {5.323, 5.365, 1, 0}, {4.123, 10.265, 1, 0}};       // create vector of sprites. each include the x, y, texture id, player distance

    render(fb, map, player, sprites, tex_walls, tex_monst);     // send frame buffer, map, player, sprites, and textures
    drop_ppm_image("../frames/out.ppm", fb.img, fb.w, fb.h);

    return 0;

}
