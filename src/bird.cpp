#include "bird.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

bool loadBird(
    Bird& bird,
    SDL_Renderer* renderer,
    const char* path,
    float startX,
    float startY
) {
    bird = {};
    bird.x = startX;
    bird.y = startY;
    bird.velocityX = 120.0f;
    bird.velocityY = 0.0f;

    bird.texture = IMG_LoadTexture(renderer, path);

    if (bird.texture == nullptr) {
        std::cerr
            << "Error cargando el pajaro: "
            << IMG_GetError()
            << std::endl;
        return false;
    }

    SDL_QueryTexture(
        bird.texture,
        nullptr,
        nullptr,
        &bird.width,
        &bird.height
    );

    return true;
}

void destroyBird(Bird& bird) {
    if (bird.texture != nullptr) {
        SDL_DestroyTexture(bird.texture);
        bird.texture = nullptr;
    }
}

void updateBird(
    Bird& bird,
    int screenWidth,
    int screenHeight,
    float deltaSeconds
) {
    bird.x += bird.velocityX * deltaSeconds;
    bird.y += bird.velocityY * deltaSeconds;

    // Oscilacion suave vertical (vuelo)
    bird.velocityY = 25.0f * SDL_sin(bird.x * 0.01f);

    // Si sale por la derecha, reaparece a la izquierda
    if (bird.x > static_cast<float>(screenWidth)) {
        bird.x = static_cast<float>(-bird.width);
        bird.y = static_cast<float>(
            screenHeight * 0.15f +
            (SDL_GetTicks() % 200)
        );
    }

    // Mantenerlo en la zona del cielo
    float minY = 20.0f;
    float maxY = static_cast<float>(screenHeight) * 0.45f;

    if (bird.y < minY) {
        bird.y = minY;
    }

    if (bird.y > maxY) {
        bird.y = maxY;
    }
}

void renderBird(SDL_Renderer* renderer, const Bird& bird) {
    SDL_Rect dest = {
        static_cast<int>(bird.x),
        static_cast<int>(bird.y),
        bird.width,
        bird.height
    };

    SDL_RenderCopy(renderer, bird.texture, nullptr, &dest);
}
