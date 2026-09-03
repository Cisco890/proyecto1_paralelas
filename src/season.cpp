#include "season.hpp"

#include "celestial_body.hpp"

#include <array>

namespace {
constexpr std::array<SeasonProfile, 4> profiles = {{
    {"Primavera", {135, 206, 235, 255}, {76, 140, 74, 255}, 36, 1.0f, 1.0f, 0.9f, 72, 0.0f, 0.0f, 0.92f, 0.95f, 0.75f},
    {"Verano",    {42, 174, 245, 255},  {57, 145, 48, 255}, 12, 0.8f, 0.7f, 1.0f, 58, 0.0f, 0.0f, 1.00f, 1.35f, 0.35f},
    {"Otono",     {190, 145, 92, 255},  {125, 105, 54, 255}, 0, 0.0f, 0.1f, 0.45f, 20, 0.0f, 0.0f, 0.62f, 0.75f, 0.65f},
    {"Invierno",  {180, 205, 220, 255}, {180, 205, 245, 255}, 0, 0.0f, 0.0f, 0.08f, 0, 1.0f, 0.75f, 0.40f, 0.60f, 0.90f}
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
    constexpr Uint32 sunriseOffset = DAY_NIGHT_CYCLE_MILLISECONDS * 3 / 4;
    const Uint32 base = currentTicks - (currentTicks % DAY_NIGHT_CYCLE_MILLISECONDS);
    Uint32 sunrise = base + sunriseOffset;
    if (currentTicks < sunrise) sunrise -= DAY_NIGHT_CYCLE_MILLISECONDS;
    return {Season::Spring, sunrise, DAY_NIGHT_CYCLE_MILLISECONDS, 15000};
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

    const float seasonProgress = std::min(
        1.0f, static_cast<float>(elapsed) / static_cast<float>(system.durationMilliseconds)
    );
    const float flowerGrowth = system.current == Season::Spring
        ? std::min(1.0f, seasonProgress * 1.8f) : 1.0f;
    const float springArrival = system.current == Season::Spring
        ? std::min(1.0f, static_cast<float>(elapsed) / 12000.0f) : 1.0f;
    const float rainEntry = std::min(1.0f, static_cast<float>(elapsed) / 5000.0f);
    const float rainProgress = system.current == Season::Winter
        ? rainEntry * (1.0f - progress) : 0.0f;

    return {
        mixColor(current.skyColor, next.skyColor, progress),
        mixColor(current.groundColor, next.groundColor, progress),
        system.current == Season::Spring
            ? static_cast<float>(current.flowerCount) * flowerGrowth
            : static_cast<float>(current.flowerCount),
        current.beePresence * springArrival,
        current.butterflyPresence * springArrival,
        current.birdPresence * springArrival,
        static_cast<float>(current.grassCount) * springArrival,
        current.rainIntensity * rainProgress,
        current.snowIntensity * rainProgress,
        current.sunlight,
        current.sunScale,
        current.cloudCoverage,
        flowerGrowth,
        system.current == Season::Spring ? flowerGrowth : 0.0f,
        rainProgress,
        seasonProgress,
        springArrival,
        progress
    };
}
