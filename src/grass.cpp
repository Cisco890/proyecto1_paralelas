#include "grass.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>

bool loadGrassTextures(
    GrassTextures& textures,
    SDL_Renderer* renderer,
    const std::array<std::array<const char*, 3>, 3>& paths
) {
    textures = {};
    for (std::size_t season = 0; season < textures.frames.size(); ++season) {
        for (std::size_t frame = 0; frame < textures.frames[season].size(); ++frame) {
            textures.frames[season][frame] = IMG_LoadTexture(renderer, paths[season][frame]);
            if (textures.frames[season][frame] == nullptr) {
                std::cerr << "Error cargando grama: " << IMG_GetError() << std::endl;
                destroyGrassTextures(textures);
                return false;
            }
        }

        if (SDL_QueryTexture(textures.frames[season][0], nullptr, nullptr,
                             &textures.widths[season],
                             &textures.heights[season]) != 0) {
            std::cerr << "Error obteniendo dimensiones de la grama: "
                      << SDL_GetError() << std::endl;
            destroyGrassTextures(textures);
            return false;
        }
    }
    return true;
}

void destroyGrassTextures(GrassTextures& textures) {
    for (auto& season : textures.frames) {
        for (SDL_Texture*& frame : season) {
            if (frame != nullptr) {
                SDL_DestroyTexture(frame);
                frame = nullptr;
            }
        }
    }
}

std::vector<GrassBlade> createGrassField(std::size_t count) {
    std::vector<GrassBlade> grass;
    grass.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        grass.push_back({
            0.01f + static_cast<float>((index * 43) % 98) / 100.0f,
            0.12f + static_cast<float>((index * 67) % 86) / 100.0f,
            static_cast<Uint32>((index % 3) * 90)
        });
    }
    return grass;
}

void renderGrassField(
    SDL_Renderer* renderer,
    const GrassTextures& textures,
    const std::vector<GrassBlade>& grass,
    int screenWidth,
    int screenHeight,
    int groundY,
    Uint32 currentTicks,
    float visibleCount,
    std::size_t seasonIndex
) {
    seasonIndex = std::min(seasonIndex, textures.frames.size() - 1);
    const float limited = std::min(visibleCount, static_cast<float>(grass.size()));
    const std::size_t count = static_cast<std::size_t>(std::ceil(limited));
    constexpr int scale = 3;

    for (std::size_t index = 0; index < count; ++index) {
        const GrassBlade& blade = grass[index];
        const std::size_t frame =
            ((currentTicks + blade.animationOffset) / 180) % textures.frames[seasonIndex].size();
        SDL_Texture* texture = textures.frames[seasonIndex][frame];
        Uint8 opacity = 255;
        if (index + 1 == count) {
            const float fraction = limited - std::floor(limited);
            if (fraction > 0.0f) {
                opacity = static_cast<Uint8>(fraction * 255.0f);
            }
        }
        SDL_SetTextureAlphaMod(texture, opacity);
        SDL_Rect destination = {
            static_cast<int>(blade.horizontalPosition * screenWidth),
            groundY + static_cast<int>(
                blade.verticalPosition * (screenHeight - groundY)
            ) - textures.heights[seasonIndex] * scale,
            textures.widths[seasonIndex] * scale,
            textures.heights[seasonIndex] * scale
        };
        SDL_RenderCopy(renderer, texture, nullptr, &destination);
        SDL_SetTextureAlphaMod(texture, 255);
    }
}
