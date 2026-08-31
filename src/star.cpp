#include "star.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

namespace {
void renderStar(
    SDL_Renderer* renderer,
    const StarTextures& textures,
    const Star& star,
    int screenWidth,
    int skyHeight,
    Uint32 currentTicks,
    Uint8 opacity
) {
    const int centerX = static_cast<int>(star.horizontalPosition * screenWidth);
    const int centerY = static_cast<int>(star.verticalPosition * skyHeight);
    const int baseSize = std::max(
        4,
        static_cast<int>(
            std::min(screenWidth, skyHeight) * star.visualSize
        )
    );

    // Un brillo suave evita que todo el campo se vea completamente estatico.
    const float twinkle = 0.72f + 0.28f * std::sin(
        (static_cast<float>(currentTicks + star.animationOffset) / 700.0f)
    );
    const Uint8 finalOpacity = static_cast<Uint8>(opacity * twinkle);

    if (star.shootingStar && !textures.shootingFrames.empty()) {
        constexpr Uint32 cycleDuration = 10000;
        constexpr Uint32 flightDuration = 1400;
        const Uint32 cycle = (currentTicks + star.animationOffset) % cycleDuration;
        if (cycle < flightDuration) {
            const float progress = static_cast<float>(cycle) / flightDuration;
            const int headX = static_cast<int>(
                (1.10f - progress * 1.20f) * screenWidth
            );
            const int headY = static_cast<int>(
                (0.08f + star.verticalPosition * 0.46f + progress * 0.24f) * skyHeight
            );
            const Uint8 shootingOpacity = static_cast<Uint8>(
                opacity * (1.0f - progress)
            );
            const std::size_t frame =
                ((currentTicks + star.animationOffset) / 180) %
                textures.shootingFrames.size();
            SDL_Texture* shootingTexture = textures.shootingFrames[frame];
            SDL_Rect destination = {
                headX - textures.shootingWidth / 2,
                headY - textures.shootingHeight / 2,
                textures.shootingWidth,
                textures.shootingHeight
            };
            SDL_SetTextureAlphaMod(shootingTexture, shootingOpacity);
            SDL_RenderCopy(renderer, shootingTexture, nullptr, &destination);
            SDL_SetTextureAlphaMod(shootingTexture, 255);
        }
    }

    if (!textures.frames.empty()) {
        constexpr Uint32 frameDuration = 250;
        const std::size_t frame =
            ((currentTicks + star.animationOffset) / frameDuration) %
            textures.frames.size();
        SDL_Texture* texture = textures.frames[frame];
        const float aspectRatio = textures.height > 0
            ? static_cast<float>(textures.width) / textures.height
            : 1.0f;
        const int width = std::max(1, static_cast<int>(baseSize * aspectRatio));
        SDL_Rect destination = {
            centerX - width / 2,
            centerY - baseSize / 2,
            width,
            baseSize
        };
        SDL_SetTextureAlphaMod(texture, finalOpacity);
        SDL_RenderCopy(renderer, texture, nullptr, &destination);
        SDL_SetTextureAlphaMod(texture, 255);
        return;
    }

    const int radius = std::max(1, baseSize / 4);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, finalOpacity);
    for (int y = -radius; y <= radius; ++y) {
        const int halfWidth = static_cast<int>(
            std::sqrt(static_cast<float>(radius * radius - y * y))
        );
        SDL_RenderDrawLine(
            renderer,
            centerX - halfWidth,
            centerY + y,
            centerX + halfWidth,
            centerY + y
        );
    }
}
}

bool loadStarTextures(
    StarTextures& textures,
    SDL_Renderer* renderer,
    const std::vector<const char*>& framePaths,
    const std::vector<const char*>& shootingFramePaths
) {
    destroyStarTextures(textures);
    if (framePaths.empty()) return true;

    auto loadFrames = [&](const std::vector<const char*>& paths,
                          std::vector<SDL_Texture*>& destination) {
        for (const char* path : paths) {
            SDL_Texture* texture = IMG_LoadTexture(renderer, path);
            if (texture == nullptr) return false;
            destination.push_back(texture);
        }
        return true;
    };

    if (!loadFrames(framePaths, textures.frames) ||
        !loadFrames(shootingFramePaths, textures.shootingFrames)) {
        std::cerr << "Error cargando estrellas: " << IMG_GetError() << std::endl;
        destroyStarTextures(textures);
        return false;
    }

    if (!textures.frames.empty() && SDL_QueryTexture(
            textures.frames.front(), nullptr, nullptr,
            &textures.width, &textures.height
        ) != 0) {
        std::cerr << "Error obteniendo dimensiones de la estrella: "
                  << SDL_GetError() << std::endl;
        destroyStarTextures(textures);
        return false;
    }
    if (!textures.shootingFrames.empty() && SDL_QueryTexture(
            textures.shootingFrames.front(), nullptr, nullptr,
            &textures.shootingWidth, &textures.shootingHeight
        ) != 0) {
        std::cerr << "Error obteniendo dimensiones de estrella fugaz: "
                  << SDL_GetError() << std::endl;
        destroyStarTextures(textures);
        return false;
    }
    return true;
}

void destroyStarTextures(StarTextures& textures) {
    for (SDL_Texture*& texture : textures.frames) {
        if (texture != nullptr) SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    textures.frames.clear();
    for (SDL_Texture*& texture : textures.shootingFrames) {
        if (texture != nullptr) SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    textures.shootingFrames.clear();
    textures.width = 0;
    textures.height = 0;
    textures.shootingWidth = 0;
    textures.shootingHeight = 0;
}

std::vector<Star> createStarField(std::size_t count) {
    std::vector<Star> stars;
    stars.reserve(count);

    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_real_distribution<float> horizontal(0.02f, 0.98f);
    std::uniform_real_distribution<float> vertical(0.03f, 0.94f);
    std::uniform_real_distribution<float> size(0.007f, 0.014f);
    std::uniform_int_distribution<Uint32> offset(0, 1400);

    for (std::size_t index = 0; index < count; ++index) {
        stars.push_back({
            horizontal(generator),
            vertical(generator),
            size(generator),
            offset(generator),
            index % 24 == 0
        });
    }
    return stars;
}

void renderStarField(
    SDL_Renderer* renderer,
    const StarTextures& textures,
    const std::vector<Star>& stars,
    int screenWidth,
    int skyHeight,
    Uint32 currentTicks,
    float visibleCount
) {
    if (stars.empty() || screenWidth <= 0 || skyHeight <= 0) return;

    visibleCount = std::clamp(
        visibleCount,
        0.0f,
        static_cast<float>(stars.size())
    );
    const std::size_t completeStars = static_cast<std::size_t>(
        std::floor(visibleCount)
    );

    SDL_BlendMode previousBlendMode = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (std::size_t index = 0; index < completeStars; ++index) {
        renderStar(
            renderer, textures, stars[index], screenWidth, skyHeight,
            currentTicks, 255
        );
    }

    const float partialStar = visibleCount - completeStars;
    if (partialStar > 0.0f && completeStars < stars.size()) {
        renderStar(
            renderer, textures, stars[completeStars], screenWidth, skyHeight,
            currentTicks, static_cast<Uint8>(partialStar * 255.0f)
        );
    }

    SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
}
