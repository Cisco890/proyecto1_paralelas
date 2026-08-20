#include "weather.hpp"

#include <SDL2/SDL_image.h>

#include <cmath>
#include <iostream>

bool loadWeatherSystem(
    WeatherSystem& system,
    SDL_Renderer* renderer,
    const std::vector<const char*>& paths,
    std::size_t particleCount,
    int scale,
    float minimumSpeed,
    float speedVariation
) {
    system = {};
    system.scale = scale;

    for (const char* path : paths) {
        SDL_Texture* texture = IMG_LoadTexture(renderer, path);
        if (texture == nullptr) {
            std::cerr << "Error cargando clima: " << IMG_GetError() << std::endl;
            destroyWeatherSystem(system);
            return false;
        }
        system.textures.push_back(texture);
    }

    if (SDL_QueryTexture(
            system.textures.front(), nullptr, nullptr,
            &system.textureWidth, &system.textureHeight
        ) != 0) {
        std::cerr << "Error obteniendo dimensiones del clima: "
                  << SDL_GetError() << std::endl;
        destroyWeatherSystem(system);
        return false;
    }

    system.particles.reserve(particleCount);
    for (std::size_t index = 0; index < particleCount; ++index) {
        const float x = static_cast<float>((index * 73) % 1000) / 1000.0f;
        const float y = -static_cast<float>((index * 47) % 1000) / 1000.0f;
        const float variation = static_cast<float>((index * 29) % 100) / 100.0f;
        const float drift = static_cast<float>(static_cast<int>(index % 7) - 3) * 4.0f;
        system.particles.push_back({
            x, y, minimumSpeed + speedVariation * variation, drift,
            index % system.textures.size()
        });
    }

    return true;
}

void destroyWeatherSystem(WeatherSystem& system) {
    for (SDL_Texture*& texture : system.textures) {
        if (texture != nullptr) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
    system.textures.clear();
    system.particles.clear();
}

void updateWeatherSystem(
    WeatherSystem& system,
    int screenWidth,
    int screenHeight,
    float deltaSeconds,
    float intensity
) {
    if (intensity <= 0.0f || screenWidth <= 0 || screenHeight <= 0) {
        return;
    }

    for (WeatherParticle& particle : system.particles) {
        particle.y += particle.speed * deltaSeconds / screenHeight;
        particle.x += particle.drift * deltaSeconds / screenWidth;

        if (particle.y > 1.05f) {
            particle.y = -0.05f;
        }
        if (particle.x > 1.02f) {
            particle.x = -0.02f;
        } else if (particle.x < -0.02f) {
            particle.x = 1.02f;
        }
    }
}

void renderWeatherSystem(
    SDL_Renderer* renderer,
    const WeatherSystem& system,
    float intensity
) {
    if (intensity <= 0.0f || system.textures.empty()) {
        return;
    }

    int screenWidth = 0;
    int screenHeight = 0;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);
    const float visible = intensity * system.particles.size();
    const std::size_t count = static_cast<std::size_t>(std::ceil(visible));

    for (std::size_t index = 0; index < count; ++index) {
        const WeatherParticle& particle = system.particles[index];
        SDL_Texture* texture = system.textures[particle.frame];
        Uint8 opacity = 255;
        if (index + 1 == count) {
            const float fraction = visible - std::floor(visible);
            if (fraction > 0.0f) {
                opacity = static_cast<Uint8>(fraction * 255.0f);
            }
        }
        SDL_SetTextureAlphaMod(texture, opacity);
        SDL_Rect destination = {
            static_cast<int>(particle.x * screenWidth),
            static_cast<int>(particle.y * screenHeight),
            system.textureWidth * system.scale,
            system.textureHeight * system.scale
        };
        SDL_RenderCopy(renderer, texture, nullptr, &destination);
        SDL_SetTextureAlphaMod(texture, 255);
    }
}
