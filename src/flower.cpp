#include "flower.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

bool loadFlowerTextures(
    FlowerTextures& textures,
    SDL_Renderer* renderer,
    const char* firstFramePath,
    const char* secondFramePath
) {
    textures = {};
    textures.frames[0] = IMG_LoadTexture(renderer, firstFramePath);
    textures.frames[1] = IMG_LoadTexture(renderer, secondFramePath);

    if (textures.frames[0] == nullptr || textures.frames[1] == nullptr) {
        std::cerr << "Error cargando las flores: " << IMG_GetError() << std::endl;
        destroyFlowerTextures(textures);
        return false;
    }

    if (SDL_QueryTexture(
            textures.frames[0], nullptr, nullptr,
            &textures.width, &textures.height
        ) != 0) {
        std::cerr << "Error obteniendo dimensiones de la flor: "
                  << SDL_GetError() << std::endl;
        destroyFlowerTextures(textures);
        return false;
    }

    return true;
}

void destroyFlowerTextures(FlowerTextures& textures) {
    for (SDL_Texture*& frame : textures.frames) {
        if (frame != nullptr) {
            SDL_DestroyTexture(frame);
            frame = nullptr;
        }
    }
}

void updateFlowerPosition(
    Flower& flower,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY
) {
    constexpr int scale = 3;
    const int width = textures.width * scale;
    const int height = textures.height * scale;
    const int centerX = static_cast<int>(screenWidth * flower.horizontalPosition);
    const int groundHeight = screenHeight - groundY;
    const int bottomY = groundY + static_cast<int>(
        flower.verticalPosition * (groundHeight - 2)
    );

    flower.dest = {centerX - width / 2, bottomY - height, width, height};
}

std::vector<Flower> createFlowerField(std::size_t count) {
    std::vector<Flower> flowers;
    flowers.reserve(count);

    // Secuencia determinista: mantiene el campo estable entre ejecuciones y
    // reparte las flores por toda el area verde sin solaparlas en una fila.
    for (std::size_t index = 0; index < count; ++index) {
        const float x = 0.03f + static_cast<float>((index * 37) % 94) / 100.0f;
        const float y = 0.18f + static_cast<float>((index * 53) % 76) / 100.0f;
        flowers.push_back({{}, x, y, static_cast<Uint32>((index % 2) * 175)});
    }

    return flowers;
}

void updateFlowerPositionsParallel(
    std::vector<Flower>& flowers,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY
) {
    if (flowers.empty()) {
        return;
    }

    const unsigned int available = std::thread::hardware_concurrency();
    const std::size_t workerCount = std::min<std::size_t>(
        available == 0 ? 2 : available,
        flowers.size()
    );
    const std::size_t chunkSize = (flowers.size() + workerCount - 1) / workerCount;
    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    for (std::size_t worker = 0; worker < workerCount; ++worker) {
        const std::size_t begin = worker * chunkSize;
        const std::size_t end = std::min(begin + chunkSize, flowers.size());

        workers.emplace_back([&, begin, end]() {
            for (std::size_t index = begin; index < end; ++index) {
                updateFlowerPosition(
                    flowers[index], textures, screenWidth, screenHeight, groundY
                );
            }
        });
    }

    for (std::thread& worker : workers) {
        worker.join();
    }
}

void renderFlower(
    SDL_Renderer* renderer,
    const FlowerTextures& textures,
    const Flower& flower,
    Uint32 currentTicks,
    Uint8 opacity
) {
    constexpr Uint32 frameDuration = 350;
    const std::size_t frame =
        ((currentTicks + flower.animationOffset) / frameDuration) % textures.frames.size();
    SDL_Texture* texture = textures.frames[frame];
    SDL_SetTextureAlphaMod(texture, opacity);
    SDL_RenderCopy(renderer, texture, nullptr, &flower.dest);
    SDL_SetTextureAlphaMod(texture, 255);
}
