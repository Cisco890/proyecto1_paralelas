#include "cat.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

bool loadCat(Cat& cat, SDL_Renderer* renderer) {
    cat = {};
    cat.horizontalPosition = 0.72f;
    cat.horizontalSpeed = 0.07f;
    cat.direction = 1;
    cat.visibilityTimer = 0.0f;
    cat.visible = true;
    const std::array<const char*, 3> paths = {{
        "assets/animals/walking_cat_1.png",
        "assets/animals/walking_cat_2.png",
        "assets/animals/walking_cat_3.png"
    }};
    const std::array<const char*, 3> sleepingPaths = {{
        "assets/animals/sleeping_cat_frame_1.png",
        "assets/animals/sleeping_cat_frame_2.png",
        "assets/animals/sleeping_cat_frame_3.png"
    }};
    for (std::size_t index = 0; index < cat.frames.size(); ++index) {
        cat.frames[index] = IMG_LoadTexture(renderer, paths[index]);
        cat.sleepingFrames[index] = IMG_LoadTexture(renderer, sleepingPaths[index]);
        if (cat.frames[index] == nullptr || cat.sleepingFrames[index] == nullptr) {
            std::cerr << "Error cargando el gato: " << IMG_GetError() << std::endl;
            destroyCat(cat);
            return false;
        }
    }
    if (SDL_QueryTexture(cat.frames[0], nullptr, nullptr,
                         &cat.width, &cat.height) != 0) {
        std::cerr << "Error obteniendo dimensiones del gato: " << SDL_GetError() << std::endl;
        destroyCat(cat);
        return false;
    }
    return true;
}

void destroyCat(Cat& cat) {
    for (SDL_Texture*& frame : cat.frames) {
        if (frame != nullptr) SDL_DestroyTexture(frame);
        frame = nullptr;
    }
    for (SDL_Texture*& frame : cat.sleepingFrames) {
        if (frame != nullptr) SDL_DestroyTexture(frame);
        frame = nullptr;
    }
}

void updateCat(Cat& cat, int screenWidth, int groundY, float deltaSeconds,
               bool walking) {
    constexpr int renderSize = 100;
    const float aspectRatio = cat.height > 0
        ? static_cast<float>(cat.width) / cat.height
        : 1.0f;
    const int renderWidth = static_cast<int>(renderSize * aspectRatio);

    if (walking && screenWidth > renderWidth) {
        const float edgeMargin = static_cast<float>(renderWidth) /
                                 (2.0f * screenWidth);
        constexpr float visibleDuration = 12.0f;
        constexpr float hiddenDuration = 5.0f;

        cat.visibilityTimer += deltaSeconds;
        if (cat.visible) {
            cat.horizontalPosition += cat.horizontalSpeed * deltaSeconds * cat.direction;
            if (cat.horizontalPosition >= 1.0f - edgeMargin) {
                cat.horizontalPosition = 1.0f - edgeMargin;
                cat.direction = -1;
            } else if (cat.horizontalPosition <= edgeMargin) {
                cat.horizontalPosition = edgeMargin;
                cat.direction = 1;
            }

            if (cat.visibilityTimer >= visibleDuration) {
                cat.visibilityTimer = 0.0f;
                cat.visible = false;
                cat.horizontalPosition = cat.direction > 0 ? -0.15f : 1.15f;
            }
        } else if (cat.visibilityTimer >= hiddenDuration) {
            cat.visibilityTimer = 0.0f;
            cat.visible = true;
            cat.horizontalPosition = cat.direction > 0 ? edgeMargin : 1.0f - edgeMargin;
        }
    }

    cat.dest = {
        static_cast<int>(cat.horizontalPosition * screenWidth) - renderWidth / 2,
        groundY - renderSize + 30,
        renderWidth,
        renderSize
    };
}

void renderCat(SDL_Renderer* renderer, const Cat& cat, Uint32 currentTicks,
               bool sleeping, Uint8 opacity) {
    if (!cat.visible || opacity == 0 || cat.frames[0] == nullptr) return;
    const auto& animationFrames = sleeping ? cat.sleepingFrames : cat.frames;
    SDL_Texture* texture = animationFrames[(currentTicks / 420) % animationFrames.size()];
    SDL_SetTextureAlphaMod(texture, opacity);
    const SDL_RendererFlip flip = cat.direction < 0
        ? SDL_FLIP_HORIZONTAL
        : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, texture, nullptr, &cat.dest, 0.0, nullptr, flip);
    SDL_SetTextureAlphaMod(texture, 255);
}
