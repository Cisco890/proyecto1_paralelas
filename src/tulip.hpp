#ifndef TULIP_HPP
#define TULIP_HPP

#include "flower.hpp"

struct TulipTextures {
    FlowerTextures red;
    FlowerTextures orange;
};

struct Tulip {
    Flower placement;
    bool orange;
};

bool loadTulipTextures(TulipTextures& textures, SDL_Renderer* renderer);
void destroyTulipTextures(TulipTextures& textures);
std::vector<Tulip> createTulipField(std::size_t count);
void updateTulipPositionsParallel(std::vector<Tulip>& tulips,
                                  const TulipTextures& textures,
                                  int screenWidth, int screenHeight, int groundY);
void renderTulip(SDL_Renderer* renderer, const TulipTextures& textures,
                 const Tulip& tulip, Uint32 currentTicks, Uint8 opacity = 255);

#endif
