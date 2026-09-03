#include "cloud.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <iostream>
#include <omp.h>

namespace {
constexpr int cloudRenderScale = 2;

template <std::size_t size>
void destroySet(std::array<SDL_Texture*, size>& set) {
    for (SDL_Texture*& texture : set) {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
}

void updateCloudRange(std::vector<Cloud>& clouds,
                      const CloudTextures& textures,
                      int screenWidth, int screenHeight,
                      float deltaSeconds,
                      std::size_t begin, std::size_t end) {
    for (std::size_t index = begin; index < end; ++index) {
        Cloud& cloud = clouds[index];
        cloud.x += cloud.speed * deltaSeconds;
        if (cloud.x > 1.15f) cloud.x = -0.20f;
        cloud.dest = {static_cast<int>(cloud.x * screenWidth),
                      static_cast<int>(cloud.y * screenHeight),
                      textures.width * cloudRenderScale,
                      textures.height * cloudRenderScale};
    }
}
}

bool loadCloudTextures(
    CloudTextures& textures, SDL_Renderer* renderer,
    const std::array<const char*, 3>& dayPaths,
    const std::array<const char*, 3>& nightPaths,
    const std::array<const char*, 3>& rainPaths
) {
    textures = {};
    for (std::size_t index = 0; index < 3; ++index) {
        textures.day[index] = IMG_LoadTexture(renderer, dayPaths[index]);
        textures.night[index] = IMG_LoadTexture(renderer, nightPaths[index]);
        textures.rain[index] = IMG_LoadTexture(renderer, rainPaths[index]);
        if (textures.day[index]) {
            SDL_SetTextureBlendMode(textures.day[index], SDL_BLENDMODE_BLEND);
        }
        if (textures.night[index]) {
            SDL_SetTextureBlendMode(textures.night[index], SDL_BLENDMODE_BLEND);
        }
        if (textures.rain[index]) {
            SDL_SetTextureBlendMode(textures.rain[index], SDL_BLENDMODE_BLEND);
        }
    }

    for (std::size_t index = 0; index < 3; ++index) {
        if (!textures.day[index] || !textures.night[index] ||
            !textures.rain[index]) {
            std::cerr << "Error cargando las nubes: " << IMG_GetError() << std::endl;
            destroyCloudTextures(textures);
            return false;
        }
    }

    if (SDL_QueryTexture(textures.day[0], nullptr, nullptr,
                         &textures.width, &textures.height) != 0) {
        std::cerr << "Error obteniendo dimensiones de las nubes: "
                  << SDL_GetError() << std::endl;
        destroyCloudTextures(textures);
        return false;
    }
    return true;
}

void destroyCloudTextures(CloudTextures& textures) {
    destroySet(textures.day);
    destroySet(textures.night);
    destroySet(textures.rain);
}

std::vector<Cloud> createCloudField(std::size_t count) {
    std::vector<Cloud> clouds;
    clouds.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        clouds.push_back({{}, -0.15f + static_cast<float>((index * 31) % 120) / 100.0f,
                          -0.03f + static_cast<float>((index * 19) % 4) * 0.08f,
                          0.012f + static_cast<float>(index % 4) * 0.004f,
                          static_cast<Uint32>(index * 130), index % 3});
    }
    return clouds;
}

void updateCloudPositionsSequential(std::vector<Cloud>& clouds,
                                    const CloudTextures& textures,
                                    int screenWidth, int screenHeight,
                                    float deltaSeconds) {
    if (clouds.empty() || screenWidth <= 0 || screenHeight <= 0) return;
    updateCloudRange(
        clouds, textures, screenWidth, screenHeight, deltaSeconds, 0, clouds.size()
    );
}

void updateCloudPositionsParallel(std::vector<Cloud>& clouds,
                                  const CloudTextures& textures,
                                  int screenWidth, int screenHeight,
                                  float deltaSeconds) {
    if (clouds.empty() || screenWidth <= 0 || screenHeight <= 0) return;
    const int requestedThreads = std::min<int>(
        omp_get_max_threads(), static_cast<int>(clouds.size())
    );

    #pragma omp parallel num_threads(requestedThreads)
    {
        const std::size_t threadCount = static_cast<std::size_t>(
            omp_get_num_threads()
        );
        const std::size_t threadIndex = static_cast<std::size_t>(
            omp_get_thread_num()
        );
        const std::size_t chunk =
            (clouds.size() + threadCount - 1) / threadCount;
        const std::size_t begin = threadIndex * chunk;
        const std::size_t end = std::min(begin + chunk, clouds.size());
        updateCloudRange(
            clouds, textures, screenWidth, screenHeight, deltaSeconds, begin, end
        );
    }
}

void renderCloud(SDL_Renderer* renderer, const CloudTextures& textures,
                 const Cloud& cloud, bool night, bool stormy, Uint8 opacity,
                 int verticalOffset) {
    if (opacity == 0) return;
    const auto& frames = stormy ? textures.rain
                                : (night ? textures.night : textures.day);
    SDL_Texture* texture = frames[cloud.variant];
    SDL_Rect destination = cloud.dest;
    destination.y += verticalOffset;
    SDL_SetTextureAlphaMod(texture, opacity);
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
    SDL_SetTextureAlphaMod(texture, 255);
}
