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
            i % 12 == 0,
            i < 48,
            false,
            static_cast<Uint32>((i * 137) % 900)
        });
    }
    return leaves;
}

void updateLeavesParallel(std::vector<Leaf>& leaves, const LeafTextures& textures,
                          const SDL_Rect& treeDest, LeafSeason season,
                          float deltaSeconds, Uint32 currentTicks) {
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
            for (std::size_t i = begin; i < end; ++i) {
                Leaf& leaf = leaves[i];
                if (season == LeafSeason::Spring && leaf.falling) {
                    leaf.settled = false;
                    leaf.y += leaf.fallSpeed * deltaSeconds / 100.0f;
                    if (leaf.y > 1.12f) {
                        leaf.y = -0.08f;
                        leaf.x = 0.18f + static_cast<float>((i * 53) % 64) / 100.0f;
                    }
                } else if (season == LeafSeason::Autumn && leaf.autumnLeaf && !leaf.settled) {
                    leaf.y += leaf.fallSpeed * deltaSeconds / 100.0f;
                    if (leaf.y >= 0.96f) {
                        leaf.y = 0.96f + static_cast<float>(i % 5) * 0.008f;
                        leaf.settled = true;
                    }
                } else if (season == LeafSeason::Spring && leaf.autumnLeaf) {
                    // Al salir de otono se vacian las pilas para que vuelvan
                    // a formarse gradualmente en la siguiente temporada.
                    leaf.settled = false;
                    if (!leaf.falling) {
                        leaf.y = 0.10f + static_cast<float>((i * 31) % 45) / 100.0f;
                    }
                    leaf.phase += deltaSeconds * 1.5f;
                } else if (season == LeafSeason::Spring && !leaf.falling) {
                    leaf.phase += deltaSeconds * 1.5f;
                }
                const float sway = std::sin(currentTicks * 0.0025f + leaf.phase) * leaf.drift;
                const int size = std::max(4, textures.width * 2);
                leaf.dest = {treeDest.x + static_cast<int>(leaf.x * treeDest.w + sway) - size / 2,
                             treeDest.y + static_cast<int>(leaf.y * treeDest.h), size, size};
            }
        });
    }
    for (auto& worker : workers) worker.join();
}

void renderLeaves(SDL_Renderer* renderer, const LeafTextures& textures,
                  const std::vector<Leaf>& leaves, LeafSeason season,
                  std::size_t visibleCount, Uint32 currentTicks, Uint8 opacity) {
    const std::size_t seasonIndex = static_cast<std::size_t>(season);
    const std::size_t count = std::min(visibleCount, leaves.size());
    for (std::size_t i = 0; i < count; ++i) {
        const Leaf& leaf = leaves[i];
        // El arbol base ya contiene su follaje. Solo se dibujan las hojas
        // que realmente estan en proceso de caida; las demas no quedan
        // suspendidas sobre la copa.
        if (season == LeafSeason::Spring && !leaf.falling) continue;
        if (season == LeafSeason::Autumn && !leaf.autumnLeaf) continue;
        SDL_Texture* texture = textures.frames[seasonIndex]
            [(currentTicks + leaf.animationOffset) / 180 % 3];
        SDL_SetTextureAlphaMod(texture, opacity);
        const double angle = std::sin(currentTicks * 0.002 + leaf.phase) * 12.0;
        SDL_RenderCopyEx(renderer, texture, nullptr, &leaf.dest, angle, nullptr, SDL_FLIP_NONE);
        SDL_SetTextureAlphaMod(texture, 255);
    }
}
