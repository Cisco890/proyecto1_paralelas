#ifndef CELESTIAL_BODY_HPP
#define CELESTIAL_BODY_HPP

#include <SDL2/SDL.h>

constexpr Uint32 DAY_NIGHT_CYCLE_MILLISECONDS = 60000;

enum class CelestialBodyType {
    Sun,
    Moon
};

struct CelestialBody {
    CelestialBodyType type;
    SDL_Color fallbackColor;
    SDL_Texture* texture;
    SDL_FRect destination;
    bool visible;
};

CelestialBody createCelestialBody(
    CelestialBodyType type,
    SDL_Color fallbackColor
);

// Esta funcion es opcional: mientras no haya un asset, el cuerpo celeste se
// dibuja como un circulo. Al cargar una textura, el movimiento no cambia.
bool loadCelestialBodyTexture(
    CelestialBody& body,
    SDL_Renderer* renderer,
    const char* assetPath
);

void destroyCelestialBody(CelestialBody& body);

void updateCelestialBody(
    CelestialBody& body,
    int screenWidth,
    int screenHeight,
    int groundY,
    Uint32 currentTicks,
    float sizeScale = 1.0f
);

void renderCelestialBody(
    SDL_Renderer* renderer,
    const CelestialBody& body,
    Uint8 opacity = 255
);

#endif
