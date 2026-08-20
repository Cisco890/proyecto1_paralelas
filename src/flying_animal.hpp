#ifndef FLYING_ANIMAL_HPP
#define FLYING_ANIMAL_HPP

#include <SDL2/SDL.h>

#include <array>

struct FlyingAnimal {
    std::array<SDL_Texture*, 3> frames;
    float x;
    float y;
    float velocityX;
    float baseY;
    float phase;
    int width;
    int height;
};

bool loadFlyingAnimal(
    FlyingAnimal& animal,
    SDL_Renderer* renderer,
    const std::array<const char*, 3>& paths,
    float startX,
    float startY,
    float velocityX,
    float phase
);

void destroyFlyingAnimal(FlyingAnimal& animal);
void updateFlyingAnimal(
    FlyingAnimal& animal,
    int screenWidth,
    int groundY,
    float deltaSeconds,
    Uint32 currentTicks
);
void renderFlyingAnimal(
    SDL_Renderer* renderer,
    const FlyingAnimal& animal,
    Uint32 currentTicks,
    Uint8 opacity
);

#endif
