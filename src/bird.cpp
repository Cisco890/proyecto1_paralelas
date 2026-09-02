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

bool loadAnimatedBird(Bird& bird, SDL_Renderer* renderer,
                      const std::array<const char*, 3>& paths,
                      float startX, float startY, Uint32 animationOffset) {
    bird = {};
    bird.x = startX;
    bird.y = startY;
    bird.velocityX = 120.0f;
    bird.animationOffset = animationOffset;

    for (std::size_t index = 0; index < bird.animationFrames.size(); ++index) {
        bird.animationFrames[index] = IMG_LoadTexture(renderer, paths[index]);
        if (bird.animationFrames[index] == nullptr) {
            std::cerr << "Error cargando los cuadros del pajaro: "
                      << IMG_GetError() << std::endl;
            destroyBird(bird);
            return false;
        }
    }

    if (SDL_QueryTexture(bird.animationFrames[0], nullptr, nullptr,
                         &bird.width, &bird.height) != 0) {
        std::cerr << "Error obteniendo dimensiones del pajaro: "
                  << SDL_GetError() << std::endl;
        destroyBird(bird);
        return false;
    }
    return true;
}

void destroyBird(Bird& bird) {
    if (bird.texture != nullptr) {
        SDL_DestroyTexture(bird.texture);
        bird.texture = nullptr;
    }
    for (SDL_Texture*& frame : bird.animationFrames) {
        if (frame != nullptr) {
            SDL_DestroyTexture(frame);
            frame = nullptr;
        }
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

    SDL_Texture* texture = bird.texture;
    if (bird.animationFrames[0] != nullptr) {
        texture = bird.animationFrames[((SDL_GetTicks() + bird.animationOffset) / 180) %
                                       bird.animationFrames.size()];
    }
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}
