#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "src/bird.hpp"
#include "src/celestial_body.hpp"
#include "src/cloud.hpp"
#include "src/flower.hpp"
#include "src/flying_animal.hpp"
#include "src/grass.hpp"
#include "src/season.hpp"
#include "src/tree.hpp"
#include "src/tulip.hpp"
#include "src/weather.hpp"

namespace {
float getDaylightFactor(Uint32 ticks) {
    constexpr float twoPi = 6.28318530718f;
    const float phase =
        static_cast<float>(ticks % DAY_NIGHT_CYCLE_MILLISECONDS) /
        static_cast<float>(DAY_NIGHT_CYCLE_MILLISECONDS);
    // Comienza de día, llega a la noche a mitad del ciclo y vuelve a amanecer.
    const float daylight = 0.5f + 0.5f * std::sin(phase * twoPi + 1.57079632679f);
    return 0.30f + daylight * 0.70f;
}
}

int main() {
    // Configuracion para Pixel Art
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr
            << "Error iniciando SDL: "
            << SDL_GetError()
            << std::endl;
        return 1;
    }

    // Inicializar SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr
            << "Error iniciando SDL_image: "
            << IMG_GetError()
            << std::endl;
        SDL_Quit();
        return 1;
    }

    // Obtener tamaño disponible del escritorio
    SDL_Rect displayBounds;

    if (SDL_GetDisplayUsableBounds(0, &displayBounds) != 0) {
        std::cerr
            << "Error obteniendo tamaño del escritorio: "
            << SDL_GetError()
            << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    std::cout
        << "Escritorio disponible: "
        << displayBounds.w
        << "x"
        << displayBounds.h
        << std::endl;

    // Crear ventana normal
    //
    // IMPORTANTE:
    // No usamos FULLSCREEN para conservar:
    // - Barra de titulo
    // - Minimizar
    // - Maximizar / restaurar
    // - Cerrar
    //
    SDL_Window* window = SDL_CreateWindow(
        "Arbol Estacional - Screen Saver",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        displayBounds.w,
        displayBounds.h,
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_MAXIMIZED
    );

    if (window == nullptr) {
        std::cerr
            << "Error creando ventana: "
            << SDL_GetError()
            << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC
    );

    if (renderer == nullptr) {
        std::cerr
            << "Error creando renderer: "
            << SDL_GetError()
            << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --------------------------------------
    // Elementos a renderizar (memoria)
    // --------------------------------------
    Tree tree;
    CelestialBody sun = createCelestialBody(
        CelestialBodyType::Sun,
        {255, 221, 64, 255}
    );
    CelestialBody moon = createCelestialBody(
        CelestialBodyType::Moon,
        {245, 245, 235, 255}
    );

    if (!loadTree(tree, renderer, "assets/tree/tree.png")) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    std::vector<Bird> birds;
    Bird bird;

    if (!loadBird(
            bird,
            renderer,
            "assets/animals/bird.png",
            50.0f,
            static_cast<float>(displayBounds.h) * 0.20f
        )) {
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    birds.push_back(bird);

    FlowerTextures flowerTextures;

    if (!loadFlowerTextures(
            flowerTextures,
            renderer,
            "assets/flowers/pink_flower_frame_1.png",
            "assets/flowers/pink_flower_frame_2.png"
        )) {
        for (Bird& currentBird : birds) {
            destroyBird(currentBird);
        }
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FlyingAnimal bee;
    if (!loadFlyingAnimal(
            bee,
            renderer,
            {{
                "assets/animals/bee_frame_1.png",
                "assets/animals/bee_frame_2.png",
                "assets/animals/bee_frame_3.png"
            }},
            100.0f,
            static_cast<float>(displayBounds.h) * 0.68f,
            75.0f,
            0.0f
        )) {
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) {
            destroyBird(currentBird);
        }
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    FlyingAnimal butterfly;
    if (!loadFlyingAnimal(
            butterfly,
            renderer,
            {{
                "assets/animals/purple_butterfly_frame_1.png",
                "assets/animals/purple_butterfly_frame_2.png",
                "assets/animals/purple_butterfly_frame_3.png"
            }},
            static_cast<float>(displayBounds.w) * 0.55f,
            static_cast<float>(displayBounds.h) * 0.72f,
            48.0f,
            2.1f
        )) {
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) {
            destroyBird(currentBird);
        }
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    GrassTextures grassTextures;
    if (!loadGrassTextures(
            grassTextures,
            renderer,
            {{
                "assets/grass/spring_grass_frame_1.png",
                "assets/grass/spring_grass_frame_2.png",
                "assets/grass/spring_grass_frame_3.png"
            }}
        )) {
        destroyFlyingAnimal(butterfly);
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) {
            destroyBird(currentBird);
        }
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    WeatherSystem rain;
    if (!loadWeatherSystem(
            rain,
            renderer,
            {"assets/rain/raindrop.png"},
            220,
            2,
            420.0f,
            180.0f
        )) {
        destroyGrassTextures(grassTextures);
        destroyFlyingAnimal(butterfly);
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) {
            destroyBird(currentBird);
        }
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    // La lluvia comienza debajo de la base visible de las nubes.
    setWeatherSpawnHeight(rain, 0.20f);

    WeatherSystem snow;
    if (!loadWeatherSystem(
            snow,
            renderer,
            {
                "assets/snow/snowflake_1.png",
                "assets/snow/snowflake_2.png",
                "assets/snow/snowflake_3.png"
            },
            150,
            2,
            55.0f,
            65.0f
        )) {
        destroyWeatherSystem(rain);
        destroyGrassTextures(grassTextures);
        destroyFlyingAnimal(butterfly);
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) {
            destroyBird(currentBird);
        }
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    CloudTextures cloudTextures;
    if (!loadCloudTextures(
            cloudTextures, renderer,
            "assets/clouds/normal_cloud_1.png",
            "assets/clouds/normal_cloud_2.png",
            "assets/clouds/rain_cloud_1.png",
            "assets/clouds/rain_cloud_2.png"
        )) {
        destroyWeatherSystem(snow);
        destroyWeatherSystem(rain);
        destroyGrassTextures(grassTextures);
        destroyFlyingAnimal(butterfly);
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) destroyBird(currentBird);
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    TulipTextures tulipTextures;
    if (!loadTulipTextures(tulipTextures, renderer)) {
        destroyCloudTextures(cloudTextures);
        destroyWeatherSystem(snow);
        destroyWeatherSystem(rain);
        destroyGrassTextures(grassTextures);
        destroyFlyingAnimal(butterfly);
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) destroyBird(currentBird);
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    Uint32 previousTicks = SDL_GetTicks();
    SeasonSystem seasonSystem = createSeasonSystem(previousTicks);
    std::vector<Flower> flowers = createFlowerField(36);
    std::vector<Tulip> tulips = createTulipField(20);
    std::vector<Cloud> clouds = createCloudField(4);
    std::vector<GrassBlade> grass = createGrassField(72);
    int previousFlowerScreenWidth = -1;
    int previousFlowerScreenHeight = -1;
    int previousFlowerGroundY = -1;
    int previousTulipScreenWidth = -1;
    int previousTulipScreenHeight = -1;
    int previousTulipGroundY = -1;

    while (running) {
        Uint32 currentTicks = SDL_GetTicks();
        float deltaSeconds =
            (currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;

        if (deltaSeconds > 0.05f) {
            deltaSeconds = 0.05f;
        }

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (
                event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE
            ) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1:
                        setSeason(seasonSystem, Season::Spring, currentTicks);
                        break;
                    case SDLK_2:
                        setSeason(seasonSystem, Season::Summer, currentTicks);
                        break;
                    case SDLK_3:
                        setSeason(seasonSystem, Season::Autumn, currentTicks);
                        break;
                    case SDLK_4:
                        setSeason(seasonSystem, Season::Winter, currentTicks);
                        break;
                    default:
                        break;
                }
            }
        }

        updateSeason(seasonSystem, currentTicks);
        const SeasonVisualState seasonVisual =
            getSeasonVisualState(seasonSystem, currentTicks);

        int screenWidth = 0;
        int screenHeight = 0;

        SDL_GetRendererOutputSize(
            renderer,
            &screenWidth,
            &screenHeight
        );

        for (Bird& currentBird : birds) {
            updateBird(
                currentBird,
                screenWidth,
                screenHeight,
                deltaSeconds
            );
        }

        // El suelo comienza al 85% de la altura
        int groundY = static_cast<int>(screenHeight * 0.85);

        updateCelestialBody(
            sun, screenWidth, screenHeight, groundY, currentTicks
        );
        updateCelestialBody(
            moon, screenWidth, screenHeight, groundY, currentTicks
        );

        updateFlyingAnimal(bee, screenWidth, groundY, deltaSeconds, currentTicks);
        updateFlyingAnimal(
            butterfly, screenWidth, groundY, deltaSeconds, currentTicks
        );
        updateWeatherSystem(
            rain,
            screenWidth,
            screenHeight,
            deltaSeconds,
            seasonVisual.rainIntensity
        );
        updateWeatherSystem(
            snow,
            screenWidth,
            screenHeight,
            deltaSeconds,
            seasonVisual.snowIntensity
        );
        updateCloudPositionsParallel(
            clouds, cloudTextures, screenWidth, screenHeight, deltaSeconds
        );

        updateTreePosition(tree, screenWidth, groundY);

        // Cada flor actualiza su posicion en paralelo cuando cambia la ventana.
        if (
            screenWidth != previousFlowerScreenWidth ||
            screenHeight != previousFlowerScreenHeight ||
            groundY != previousFlowerGroundY
        ) {
            updateFlowerPositionsParallel(
                flowers, flowerTextures, screenWidth, screenHeight, groundY
            );

            previousFlowerScreenWidth = screenWidth;
            previousFlowerScreenHeight = screenHeight;
            previousFlowerGroundY = groundY;
        }

        if (
            screenWidth != previousTulipScreenWidth ||
            screenHeight != previousTulipScreenHeight ||
            groundY != previousTulipGroundY
        ) {
            updateTulipPositionsParallel(
                tulips, tulipTextures, screenWidth, screenHeight, groundY
            );
            previousTulipScreenWidth = screenWidth;
            previousTulipScreenHeight = screenHeight;
            previousTulipGroundY = groundY;
        }

        // Cielo
        const float daylight = getDaylightFactor(currentTicks);
        SDL_Color skyColor = seasonVisual.skyColor;
        const float rainDarkening = std::min(1.0f, seasonVisual.rainIntensity) * 0.18f;
        const float skyBrightness = daylight * (1.0f - rainDarkening);
        skyColor.r = static_cast<Uint8>(skyColor.r * skyBrightness);
        skyColor.g = static_cast<Uint8>(skyColor.g * skyBrightness);
        skyColor.b = static_cast<Uint8>(skyColor.b * skyBrightness);

        SDL_SetRenderDrawColor(
            renderer,
            skyColor.r,
            skyColor.g,
            skyColor.b,
            skyColor.a
        );
        SDL_RenderClear(renderer);

        // Se dibujan al fondo para que las nubes y el arbol puedan ocultarlos.
        renderCelestialBody(renderer, sun);
        renderCelestialBody(renderer, moon);

        // Suelo temporal
        SDL_Rect ground = {
            0,
            groundY,
            screenWidth,
            screenHeight - groundY
        };

        SDL_Color groundColor = seasonVisual.groundColor;
        groundColor.r = static_cast<Uint8>(groundColor.r * daylight);
        groundColor.g = static_cast<Uint8>(groundColor.g * daylight);
        groundColor.b = static_cast<Uint8>(groundColor.b * daylight);

        SDL_SetRenderDrawColor(
            renderer,
            groundColor.r,
            groundColor.g,
            groundColor.b,
            groundColor.a
        );
        SDL_RenderFillRect(renderer, &ground);

        renderGrassField(
            renderer,
            grassTextures,
            grass,
            screenWidth,
            screenHeight,
            groundY,
            currentTicks,
            seasonVisual.grassCount
        );

        const std::size_t completeFlowers = static_cast<std::size_t>(
            std::floor(seasonVisual.flowerCount)
        );
        for (std::size_t index = 0; index < completeFlowers; ++index) {
            renderFlower(renderer, flowerTextures, flowers[index], currentTicks);
        }

        const float partialFlower = seasonVisual.flowerCount - completeFlowers;
        if (partialFlower > 0.0f && completeFlowers < flowers.size()) {
            renderFlower(
                renderer,
                flowerTextures,
                flowers[completeFlowers],
                currentTicks,
                static_cast<Uint8>(partialFlower * 255.0f)
            );
        }

        const bool springIsCurrent = seasonSystem.current == Season::Spring;
        const bool springIsNext = getNextSeason(seasonSystem.current) == Season::Spring;
        float tulipPresence = springIsCurrent ? 1.0f : 0.0f;
        if (springIsNext) tulipPresence = seasonVisual.transitionProgress;
        for (const Tulip& tulip : tulips) {
            renderTulip(renderer, tulipTextures, tulip, currentTicks,
                        static_cast<Uint8>(tulipPresence * 255.0f));
        }

        // Las nubes forman parte del fondo; el arbol se dibuja encima.
        const bool stormy = seasonVisual.rainIntensity > 0.0f ||
                            seasonVisual.snowIntensity > 0.0f;
        // La precipitación se dibuja antes que las nubes para que quede detrás.
        renderWeatherSystem(renderer, rain, seasonVisual.rainIntensity);
        renderWeatherSystem(renderer, snow, seasonVisual.snowIntensity);
        for (const Cloud& cloud : clouds) {
            renderCloud(renderer, cloudTextures, cloud, stormy);
        }

        renderTree(renderer, tree);

        renderFlyingAnimal(
            renderer,
            bee,
            currentTicks,
            static_cast<Uint8>(seasonVisual.beePresence * 255.0f)
        );
        renderFlyingAnimal(
            renderer,
            butterfly,
            currentTicks,
            static_cast<Uint8>(seasonVisual.butterflyPresence * 255.0f)
        );

        for (const Bird& currentBird : birds) {
            renderBird(renderer, currentBird);
        }

        // El clima se dibuja al final para que caiga delante del escenario.
        SDL_RenderPresent(renderer);
    }

    destroyWeatherSystem(snow);
    destroyWeatherSystem(rain);
    destroyCelestialBody(moon);
    destroyCelestialBody(sun);
    destroyCloudTextures(cloudTextures);
    destroyTulipTextures(tulipTextures);
    destroyGrassTextures(grassTextures);
    destroyFlyingAnimal(butterfly);
    destroyFlyingAnimal(bee);
    destroyFlowerTextures(flowerTextures);

    for (Bird& currentBird : birds) {
        destroyBird(currentBird);
    }

    destroyTree(tree);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
