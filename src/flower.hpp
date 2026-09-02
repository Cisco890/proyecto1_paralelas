#ifndef FLOWER_HPP
#define FLOWER_HPP

#include <SDL2/SDL.h>

#include <array>
#include <vector>

struct FlowerTextures {
    std::array<SDL_Texture*, 2> frames;
    std::array<SDL_Texture*, 2> groundFrames;
    int width;
    int height;
    int groundWidth;
    int groundHeight;
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

void updateFlowerPositionsSequential(
    std::vector<Flower>& flowers,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY
);

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

void renderGroundFlower(
    SDL_Renderer* renderer,
    const FlowerTextures& textures,
    const Flower& flower,
    std::size_t variant,
    Uint8 opacity = 255
);

#endif
