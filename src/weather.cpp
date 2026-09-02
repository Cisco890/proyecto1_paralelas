#include "weather.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <omp.h>

namespace {
float pseudoRandom(std::size_t index, unsigned int seed) {
    unsigned int value = static_cast<unsigned int>(index) * 747796405u + seed;
    value = (value ^ (value >> 16u)) * 2246822519u;
    value ^= value >> 13u;
    return static_cast<float>(value & 0xffffu) / 65535.0f;
}
}

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
    system.spawnY = -0.05f;
    system.groundY = 0.85f;

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
        const float x = pseudoRandom(index, 17u);
        const float y = system.spawnY + pseudoRandom(index, 31u) * 0.03f;
        const float variation = pseudoRandom(index, 47u);
        const float drift = (pseudoRandom(index, 61u) - 0.5f) * 14.0f;
        system.particles.push_back({
            x, y, minimumSpeed + speedVariation * variation, drift,
            index % system.textures.size(), 0.0f, false
        });
    }

    return true;
}

void setWeatherImpactAnimation(WeatherSystem& system, bool enabled,
                               float groundNormalized) {
    system.impactAnimation = enabled;
    system.groundY = groundNormalized;
    for (WeatherParticle& particle : system.particles) {
        particle.frame = 0;
        particle.impactElapsed = 0.0f;
        particle.impacted = false;
    }
}

void setWeatherAccumulation(WeatherSystem& system, bool enabled,
                            float groundNormalized) {
    system.accumulate = enabled;
    system.groundY = groundNormalized;
    system.accumulationHeights.assign(24, 0);
    for (WeatherParticle& particle : system.particles) {
        particle.impacted = false;
        particle.impactElapsed = 0.0f;
    }
}

void setWeatherSpawnHeight(WeatherSystem& system, float normalizedY) {
    system.spawnY = normalizedY;
    for (std::size_t index = 0; index < system.particles.size(); ++index) {
        system.particles[index].y = normalizedY +
            pseudoRandom(index, 31u) * 0.03f;
    }
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
    system.accumulationHeights.clear();
}

void updateWeatherSystem(
    WeatherSystem& system,
    int screenWidth,
    int screenHeight,
    float deltaSeconds,
    float intensity
) {
    if (intensity <= 0.0f || screenWidth <= 0 || screenHeight <= 0) {
        if (system.accumulate && intensity <= 0.0f) {
            std::fill(system.accumulationHeights.begin(),
                      system.accumulationHeights.end(), 0);
            for (std::size_t index = 0; index < system.particles.size(); ++index) {
                system.particles[index].y = system.spawnY - 0.02f;
                system.particles[index].frame = 0;
                system.particles[index].impacted = false;
            }
        }
        return;
    }

    for (std::size_t index = 0; index < system.particles.size(); ++index) {
        WeatherParticle& particle = system.particles[index];
        if (system.accumulate) {
            if (particle.impacted) continue;
            particle.y += particle.speed * deltaSeconds / screenHeight;
            particle.x += particle.drift * deltaSeconds / screenWidth;
            if (particle.y >= system.groundY) {
                constexpr std::size_t moundCount = 24;
                const std::size_t mound = std::min(
                    moundCount - 1,
                    static_cast<std::size_t>(std::max(0.0f, particle.x) * moundCount)
                );
                const std::size_t layer = system.accumulationHeights[mound]++;
                // Se conserva una pequena variacion horizontal dentro de cada
                // franja para formar monticulos anchos y no columnas.
                particle.x = (static_cast<float>(mound) + 0.20f +
                              pseudoRandom(index, 73u) * 0.60f) / moundCount;
                particle.y = system.groundY -
                    static_cast<float>(std::min<std::size_t>(layer, 12)) * 0.007f;
                particle.frame = index % system.textures.size();
                particle.impacted = true;
            }
            continue;
        }
        if (system.impactAnimation) {
            if (particle.impacted) {
                particle.impactElapsed += deltaSeconds;
                if (particle.impactElapsed < 0.12f) {
                    particle.frame = 1;
                } else if (particle.impactElapsed < 0.42f) {
                    particle.frame = 2;
                } else {
                    particle.y = system.spawnY - 0.02f;
                    particle.frame = 0;
                    particle.impactElapsed = 0.0f;
                    particle.impacted = false;
                }
                continue;
            }
            particle.y += particle.speed * deltaSeconds / screenHeight;
            particle.x += particle.drift * deltaSeconds / screenWidth;
            if (particle.y >= system.groundY) {
                particle.y = system.groundY;
                particle.frame = 1;
                particle.impactElapsed = 0.0f;
                particle.impacted = true;
            }
        } else {
            particle.y += particle.speed * deltaSeconds / screenHeight;
            particle.x += particle.drift * deltaSeconds / screenWidth;

            if (particle.y > 1.05f) {
                particle.y = system.spawnY - 0.02f;
            }
        }
        if (particle.x > 1.02f) {
            particle.x = -0.02f;
        } else if (particle.x < -0.02f) {
            particle.x = 1.02f;
        }
    }
}

void updateWeatherSystemParallel(
    WeatherSystem& system,
    int screenWidth,
    int screenHeight,
    float deltaSeconds,
    float intensity
) {
    if (intensity <= 0.0f || screenWidth <= 0 || screenHeight <= 0) {
        updateWeatherSystem(system, screenWidth, screenHeight,
                            deltaSeconds, intensity);
        return;
    }

    #pragma omp parallel for schedule(static)
    for (std::ptrdiff_t rawIndex = 0;
         rawIndex < static_cast<std::ptrdiff_t>(system.particles.size());
         ++rawIndex) {
        const std::size_t index = static_cast<std::size_t>(rawIndex);
        WeatherParticle& particle = system.particles[index];
        if (system.accumulate) {
            if (particle.impacted) continue;
            particle.y += particle.speed * deltaSeconds / screenHeight;
            particle.x += particle.drift * deltaSeconds / screenWidth;
            if (particle.y >= system.groundY) {
                constexpr std::size_t moundCount = 24;
                const std::size_t mound = std::min(
                    moundCount - 1,
                    static_cast<std::size_t>(std::max(0.0f, particle.x) * moundCount)
                );
                std::size_t layer = 0;
                #pragma omp atomic capture
                layer = system.accumulationHeights[mound]++;
                particle.x = (static_cast<float>(mound) + 0.20f +
                              pseudoRandom(index, 73u) * 0.60f) / moundCount;
                particle.y = system.groundY -
                    static_cast<float>(std::min<std::size_t>(layer, 12)) * 0.007f;
                particle.frame = index % system.textures.size();
                particle.impacted = true;
            }
            continue;
        }
        if (system.impactAnimation) {
            if (particle.impacted) {
                particle.impactElapsed += deltaSeconds;
                if (particle.impactElapsed < 0.12f) particle.frame = 1;
                else if (particle.impactElapsed < 0.42f) particle.frame = 2;
                else {
                    particle.y = system.spawnY - 0.02f;
                    particle.frame = 0;
                    particle.impactElapsed = 0.0f;
                    particle.impacted = false;
                }
                continue;
            }
            particle.y += particle.speed * deltaSeconds / screenHeight;
            particle.x += particle.drift * deltaSeconds / screenWidth;
            if (particle.y >= system.groundY) {
                particle.y = system.groundY;
                particle.frame = 1;
                particle.impactElapsed = 0.0f;
                particle.impacted = true;
            }
        } else {
            particle.y += particle.speed * deltaSeconds / screenHeight;
            particle.x += particle.drift * deltaSeconds / screenWidth;
            if (particle.y > 1.05f) particle.y = system.spawnY - 0.02f;
        }
        if (particle.x > 1.02f) particle.x = -0.02f;
        else if (particle.x < -0.02f) particle.x = 1.02f;
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
