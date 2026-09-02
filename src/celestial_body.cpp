#include "celestial_body.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
constexpr float pi = 3.14159265359f;

float getArcProgress(CelestialBodyType type, Uint32 currentTicks) {
    const float cycleProgress =
        static_cast<float>(currentTicks % DAY_NIGHT_CYCLE_MILLISECONDS) /
        static_cast<float>(DAY_NIGHT_CYCLE_MILLISECONDS);

    // El sol sale al 75% del ciclo y se oculta al 25% del siguiente. La luna
    // ocupa la mitad opuesta. Asi ambos coinciden con el brillo del cielo.
    const float arcStart = type == CelestialBodyType::Sun ? 0.75f : 0.25f;
    float elapsed = cycleProgress - arcStart;
    if (elapsed < 0.0f) elapsed += 1.0f;

    return elapsed < 0.5f ? elapsed / 0.5f : -1.0f;
}

void fillCircle(
    SDL_Renderer* renderer,
    int centerX,
    int centerY,
    int radius
) {
    for (int y = -radius; y <= radius; ++y) {
        const int halfWidth = static_cast<int>(
            std::sqrt(static_cast<float>(radius * radius - y * y))
        );
        SDL_RenderDrawLine(
            renderer,
            centerX - halfWidth,
            centerY + y,
            centerX + halfWidth,
            centerY + y
        );
    }
}
}

CelestialBody createCelestialBody(
    CelestialBodyType type,
    SDL_Color fallbackColor
) {
    return {type, fallbackColor, nullptr, {}, false};
}

bool loadCelestialBodyTexture(
    CelestialBody& body,
    SDL_Renderer* renderer,
    const char* assetPath
) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, assetPath);
    if (texture == nullptr) {
        std::cerr << "Error cargando cuerpo celeste: "
                  << IMG_GetError() << std::endl;
        return false;
    }

    if (body.texture != nullptr) SDL_DestroyTexture(body.texture);
    body.texture = texture;
    return true;
}

void destroyCelestialBody(CelestialBody& body) {
    if (body.texture != nullptr) {
        SDL_DestroyTexture(body.texture);
        body.texture = nullptr;
    }
}

void updateCelestialBody(
    CelestialBody& body,
    int screenWidth,
    int screenHeight,
    int groundY,
    Uint32 currentTicks
) {
    if (screenWidth <= 0 || screenHeight <= 0) {
        body.visible = false;
        return;
    }

    const float progress = getArcProgress(body.type, currentTicks);
    body.visible = progress >= 0.0f;
    if (!body.visible) return;

    const float diameter = std::clamp(
        static_cast<float>(std::min(screenWidth, screenHeight)) * 0.09f,
        48.0f,
        110.0f
    );
    const float radius = diameter * 0.5f;
    const float centerX = -radius +
        (static_cast<float>(screenWidth) + diameter) * progress;
    const float horizonY = static_cast<float>(groundY) - radius * 0.20f;
    const float arcHeight = static_cast<float>(screenHeight) * 0.62f;
    const float centerY = horizonY - std::sin(progress * pi) * arcHeight;

    body.destination = {
        centerX - radius,
        centerY - radius,
        diameter,
        diameter
    };
}

void renderCelestialBody(
    SDL_Renderer* renderer,
    const CelestialBody& body
) {
    if (!body.visible) return;

    if (body.texture != nullptr) {
        SDL_RenderCopyF(renderer, body.texture, nullptr, &body.destination);
        return;
    }

    SDL_SetRenderDrawColor(
        renderer,
        body.fallbackColor.r,
        body.fallbackColor.g,
        body.fallbackColor.b,
        body.fallbackColor.a
    );
    const int radius = static_cast<int>(body.destination.w * 0.5f);
    fillCircle(
        renderer,
        static_cast<int>(body.destination.x + body.destination.w * 0.5f),
        static_cast<int>(body.destination.y + body.destination.h * 0.5f),
        radius
    );
}
