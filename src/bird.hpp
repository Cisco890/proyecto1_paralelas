#ifndef BIRD_HPP
#define BIRD_HPP

#include <SDL2/SDL.h>

#include <array>

// Elemento dinamico: vuela y actualiza su ubicacion.
struct Bird {
    float x;
    float y;
    float velocityX;
    float velocityY;
    int width;
    int height;
    SDL_Texture* texture;
    std::array<SDL_Texture*, 3> animationFrames{};
    Uint32 animationOffset = 0;
};

// Carga el PNG y deja el pajaro listo para usarse.
// Retorna false si falla la carga.
bool loadBird(
    Bird& bird,
    SDL_Renderer* renderer,
    const char* path,
    float startX,
    float startY
);

bool loadAnimatedBird(
    Bird& bird,
    SDL_Renderer* renderer,
    const std::array<const char*, 3>& paths,
    float startX,
    float startY,
    Uint32 animationOffset = 0
);

void destroyBird(Bird& bird);

// Determina la siguiente ubicacion del pajaro.
void updateBird(
    Bird& bird,
    int screenWidth,
    int screenHeight,
    float deltaSeconds
);

void renderBird(SDL_Renderer* renderer, const Bird& bird, Uint8 opacity = 255);

#endif
