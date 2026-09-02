#include "flower.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <iostream>
#include <omp.h>
#include <vector>

namespace {
void updateFlowerRange(
    std::vector<Flower>& flowers,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY,
    std::size_t begin,
    std::size_t end
) {
    for (std::size_t index = begin; index < end; ++index) {
        updateFlowerPosition(
            flowers[index], textures, screenWidth, screenHeight, groundY
        );
    }
}
}

bool loadFlowerTextures(
    FlowerTextures& textures,
    SDL_Renderer* renderer,
    const char* firstFramePath,
    const char* secondFramePath
) {
    textures = {};
    textures.frames[0] = IMG_LoadTexture(renderer, firstFramePath);
    textures.frames[1] = IMG_LoadTexture(renderer, secondFramePath);
    textures.groundFrames[0] = IMG_LoadTexture(
        renderer, "assets/flowers/pink_ground_flower.png"
    );
    textures.groundFrames[1] = IMG_LoadTexture(
        renderer, "assets/flowers/light_pink_ground_flower.png"
    );

    if (textures.frames[0] == nullptr || textures.frames[1] == nullptr ||
        textures.groundFrames[0] == nullptr || textures.groundFrames[1] == nullptr) {
        std::cerr << "Error cargando las flores: " << IMG_GetError() << std::endl;
        destroyFlowerTextures(textures);
        return false;
    }

    if (SDL_QueryTexture(
            textures.groundFrames[0], nullptr, nullptr,
            &textures.groundWidth, &textures.groundHeight
        ) != 0) {
        std::cerr << "Error obteniendo dimensiones de la flor de suelo: "
                  << SDL_GetError() << std::endl;
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
    for (SDL_Texture*& frame : textures.groundFrames) {
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

void updateFlowerPositionsSequential(
    std::vector<Flower>& flowers,
    const FlowerTextures& textures,
    int screenWidth,
    int screenHeight,
    int groundY
) {
    updateFlowerRange(
        flowers, textures, screenWidth, screenHeight, groundY, 0, flowers.size()
    );
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

    const int requestedThreads = std::min<int>(
        omp_get_max_threads(), static_cast<int>(flowers.size())
    );

    // Cada hilo modifica un rango exclusivo. La barrera implicita al final
    // garantiza que todas las posiciones esten listas antes del renderizado.
    #pragma omp parallel num_threads(requestedThreads)
    {
        const std::size_t threadCount = static_cast<std::size_t>(
            omp_get_num_threads()
        );
        const std::size_t threadIndex = static_cast<std::size_t>(
            omp_get_thread_num()
        );
        const std::size_t chunkSize =
            (flowers.size() + threadCount - 1) / threadCount;
        const std::size_t begin = threadIndex * chunkSize;
        const std::size_t end = std::min(begin + chunkSize, flowers.size());
        updateFlowerRange(
            flowers, textures, screenWidth, screenHeight, groundY, begin, end
        );
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

void renderGroundFlower(
    SDL_Renderer* renderer,
    const FlowerTextures& textures,
    const Flower& flower,
    std::size_t variant,
    Uint8 opacity
) {
    if (textures.groundFrames.empty()) return;
    variant %= textures.groundFrames.size();
    SDL_Texture* texture = textures.groundFrames[variant];
    const int width = textures.groundWidth * 3;
    const int height = textures.groundHeight * 3;
    SDL_Rect destination = {
        flower.dest.x + (flower.dest.w - width) / 2,
        flower.dest.y + flower.dest.h - height,
        width,
        height
    };
    SDL_SetTextureAlphaMod(texture, opacity);
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
    SDL_SetTextureAlphaMod(texture, 255);
}
