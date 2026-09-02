#include "cat.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

bool loadCat(Cat& cat, SDL_Renderer* renderer) {
    cat = {};
    cat.horizontalPosition = 0.72f;
    const std::array<const char*, 3> paths = {{
        "assets/animals/sitting_cat_frame_1.png",
        "assets/animals/sitting_cat_frame_2.png",
        "assets/animals/sitting_cat_frame_2.png"
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

void updateCat(Cat& cat, int screenWidth, int groundY) {
    constexpr int renderSize = 100;
    const float aspectRatio = cat.height > 0
        ? static_cast<float>(cat.width) / cat.height
        : 1.0f;
    const int renderWidth = static_cast<int>(renderSize * aspectRatio);
    cat.dest = {
        static_cast<int>(cat.horizontalPosition * screenWidth) - renderWidth / 2,
        groundY - renderSize + 30,
        renderWidth,
        renderSize
    };
}

void renderCat(SDL_Renderer* renderer, const Cat& cat, Uint32 currentTicks,
               bool sleeping, Uint8 opacity) {
    if (opacity == 0 || cat.frames[0] == nullptr) return;
    const auto& animationFrames = sleeping ? cat.sleepingFrames : cat.frames;
    SDL_Texture* texture = animationFrames[(currentTicks / 420) % animationFrames.size()];
    SDL_SetTextureAlphaMod(texture, opacity);
    SDL_RenderCopy(renderer, texture, nullptr, &cat.dest);
    SDL_SetTextureAlphaMod(texture, 255);
}
