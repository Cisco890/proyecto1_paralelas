#include "flying_animal.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

bool loadFlyingAnimal(
    FlyingAnimal& animal,
    SDL_Renderer* renderer,
    const std::array<const char*, 3>& paths,
    float startX,
    float startY,
    float velocityX,
    float phase
) {
    animal = {};
    animal.x = startX;
    animal.y = startY;
    animal.baseY = startY;
    animal.velocityX = velocityX;
    animal.phase = phase;

    for (std::size_t index = 0; index < animal.frames.size(); ++index) {
        animal.frames[index] = IMG_LoadTexture(renderer, paths[index]);
        if (animal.frames[index] == nullptr) {
            std::cerr << "Error cargando animal animado: " << IMG_GetError() << std::endl;
            destroyFlyingAnimal(animal);
            return false;
        }
    }

    if (SDL_QueryTexture(
            animal.frames[0], nullptr, nullptr, &animal.width, &animal.height
        ) != 0) {
        std::cerr << "Error obteniendo dimensiones del animal: "
                  << SDL_GetError() << std::endl;
        destroyFlyingAnimal(animal);
        return false;
    }

    return true;
}

void destroyFlyingAnimal(FlyingAnimal& animal) {
    for (SDL_Texture*& frame : animal.frames) {
        if (frame != nullptr) {
            SDL_DestroyTexture(frame);
            frame = nullptr;
        }
    }
}

void updateFlyingAnimal(
    FlyingAnimal& animal,
    int screenWidth,
    int groundY,
    float deltaSeconds,
    Uint32 currentTicks
) {
    animal.x += animal.velocityX * deltaSeconds;
    if (animal.x > static_cast<float>(screenWidth + animal.width)) {
        animal.x = static_cast<float>(-animal.width * 3);
    }

    const float time = static_cast<float>(currentTicks) / 1000.0f;
    animal.y = animal.baseY + SDL_sin(time * 3.2f + animal.phase) * 18.0f;
    const float maximumY = static_cast<float>(groundY - animal.height * 3);
    if (animal.y > maximumY) {
        animal.y = maximumY;
    }
}

void renderFlyingAnimal(
    SDL_Renderer* renderer,
    const FlyingAnimal& animal,
    Uint32 currentTicks,
    Uint8 opacity
) {
    if (opacity == 0) {
        return;
    }

    constexpr Uint32 frameDuration = 120;
    const std::size_t frameIndex =
        (currentTicks / frameDuration) % animal.frames.size();
    SDL_Texture* texture = animal.frames[frameIndex];
    SDL_SetTextureAlphaMod(texture, opacity);

    constexpr int scale = 3;
    SDL_Rect destination = {
        static_cast<int>(animal.x),
        static_cast<int>(animal.y),
        animal.width * scale,
        animal.height * scale
    };
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
    SDL_SetTextureAlphaMod(texture, 255);
}
