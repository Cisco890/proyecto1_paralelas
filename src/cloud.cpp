#include "cloud.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <iostream>
#include <thread>

namespace {
void destroySet(std::array<SDL_Texture*, 2>& set) {
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
                      textures.width * 4, textures.height * 4};
    }
}
}

bool loadCloudTextures(
    CloudTextures& textures, SDL_Renderer* renderer,
    const char* normalFirst, const char* normalSecond,
    const char* rainFirst, const char* rainSecond
) {
    textures = {};
    textures.normal[0] = IMG_LoadTexture(renderer, normalFirst);
    textures.normal[1] = IMG_LoadTexture(renderer, normalSecond);
    textures.rain[0] = IMG_LoadTexture(renderer, rainFirst);
    textures.rain[1] = IMG_LoadTexture(renderer, rainSecond);

    if (!textures.normal[0] || !textures.normal[1] ||
        !textures.rain[0] || !textures.rain[1]) {
        std::cerr << "Error cargando las nubes: " << IMG_GetError() << std::endl;
        destroyCloudTextures(textures);
        return false;
    }

    if (SDL_QueryTexture(textures.normal[0], nullptr, nullptr,
                         &textures.width, &textures.height) != 0) {
        std::cerr << "Error obteniendo dimensiones de las nubes: "
                  << SDL_GetError() << std::endl;
        destroyCloudTextures(textures);
        return false;
    }
    return true;
}

void destroyCloudTextures(CloudTextures& textures) {
    destroySet(textures.normal);
    destroySet(textures.rain);
}

std::vector<Cloud> createCloudField(std::size_t count) {
    std::vector<Cloud> clouds;
    clouds.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        clouds.push_back({{}, -0.15f + static_cast<float>((index * 31) % 120) / 100.0f,
                          -0.15f + static_cast<float>((index * 19) % 2) / 100.0f,
                          0.012f + static_cast<float>(index % 4) * 0.004f,
                          static_cast<Uint32>(index * 130), index % 2});
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
    const unsigned int available = std::thread::hardware_concurrency();
    const std::size_t workersCount = std::min<std::size_t>(
        available == 0 ? 2 : available, clouds.size());
    const std::size_t chunk = (clouds.size() + workersCount - 1) / workersCount;
    std::vector<std::thread> workers;
    for (std::size_t worker = 0; worker < workersCount; ++worker) {
        const std::size_t begin = worker * chunk;
        const std::size_t end = std::min(begin + chunk, clouds.size());
        workers.emplace_back([&, begin, end]() {
            updateCloudRange(
                clouds, textures, screenWidth, screenHeight, deltaSeconds, begin, end
            );
        });
    }
    for (std::thread& worker : workers) worker.join();
}

void renderCloud(SDL_Renderer* renderer, const CloudTextures& textures,
                 const Cloud& cloud, bool stormy) {
    const auto& frames = stormy ? textures.rain : textures.normal;
    SDL_RenderCopy(renderer, frames[cloud.variant], nullptr, &cloud.dest);
}
