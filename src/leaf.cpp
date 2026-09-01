#include "leaf.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>

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
            if (!leaf.falling && autumnProgress >= fallStart) {
                leaf.falling = true;
            }
            if (leaf.falling) {
                leaf.y += leaf.fallSpeed * deltaSeconds / 100.0f;
                if (leaf.y >= 0.96f) {
                    leaf.y = 0.96f + static_cast<float>(i % 5) * 0.008f;
                    leaf.settled = true;
                    // Al llegar al suelo se retira de la escena; no se forma
                    // una pila de hojas durante otono.
                    leaf.visible = false;
                }
            }
        } else if (season == LeafSeason::Spring) {
            // Primavera y verano conservan las hojas en la copa. Al salir
            // de otono se reinician las que quedaron acumuladas en el suelo.
            if (leaf.autumnLeaf && leaf.settled) {
                leaf.y = 0.10f + static_cast<float>((i * 31) % 45) / 100.0f;
            }
            leaf.settled = false;
            leaf.falling = false;
            leaf.visible = true;
            leaf.phase += deltaSeconds * 1.5f;
        }
        const bool isSettledAutumnLeaf =
            season == LeafSeason::Autumn && leaf.autumnLeaf && leaf.settled;
        const float sway = isSettledAutumnLeaf
            ? 0.0f
            : std::sin(currentTicks * 0.0025f + leaf.phase) * leaf.drift;
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
    const std::size_t firstWindLeaf = 48;
    const std::size_t windLeafCount = std::min<std::size_t>(8, leaves.size() - firstWindLeaf);

    // Estas hojas no pertenecen a la pila del suelo: forman una rafaga breve
    // que cruza el escenario mientras las hojas asentadas permanecen inmoviles.
    for (std::size_t offset = 0; offset < windLeafCount; ++offset) {
        const Leaf& leaf = leaves[firstWindLeaf + offset];
        const float x = (-0.12f + progress * 1.25f) * screenWidth +
            static_cast<float>(offset * 34);
        const float y = screenHeight * (0.32f + static_cast<float>(offset % 4) * 0.07f) +
            std::sin(progress * 12.0f + leaf.phase) * 24.0f;
        const int size = std::max(4, textures.width * 2);
        SDL_Rect destination = {
            static_cast<int>(x), static_cast<int>(y), size, size
        };
        SDL_Texture* texture = textures.frames[static_cast<std::size_t>(LeafSeason::Autumn)]
            [(currentTicks + leaf.animationOffset) / 180 % 3];
        SDL_SetTextureAlphaMod(texture, opacity);
        SDL_RenderCopyEx(
            renderer, texture, nullptr, &destination,
            progress * 540.0 + leaf.phase, nullptr, SDL_FLIP_NONE
        );
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
        leaves, textures, treeDest, season, deltaSeconds, currentTicks, autumnProgress,
        0, leaves.size()
    );
}

void updateLeavesParallel(std::vector<Leaf>& leaves, const LeafTextures& textures,
                          const SDL_Rect& treeDest, LeafSeason season,
                          float deltaSeconds, Uint32 currentTicks, float autumnProgress) {
    if (leaves.empty()) return;
    // No se crea un hilo por hoja: en equipos con muchos nucleos eso puede
    // generar decenas de hilos por cuadro y hacer que SDL parezca congelado.
    // Se conserva el paralelismo usando un grupo pequeno y estable.
    const unsigned int available = std::max(1u, std::thread::hardware_concurrency());
    const std::size_t workersCount = std::min<std::size_t>(
        leaves.size(), std::min<unsigned int>(available, 8u));
    const std::size_t chunk = (leaves.size() + workersCount - 1) / workersCount;
    std::vector<std::thread> workers;
    for (std::size_t w = 0; w < workersCount; ++w) {
        const std::size_t begin = w * chunk;
        const std::size_t end = std::min(begin + chunk, leaves.size());
        workers.emplace_back([&, begin, end]() {
            updateLeafRange(
                leaves, textures, treeDest, season, deltaSeconds, currentTicks,
                autumnProgress, begin, end
            );
        });
    }
    for (auto& worker : workers) worker.join();
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
        if (season == LeafSeason::Autumn && !leaf.autumnLeaf) continue;
        SDL_Rect destination = leaf.dest;
        Uint8 leafOpacity = opacity;
        double angle = std::sin(currentTicks * 0.002 + leaf.phase) * 12.0;
        if (season == LeafSeason::Autumn && leaf.settled) angle = 0.0;
        if (season == LeafSeason::Autumn && !leaf.settled &&
            autumnExitProgress > 0.0f) {
            int screenWidth = 0;
            int screenHeight = 0;
            SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
            const float gust = autumnExitProgress * autumnExitProgress;
            destination.x += static_cast<int>(gust * screenWidth * 0.85f);
            destination.y -= static_cast<int>(gust * screenHeight *
                (0.22f + static_cast<float>(i % 4) * 0.06f));
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
