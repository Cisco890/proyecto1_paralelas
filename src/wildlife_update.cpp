#include "wildlife_update.hpp"

#include <thread>

namespace {
void updateBirdRange(std::vector<Bird>& birds, int screenWidth, int screenHeight,
                     float deltaSeconds, std::size_t begin, std::size_t end) {
    for (std::size_t index = begin; index < end; ++index) {
        updateBird(birds[index], screenWidth, screenHeight, deltaSeconds);
    }
}

bool canShowAnimals(const SeasonVisualState& season, bool isDaytime) {
    return isDaytime && season.birdPresence > 0.01f;
}
}

void updateWildlifeSequential(
    std::vector<Bird>& birds, FlyingAnimal& bee, FlyingAnimal& butterfly,
    const SeasonVisualState& season, bool isDaytime, int screenWidth,
    int screenHeight, int groundY, float deltaSeconds, Uint32 currentTicks
) {
    if (!canShowAnimals(season, isDaytime)) return;
    updateBirdRange(birds, screenWidth, screenHeight, deltaSeconds, 0, birds.size());
    if (season.beePresence > 0.01f) {
        updateFlyingAnimal(bee, screenWidth, groundY, deltaSeconds, currentTicks);
    }
    if (season.butterflyPresence > 0.01f) {
        updateFlyingAnimal(butterfly, screenWidth, groundY, deltaSeconds, currentTicks);
    }
}

void updateWildlifeParallel(
    std::vector<Bird>& birds, FlyingAnimal& bee, FlyingAnimal& butterfly,
    const SeasonVisualState& season, bool isDaytime, int screenWidth,
    int screenHeight, int groundY, float deltaSeconds, Uint32 currentTicks
) {
    if (!canShowAnimals(season, isDaytime)) return;

    std::vector<std::thread> workers;
    if (!birds.empty()) {
        workers.emplace_back([&]() {
            updateBirdRange(birds, screenWidth, screenHeight, deltaSeconds, 0, birds.size());
        });
    }
    if (season.beePresence > 0.01f) {
        workers.emplace_back([&]() {
            updateFlyingAnimal(bee, screenWidth, groundY, deltaSeconds, currentTicks);
        });
    }
    if (season.butterflyPresence > 0.01f) {
        workers.emplace_back([&]() {
            updateFlyingAnimal(butterfly, screenWidth, groundY, deltaSeconds, currentTicks);
        });
    }
    for (std::thread& worker : workers) worker.join();
}
