#ifndef GRASS_HPP
#define GRASS_HPP

#include <SDL2/SDL.h>

#include <array>
#include <vector>

struct GrassTextures {
    std::array<SDL_Texture*, 3> frames;
    int width;
    int height;
};

struct GrassBlade {
    float horizontalPosition;
    float verticalPosition;
    Uint32 animationOffset;
};

bool loadGrassTextures(
    GrassTextures& textures,
    SDL_Renderer* renderer,
    const std::array<const char*, 3>& paths
);
void destroyGrassTextures(GrassTextures& textures);
std::vector<GrassBlade> createGrassField(std::size_t count);
void renderGrassField(
    SDL_Renderer* renderer,
    const GrassTextures& textures,
    const std::vector<GrassBlade>& grass,
    int screenWidth,
    int screenHeight,
    int groundY,
    Uint32 currentTicks,
    float visibleCount
);

#endif
