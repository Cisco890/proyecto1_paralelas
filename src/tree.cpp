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
    SDL_SetTextureBlendMode(tree.texture, SDL_BLENDMODE_BLEND);

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

bool loadTreeNight(
    Tree& tree,
    SDL_Renderer* renderer,
    const char* path
) {
    tree.nightTexture = IMG_LoadTexture(renderer, path);
    if (tree.nightTexture == nullptr) {
        std::cerr
            << "Error cargando el arbol nocturno: "
            << IMG_GetError()
            << std::endl;
        return false;
    }
    SDL_SetTextureBlendMode(tree.nightTexture, SDL_BLENDMODE_BLEND);

    if (SDL_QueryTexture(
            tree.nightTexture,
            nullptr,
            nullptr,
            &tree.nightWidth,
            &tree.nightHeight
        ) != 0) {
        std::cerr
            << "Error obteniendo dimensiones del arbol nocturno: "
            << SDL_GetError()
            << std::endl;
        SDL_DestroyTexture(tree.nightTexture);
        tree.nightTexture = nullptr;
        return false;
    }

    return true;
}

void destroyTree(Tree& tree) {
    if (tree.texture != nullptr) {
        SDL_DestroyTexture(tree.texture);
        tree.texture = nullptr;
    }
    if (tree.nightTexture != nullptr) {
        SDL_DestroyTexture(tree.nightTexture);
        tree.nightTexture = nullptr;
    }
}

void updateTreePosition(
    Tree& tree,
    int screenWidth,
    int groundY
) {
    tree.dest = {
        (screenWidth - tree.width) / 2,
        groundY - tree.height + tree.groundOffset,
        tree.width,
        tree.height
    };
    tree.nightDest = {
        (screenWidth - tree.nightWidth) / 2,
        groundY - tree.nightHeight + tree.groundOffset,
        tree.nightWidth,
        tree.nightHeight
    };
}

void renderTree(SDL_Renderer* renderer, const Tree& tree, bool night,
                Uint8 opacity) {
    if (opacity == 0) return;
    SDL_Texture* texture = night && tree.nightTexture != nullptr
        ? tree.nightTexture
        : tree.texture;
    const SDL_Rect& destination = night && tree.nightTexture != nullptr
        ? tree.nightDest
        : tree.dest;
    SDL_SetTextureAlphaMod(texture, opacity);
    SDL_RenderCopy(renderer, texture, nullptr, &destination);
    SDL_SetTextureAlphaMod(texture, 255);
}
