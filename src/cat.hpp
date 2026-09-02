#ifndef CAT_HPP
#define CAT_HPP

#include <SDL2/SDL.h>

#include <array>

struct Cat {
    std::array<SDL_Texture*, 3> frames{};
    std::array<SDL_Texture*, 3> sleepingFrames{};
    int width = 0;
    int height = 0;
    float horizontalPosition = 0.72f;
    SDL_Rect dest{};
};

bool loadCat(Cat& cat, SDL_Renderer* renderer);
void destroyCat(Cat& cat);
void updateCat(Cat& cat, int screenWidth, int groundY);
void renderCat(SDL_Renderer* renderer, const Cat& cat, Uint32 currentTicks,
               bool sleeping, Uint8 opacity = 255);

#endif
