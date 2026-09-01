#ifndef WILDLIFE_UPDATE_HPP
#define WILDLIFE_UPDATE_HPP

#include "bird.hpp"
#include "flying_animal.hpp"
#include "season.hpp"

#include <vector>

// Actualiza la fauna sin usar SDL desde los hilos de trabajo.
void updateWildlifeSequential(
    std::vector<Bird>& birds,
    FlyingAnimal& bee,
    FlyingAnimal& butterfly,
    const SeasonVisualState& season,
    bool isDaytime,
    int screenWidth,
    int screenHeight,
    int groundY,
    float deltaSeconds,
    Uint32 currentTicks
);

void updateWildlifeParallel(
    std::vector<Bird>& birds,
    FlyingAnimal& bee,
    FlyingAnimal& butterfly,
    const SeasonVisualState& season,
    bool isDaytime,
    int screenWidth,
    int screenHeight,
    int groundY,
    float deltaSeconds,
    Uint32 currentTicks
);

#endif
