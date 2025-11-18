#include <iostream>
#include <vector>
#include "SDL2/SDL.h"

#include "map.h"
#include "utils.h"
#include "player.h"
#include "framebuffer.h"
#include "sprite.h"
#include "textures.h"
#include "tinyraycaster.h"

int main() {
    std::filesystem::create_directory("../frames");

    FrameBuffer fb{1024, 512, std::vector<uint32_t>(1024 * 512, pack_color(255, 255, 255))};
    Player player {3.456, 2.345, 1.523, M_PI / 3.0};
    Map map;
    Texture tex_walls("../textures/walltext.png");
    Texture tex_monst("../textures/monsters.png");
    if(!tex_walls.count || !tex_monst.count) {
        std::cerr << "Failed to load textures!" << std::endl;
        return -1;
    }

    std::vector<Sprite> sprites{{3.523, 3.812, 2, 0}, {1.834, 8.756, 2, 0}, {5.323, 5.365, 1, 0}, {4.123, 10.265, 1, 0}};

    render(fb, map, player, sprites, tex_walls, tex_monst);

    SDL_Window      *window = nullptr;
    SDL_Renderer    *renderer = nullptr;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

}
