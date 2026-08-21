#include "season.hpp"

#include <array>

namespace {
constexpr std::array<SeasonProfile, 4> profiles = {{
    {"Primavera", {135, 206, 235, 255}, {76, 140, 74, 255}, 36, 1.0f, 1.0f, 72, 0.65f, 0.0f},
    {"Verano",    {80, 180, 235, 255},  {64, 132, 62, 255},  6, 0.8f, 0.7f, 58, 0.0f, 0.0f},
    {"Otono",     {190, 145, 92, 255},  {125, 105, 54, 255}, 0, 0.0f, 0.2f, 20, 0.0f, 0.0f},
    {"Invierno",  {180, 205, 220, 255}, {220, 230, 235, 255}, 0, 0.0f, 0.0f,  0, 0.05f, 1.0f}
}};

Uint8 mixChannel(Uint8 from, Uint8 to, float progress) {
    return static_cast<Uint8>(from + (to - from) * progress);
}

SDL_Color mixColor(SDL_Color from, SDL_Color to, float progress) {
    return {
        mixChannel(from.r, to.r, progress),
        mixChannel(from.g, to.g, progress),
        mixChannel(from.b, to.b, progress),
        mixChannel(from.a, to.a, progress)
    };
}
}

SeasonSystem createSeasonSystem(Uint32 currentTicks) {
    // Cada estacion dura 30 segundos. Se puede cambiar desde un archivo de
    // configuracion en el futuro sin afectar a los elementos del escenario.
    return {Season::Spring, currentTicks, 30000, 8000};
}

void updateSeason(SeasonSystem& system, Uint32 currentTicks) {
    if (currentTicks - system.changedAt < system.durationMilliseconds) {
        return;
    }

    const int next = (static_cast<int>(system.current) + 1) % 4;
    setSeason(system, static_cast<Season>(next), currentTicks);
}

void setSeason(SeasonSystem& system, Season season, Uint32 currentTicks) {
    system.current = season;
    system.changedAt = currentTicks;
}

const SeasonProfile& getSeasonProfile(Season season) {
    return profiles[static_cast<std::size_t>(season)];
}

Season getNextSeason(Season season) {
    return static_cast<Season>((static_cast<int>(season) + 1) % 4);
}

SeasonVisualState getSeasonVisualState(
    const SeasonSystem& system,
    Uint32 currentTicks
) {
    const SeasonProfile& current = getSeasonProfile(system.current);
    const SeasonProfile& next = getSeasonProfile(getNextSeason(system.current));
    const Uint32 elapsed = currentTicks - system.changedAt;
    const Uint32 transitionStart =
        system.durationMilliseconds - system.transitionMilliseconds;
    float progress = 0.0f;

    if (elapsed > transitionStart) {
        progress = static_cast<float>(elapsed - transitionStart) /
                   static_cast<float>(system.transitionMilliseconds);
        if (progress > 1.0f) {
            progress = 1.0f;
        }
    }

    return {
        mixColor(current.skyColor, next.skyColor, progress),
        mixColor(current.groundColor, next.groundColor, progress),
        current.flowerCount +
            (static_cast<float>(next.flowerCount) - current.flowerCount) * progress,
        current.beePresence + (next.beePresence - current.beePresence) * progress,
        current.butterflyPresence +
            (next.butterflyPresence - current.butterflyPresence) * progress,
        current.grassCount +
            (static_cast<float>(next.grassCount) - current.grassCount) * progress,
        current.rainIntensity +
            (next.rainIntensity - current.rainIntensity) * progress,
        current.snowIntensity +
            (next.snowIntensity - current.snowIntensity) * progress,
        progress
    };
}
