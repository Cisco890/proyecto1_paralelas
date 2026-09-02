#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <SDL2/SDL.h>

#include <vector>

struct WeatherParticle {
    float x;
    float y;
    float speed;
    float drift;
    std::size_t frame;
    float impactElapsed;
    bool impacted;
};

struct WeatherSystem {
    std::vector<SDL_Texture*> textures;
    std::vector<WeatherParticle> particles;
    int textureWidth;
    int textureHeight;
    int scale;
    float spawnY;
    float groundY;
    bool impactAnimation;
    bool accumulate;
    std::vector<std::size_t> accumulationHeights;
};

bool loadWeatherSystem(
    WeatherSystem& system,
    SDL_Renderer* renderer,
    const std::vector<const char*>& paths,
    std::size_t particleCount,
    int scale,
    float minimumSpeed,
    float speedVariation
);
void setWeatherSpawnHeight(WeatherSystem& system, float normalizedY);
void setWeatherImpactAnimation(WeatherSystem& system, bool enabled,
                               float groundNormalized = 0.85f);
void setWeatherAccumulation(WeatherSystem& system, bool enabled,
                            float groundNormalized = 0.85f);
void destroyWeatherSystem(WeatherSystem& system);
void updateWeatherSystem(
    WeatherSystem& system,
    int screenWidth,
    int screenHeight,
    float deltaSeconds,
    float intensity
);
void updateWeatherSystemParallel(
    WeatherSystem& system,
    int screenWidth,
    int screenHeight,
    float deltaSeconds,
    float intensity
);
void renderWeatherSystem(
    SDL_Renderer* renderer,
    const WeatherSystem& system,
    float intensity
);

#endif
