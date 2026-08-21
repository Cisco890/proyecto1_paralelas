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
};

struct WeatherSystem {
    std::vector<SDL_Texture*> textures;
    std::vector<WeatherParticle> particles;
    int textureWidth;
    int textureHeight;
    int scale;
    float spawnY;
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
void destroyWeatherSystem(WeatherSystem& system);
void updateWeatherSystem(
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
