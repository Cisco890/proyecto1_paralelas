#include "wildlife_update.hpp"

#include <omp.h>

namespace {
bool active(const SeasonVisualState& season, bool daytime) {
    return daytime && season.birdPresence > 0.01f;
}
}

void updateWildlifeSequential(std::vector<Bird>& birds, FlyingAnimal& bee,
                              FlyingAnimal& butterfly, const SeasonVisualState& season,
                              bool daytime, int screenWidth, int screenHeight,
                              int groundY, float deltaSeconds, Uint32 currentTicks) {
    if (!active(season, daytime)) return;
    for (Bird& bird : birds) updateBird(bird, screenWidth, screenHeight, deltaSeconds);
    if (season.beePresence > 0.01f)
        updateFlyingAnimal(bee, screenWidth, groundY, deltaSeconds, currentTicks);
    if (season.butterflyPresence > 0.01f)
        updateFlyingAnimal(butterfly, screenWidth, groundY, deltaSeconds, currentTicks);
}

void updateWildlifeParallel(std::vector<Bird>& birds, FlyingAnimal& bee,
                            FlyingAnimal& butterfly, const SeasonVisualState& season,
                            bool daytime, int screenWidth, int screenHeight,
                            int groundY, float deltaSeconds, Uint32 currentTicks) {
    if (!active(season, daytime)) return;
    // Hay solo tres aves y dos animales: abrir una region OpenMP por cuadro
    // cuesta mas que actualizar sus trayectorias.
    if (birds.size() < 64) {
        updateWildlifeSequential(birds, bee, butterfly, season, daytime,
                                 screenWidth, screenHeight, groundY,
                                 deltaSeconds, currentTicks);
        return;
    }
    #pragma omp parallel sections
    {
        #pragma omp section
        for (Bird& bird : birds) updateBird(bird, screenWidth, screenHeight, deltaSeconds);
        #pragma omp section
        if (season.beePresence > 0.01f)
            updateFlyingAnimal(bee, screenWidth, groundY, deltaSeconds, currentTicks);
        #pragma omp section
        if (season.butterflyPresence > 0.01f)
            updateFlyingAnimal(butterfly, screenWidth, groundY, deltaSeconds, currentTicks);
    }
}
