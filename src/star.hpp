#ifndef STAR_HPP
#define STAR_HPP

#include <SDL2/SDL.h>

#include <cstddef>
#include <vector>

struct StarTextures {
    std::vector<SDL_Texture*> frames;
    int width;
    int height;
};

struct Star {
    float horizontalPosition;
    float verticalPosition;
    float visualSize;
    Uint32 animationOffset;
};

// La textura es opcional. Si no se carga, las estrellas se dibujan como
// puntos blancos. Se pueden proporcionar varios frames para animarlas.
bool loadStarTextures(
    StarTextures& textures,
    SDL_Renderer* renderer,
    const std::vector<const char*>& framePaths
);

void destroyStarTextures(StarTextures& textures);

std::vector<Star> createStarField(std::size_t count);

void renderStarField(
    SDL_Renderer* renderer,
    const StarTextures& textures,
    const std::vector<Star>& stars,
    int screenWidth,
    int skyHeight,
    Uint32 currentTicks,
    float visibleCount
);

#endif
