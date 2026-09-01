#ifndef CLOUD_HPP
#define CLOUD_HPP

#include <SDL2/SDL.h>

#include <array>
#include <vector>

struct CloudTextures {
    std::array<SDL_Texture*, 2> normal;
    std::array<SDL_Texture*, 2> rain;
    int width;
    int height;
};

struct Cloud {
    SDL_Rect dest;
    float x;
    float y;
    float speed;
    Uint32 animationOffset;
    std::size_t variant;
};

bool loadCloudTextures(
    CloudTextures& textures,
    SDL_Renderer* renderer,
    const char* normalFirst,
    const char* normalSecond,
    const char* rainFirst,
    const char* rainSecond
);
void destroyCloudTextures(CloudTextures& textures);
std::vector<Cloud> createCloudField(std::size_t count);
void updateCloudPositionsSequential(
    std::vector<Cloud>& clouds,
    const CloudTextures& textures,
    int screenWidth,
    int screenHeight,
    float deltaSeconds
);
void updateCloudPositionsParallel(
    std::vector<Cloud>& clouds,
    const CloudTextures& textures,
    int screenWidth,
    int screenHeight,
    float deltaSeconds
);
void renderCloud(
    SDL_Renderer* renderer,
    const CloudTextures& textures,
    const Cloud& cloud,
    bool stormy,
    Uint8 opacity = 255,
    int verticalOffset = 0
);

#endif
