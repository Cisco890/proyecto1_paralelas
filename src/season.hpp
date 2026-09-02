#ifndef SEASON_HPP
#define SEASON_HPP

#include <SDL2/SDL.h>

#include <cstddef>

enum class Season {
    Spring,
    Summer,
    Autumn,
    Winter
};

struct SeasonProfile {
    const char* name;
    SDL_Color skyColor;
    SDL_Color groundColor;
    std::size_t flowerCount;
    float beePresence;
    float butterflyPresence;
    std::size_t grassCount;
    float rainIntensity;
    float snowIntensity;
};

struct SeasonVisualState {
    SDL_Color skyColor;
    SDL_Color groundColor;
    float flowerCount;
    float beePresence;
    float butterflyPresence;
    float grassCount;
    float rainIntensity;
    float snowIntensity;
    float transitionProgress;
};

struct SeasonSystem {
    Season current;
    Uint32 changedAt;
    Uint32 durationMilliseconds;
    Uint32 transitionMilliseconds;
};

SeasonSystem createSeasonSystem(Uint32 currentTicks);
void updateSeason(SeasonSystem& system, Uint32 currentTicks);
void setSeason(SeasonSystem& system, Season season, Uint32 currentTicks);
const SeasonProfile& getSeasonProfile(Season season);
Season getNextSeason(Season season);
SeasonVisualState getSeasonVisualState(
    const SeasonSystem& system,
    Uint32 currentTicks
);

#endif
