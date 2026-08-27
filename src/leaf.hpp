#ifndef LEAF_HPP
#define LEAF_HPP

#include <SDL2/SDL.h>

#include <array>
#include <vector>

enum class LeafSeason { Spring, Autumn };

struct LeafTextures {
    std::array<std::array<SDL_Texture*, 3>, 3> frames{};
    int width = 0;
    int height = 0;
};

struct Leaf {
    float x;
    float y;
    float drift;
    float fallSpeed;
    float phase;
    bool falling;
    bool autumnLeaf;
    bool settled;
    Uint32 animationOffset;
    SDL_Rect dest{};
};

bool loadLeafTextures(SDL_Renderer* renderer, LeafTextures& textures);
void destroyLeafTextures(LeafTextures& textures);
std::vector<Leaf> createLeafField(std::size_t count);

void updateLeavesParallel(
    std::vector<Leaf>& leaves,
    const LeafTextures& textures,
    const SDL_Rect& treeDest,
    LeafSeason season,
    float deltaSeconds,
    Uint32 currentTicks
);

void renderLeaves(
    SDL_Renderer* renderer,
    const LeafTextures& textures,
    const std::vector<Leaf>& leaves,
    LeafSeason season,
    std::size_t visibleCount,
    Uint32 currentTicks,
    Uint8 opacity = 255
);

#endif
