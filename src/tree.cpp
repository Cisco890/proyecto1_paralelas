#include "tree.hpp"

#include <SDL2/SDL_image.h>

#include <iostream>

bool loadTree(
    Tree& tree,
    SDL_Renderer* renderer,
    const char* path
) {
    tree = {};
    tree.texture = IMG_LoadTexture(renderer, path);

    if (tree.texture == nullptr) {
        std::cerr
            << "Error cargando el arbol: "
            << IMG_GetError()
            << std::endl;
        return false;
    }

    if (SDL_QueryTexture(
            tree.texture,
            nullptr,
            nullptr,
            &tree.width,
            &tree.height
        ) != 0) {
        std::cerr
            << "Error obteniendo dimensiones del arbol: "
            << SDL_GetError()
            << std::endl;

        destroyTree(tree);
        return false;
    }

    std::cout
        << "Tamaño del arbol: "
        << tree.width
        << "x"
        << tree.height
        << std::endl;

    return true;
}

void destroyTree(Tree& tree) {
    if (tree.texture != nullptr) {
        SDL_DestroyTexture(tree.texture);
        tree.texture = nullptr;
    }
}

void updateTreePosition(
    Tree& tree,
    int screenWidth,
    int groundY
) {
    tree.dest = {
        (screenWidth - tree.width) / 2,
        groundY - tree.height,
        tree.width,
        tree.height
    };
}

void renderTree(SDL_Renderer* renderer, const Tree& tree) {
    SDL_RenderCopy(renderer, tree.texture, nullptr, &tree.dest);
}
