#include "leaf.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <omp.h>

namespace {
const char* springPaths[3] = {
    "assets/tree/spring_leaf_1.png",
    "assets/tree/spring_leaf_2.png",
    "assets/tree/spring_leaf_3.png"
};
const char* autumnPaths[3] = {
    "assets/tree/autum_leaf_1.png",
    "assets/tree/autum_leaf_2.png",
    "assets/tree/autum_leaf_3.png"
};

void updateLeafRange(std::vector<Leaf>& leaves, const LeafTextures& textures,
                     const SDL_Rect& treeDest, LeafSeason season,
                     float deltaSeconds, Uint32 currentTicks, float autumnProgress,
                     std::size_t begin, std::size_t end) {
    for (std::size_t i = begin; i < end; ++i) {
        Leaf& leaf = leaves[i];
        if (season == LeafSeason::Autumn && leaf.autumnLeaf && !leaf.settled) {
            const float fallStart = static_cast<float>(i % 48) / 48.0f * 0.82f;
            if (!leaf.falling && autumnProgress >= fallStart) leaf.falling = true;
            if (leaf.falling) {
                leaf.y += leaf.fallSpeed * deltaSeconds / 100.0f;
                if (leaf.y >= 0.96f) {
                    leaf.settled = true;
                    leaf.visible = false;
                }
            }
        } else if (season == LeafSeason::Spring && leaf.autumnLeaf) {
            // Al salir de otono se vacian las pilas para que vuelvan
            // a formarse gradualmente en la siguiente temporada.
            leaf.settled = false;
            leaf.visible = true;
            if (!leaf.falling) {
                leaf.y = 0.10f + static_cast<float>((i * 31) % 45) / 100.0f;
            }
            leaf.phase += deltaSeconds * 1.5f;
        } else if (season == LeafSeason::Spring && !leaf.falling) {
            leaf.phase += deltaSeconds * 1.5f;
        }
        const float sway = leaf.settled ? 0.0f :
            std::sin(currentTicks * 0.0025f + leaf.phase) * leaf.drift;
        const int size = std::max(4, textures.width * 2);
        leaf.dest = {treeDest.x + static_cast<int>(leaf.x * treeDest.w + sway) - size / 2,
                     treeDest.y + static_cast<int>(leaf.y * treeDest.h), size, size};
    }
}

void renderWindLeafGroup(SDL_Renderer* renderer, const LeafTextures& textures,
                         const std::vector<Leaf>& leaves, Uint32 currentTicks,
                         Uint8 opacity) {
    constexpr Uint32 windCycleMilliseconds = 11000;
    constexpr Uint32 gustDurationMilliseconds = 2800;
    const Uint32 cyclePosition = currentTicks % windCycleMilliseconds;
    if (cyclePosition >= gustDurationMilliseconds || leaves.size() <= 48) return;

    int screenWidth = 0;
    int screenHeight = 0;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
    const float progress = static_cast<float>(cyclePosition) /
        static_cast<float>(gustDurationMilliseconds);
    const std::size_t windLeafCount = std::min<std::size_t>(8, leaves.size() - 48);
    for (std::size_t offset = 0; offset < windLeafCount; ++offset) {
        const Leaf& leaf = leaves[48 + offset];
        const int size = std::max(4, textures.width * 2);
        SDL_Rect destination = {
            static_cast<int>((-0.12f + progress * 1.25f) * screenWidth + offset * 34),
            static_cast<int>(screenHeight * (0.32f + (offset % 4) * 0.07f) +
                             std::sin(progress * 12.0f + leaf.phase) * 24.0f),
            size, size
        };
        SDL_Texture* texture = textures.frames[static_cast<std::size_t>(LeafSeason::Autumn)]
            [(currentTicks + leaf.animationOffset) / 180 % 3];
        SDL_SetTextureAlphaMod(texture, opacity);
        SDL_RenderCopyEx(renderer, texture, nullptr, &destination,
                         progress * 540.0 + leaf.phase, nullptr, SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(texture, 255);
    }
}
}

bool loadLeafTextures(SDL_Renderer* renderer, LeafTextures& textures) {
    textures = {};
    for (std::size_t frame = 0; frame < 3; ++frame) {
        textures.frames[0][frame] = IMG_LoadTexture(renderer, springPaths[frame]);
        textures.frames[1][frame] = IMG_LoadTexture(renderer, autumnPaths[frame]);
        if (textures.frames[0][frame] == nullptr || textures.frames[1][frame] == nullptr) {
            std::cerr << "Error cargando hojas de primavera: " << IMG_GetError() << std::endl;
            destroyLeafTextures(textures);
            return false;
        }
    }
    if (SDL_QueryTexture(textures.frames[0][0], nullptr, nullptr,
                         &textures.width, &textures.height) != 0) {
        std::cerr << "Error obteniendo dimensiones de las hojas: "
                  << SDL_GetError() << std::endl;
        destroyLeafTextures(textures);
        return false;
    }
    return true;
}

void destroyLeafTextures(LeafTextures& textures) {
    for (auto& season : textures.frames) {
        for (SDL_Texture*& texture : season) {
            if (texture) SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
}

std::vector<Leaf> createLeafField(std::size_t count) {
    std::vector<Leaf> leaves;
    leaves.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        leaves.push_back({
            i % 12 == 0
                ? 0.18f + static_cast<float>((i * 53) % 64) / 100.0f
                : 0.08f + static_cast<float>((i * 53) % 85) / 100.0f,
            i % 12 == 0
                ? 0.10f + static_cast<float>((i * 31) % 45) / 100.0f
                : 0.04f + static_cast<float>((i * 31) % 62) / 100.0f,
            8.0f + static_cast<float>((i * 17) % 23),
            12.0f + static_cast<float>((i * 29) % 25),
            static_cast<float>((i * 47) % 360),
            false,
            i < 48,
            false,
            true,
            static_cast<Uint32>((i * 137) % 900)
        });
    }
    return leaves;
}

void updateLeavesSequential(std::vector<Leaf>& leaves, const LeafTextures& textures,
                            const SDL_Rect& treeDest, LeafSeason season,
                            float deltaSeconds, Uint32 currentTicks, float autumnProgress) {
    if (leaves.empty()) return;
    updateLeafRange(
        leaves, textures, treeDest, season, deltaSeconds, currentTicks, autumnProgress, 0, leaves.size()
    );
}

void updateLeavesParallel(std::vector<Leaf>& leaves, const LeafTextures& textures,
                          const SDL_Rect& treeDest, LeafSeason season,
                          float deltaSeconds, Uint32 currentTicks, float autumnProgress) {
    if (leaves.empty()) return;
    // Ocho hilos como maximo evitan sobredimensionar la region paralela
    // interactiva. OMP_NUM_THREADS puede reducir aun mas esta cantidad.
    const int requestedThreads = std::min<int>(
        8, std::min<int>(omp_get_max_threads(), static_cast<int>(leaves.size()))
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
            (leaves.size() + threadCount - 1) / threadCount;
        const std::size_t begin = threadIndex * chunk;
        const std::size_t end = std::min(begin + chunk, leaves.size());
        updateLeafRange(
            leaves, textures, treeDest, season, deltaSeconds, currentTicks, autumnProgress,
            begin, end
        );
    }
}

void renderLeaves(SDL_Renderer* renderer, const LeafTextures& textures,
                  const std::vector<Leaf>& leaves, LeafSeason season,
                  std::size_t visibleCount, Uint32 currentTicks, Uint8 opacity,
                  float autumnExitProgress) {
    const std::size_t seasonIndex = static_cast<std::size_t>(season);
    const std::size_t count = std::min(visibleCount, leaves.size());
    for (std::size_t i = 0; i < count; ++i) {
        const Leaf& leaf = leaves[i];
        if (!leaf.visible) continue;
        // El arbol base ya contiene su follaje. Solo se dibujan las hojas
        // que realmente estan en proceso de caida; las demas no quedan
        // suspendidas sobre la copa.
        if (season == LeafSeason::Spring && !leaf.falling) continue;
        if (season == LeafSeason::Autumn && !leaf.falling) continue;
        if (season == LeafSeason::Autumn && !leaf.autumnLeaf) continue;
        SDL_Rect destination = leaf.dest;
        Uint8 leafOpacity = opacity;
        double angle = leaf.settled ? 0.0 : std::sin(currentTicks * 0.002 + leaf.phase) * 12.0;
        if (season == LeafSeason::Autumn && !leaf.settled && autumnExitProgress > 0.0f) {
            int width = 0, height = 0;
            SDL_GetRendererOutputSize(renderer, &width, &height);
            const float gust = autumnExitProgress * autumnExitProgress;
            destination.x += static_cast<int>(gust * width * 0.85f);
            destination.y -= static_cast<int>(gust * height * 0.25f);
            angle = gust * 720.0 + leaf.phase;
            leafOpacity = static_cast<Uint8>(opacity * (1.0f - gust));
        }
        SDL_Texture* texture = textures.frames[seasonIndex]
            [(currentTicks + leaf.animationOffset) / 180 % 3];
        SDL_SetTextureAlphaMod(texture, leafOpacity);
        SDL_RenderCopyEx(renderer, texture, nullptr, &destination, angle, nullptr, SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(texture, 255);
    }

    if (season == LeafSeason::Autumn) {
        renderWindLeafGroup(renderer, textures, leaves, currentTicks, opacity);
    }
}
