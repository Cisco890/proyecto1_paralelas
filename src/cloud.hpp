#ifndef CLOUD_HPP
#define CLOUD_HPP

#include <SDL2/SDL.h>

#include <array>
#include <vector>

struct CloudTextures {
    std::array<SDL_Texture*, 3> day;
    std::array<SDL_Texture*, 3> night;
    std::array<SDL_Texture*, 3> rain;
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
    const std::array<const char*, 3>& dayPaths,
    const std::array<const char*, 3>& nightPaths,
    const std::array<const char*, 3>& rainPaths
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
    bool night,
    bool stormy,
    Uint8 opacity = 255
);

#endif
