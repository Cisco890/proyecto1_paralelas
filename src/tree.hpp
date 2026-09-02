#ifndef TREE_HPP
#define TREE_HPP

#include <SDL2/SDL.h>

// Protagonista estatico del escenario.
struct Tree {
    int width;
    int height;
    int groundOffset;
    SDL_Texture* texture;
    SDL_Texture* nightTexture;
    int nightWidth;
    int nightHeight;
    SDL_Rect dest;
    SDL_Rect nightDest;
};

// Carga el PNG del arbol. Retorna false si falla.
bool loadTree(
    Tree& tree,
    SDL_Renderer* renderer,
    const char* path
);

bool loadTreeNight(
    Tree& tree,
    SDL_Renderer* renderer,
    const char* path
);

void destroyTree(Tree& tree);

// Recalcula la posicion (centrado, apoyado en el suelo).
void updateTreePosition(
    Tree& tree,
    int screenWidth,
    int groundY
);

void renderTree(SDL_Renderer* renderer, const Tree& tree, bool night = false,
                Uint8 opacity = 255);

#endif
