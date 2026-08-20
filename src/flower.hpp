#ifndef FLOWER_HPP
#define FLOWER_HPP

#include <SDL2/SDL.h>

#include <array>
#include <vector>

struct FlowerTextures {
    std::array<SDL_Texture*, 2> frames;
    int width;
    int height;
};

struct Flower {
    SDL_Rect dest;
    float horizontalPosition;
    float verticalPosition;
    Uint32 animationOffset;
};

bool loadFlowerTextures(
    FlowerTextures& textures,
    SDL_Renderer* renderer,
    const char* firstFramePath,
    const char* secondFramePath
);

void destroyFlowerTextures(FlowerTextures& textures);

void updateFlowerPosition(
    Flower& flower,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY
);

std::vector<Flower> createFlowerField(std::size_t count);

void updateFlowerPositionsParallel(
    std::vector<Flower>& flowers,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY
);

void renderFlower(
    SDL_Renderer* renderer,
    const FlowerTextures& textures,
    const Flower& flower,
    Uint32 currentTicks,
    Uint8 opacity = 255
);

#endif
