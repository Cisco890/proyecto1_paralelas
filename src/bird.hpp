#ifndef BIRD_HPP
#define BIRD_HPP

#include <SDL2/SDL.h>

// Elemento dinamico: vuela y actualiza su ubicacion.
struct Bird {
    float x;
    float y;
    float velocityX;
    float velocityY;
    int width;
    int height;
    SDL_Texture* texture;
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

void destroyBird(Bird& bird);

// Determina la siguiente ubicacion del pajaro.
void updateBird(
    Bird& bird,
    int screenWidth,
    int screenHeight,
    float deltaSeconds
);

void renderBird(SDL_Renderer* renderer, const Bird& bird);

#endif
