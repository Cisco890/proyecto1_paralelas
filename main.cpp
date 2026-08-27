#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "src/bird.hpp"
#include "src/celestial_body.hpp"
#include "src/cloud.hpp"
#include "src/flower.hpp"
#include "src/flying_animal.hpp"
#include "src/grass.hpp"
#include "src/leaf.hpp"
#include "src/season.hpp"
#include "src/star.hpp"
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
    // Se crea con el tamano del escritorio, sin SDL_WINDOW_MAXIMIZED.
    // Algunos drivers de WSL/WSLg se quedan bloqueados al negociar esa
    // bandera durante SDL_CreateWindow.
    //
    SDL_Window* window = SDL_CreateWindow(
        "Arbol Estacional - Screen Saver",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        displayBounds.w,
        displayBounds.h,
        SDL_WINDOW_SHOWN
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
    LeafTextures leafTextures;
    CelestialBody sun = createCelestialBody(
        CelestialBodyType::Sun,
        {255, 221, 64, 255}
    );
    CelestialBody moon = createCelestialBody(
        CelestialBodyType::Moon,
        {245, 245, 235, 255}
    );
    StarTextures starTextures = {};

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

    Bird redBird;
    if (!loadAnimatedBird(
            redBird,
            renderer,
            {{
                "assets/animals/red_bird_frame_1.png",
                "assets/animals/red_bird_frame_2.png",
                "assets/animals/red_bird_frame_3.png"
            }},
            static_cast<float>(displayBounds.w) * 0.35f,
            static_cast<float>(displayBounds.h) * 0.28f,
            90
        )) {
        destroyBird(bird);
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    // Los sprites del pajaro rojo tienen un lienzo de 1254x1254 px. Se
    // reduce su destino a 90x90 para que sea apenas mayor que la abeja y la
    // mariposa, que se renderizan a 75x75 (25 px * escala 3).
    redBird.width = 90;
    redBird.height = 90;
    birds.push_back(redBird);

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
                "assets/animals/pink_butterfly_frame_1.png",
                "assets/animals/pink_butterfly_frame_2.png",
                "assets/animals/pink_butterfly_frame_2.png"
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

    if (!loadLeafTextures(renderer, leafTextures)) {
        destroyTulipTextures(tulipTextures);
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
    std::vector<Leaf> leaves = createLeafField(72);
    constexpr std::size_t maximumStarCount = 90;
    std::vector<Star> stars = createStarField(maximumStarCount);
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

        const float daylight = getDaylightFactor(currentTicks);
        const float daytimePresence = std::clamp(
            (daylight - 0.30f) / 0.35f, 0.0f, 1.0f
        );
        const bool isDaytime = daytimePresence > 0.05f;

        if (isDaytime) {
            for (Bird& currentBird : birds) {
                updateBird(
                    currentBird,
                    screenWidth,
                    screenHeight,
                    deltaSeconds
                );
            }
        }

        // El suelo comienza al 85% de la altura
        int groundY = static_cast<int>(screenHeight * 0.85);

        updateCelestialBody(
            sun, screenWidth, screenHeight, groundY, currentTicks
        );
        updateCelestialBody(
            moon, screenWidth, screenHeight, groundY, currentTicks
        );

        if (isDaytime) {
            updateFlyingAnimal(bee, screenWidth, groundY, deltaSeconds, currentTicks);
            updateFlyingAnimal(
                butterfly, screenWidth, groundY, deltaSeconds, currentTicks
            );
        }
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

        LeafSeason leafSeason = LeafSeason::Spring;
        if (seasonSystem.current == Season::Autumn) {
            leafSeason = LeafSeason::Autumn;
        }
        // Las hojas se actualizan en paralelo: cada hilo trabaja sobre un bloque
        // independiente y la fase de animacion no toca SDL.
        updateLeavesParallel(
            leaves, leafTextures, tree.dest, leafSeason, deltaSeconds, currentTicks
        );

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
        SDL_Color skyColor = seasonVisual.skyColor;
        const SDL_Color nightSky = {18, 27, 48, 255};
        skyColor.r = static_cast<Uint8>(
            nightSky.r + (skyColor.r - nightSky.r) * daytimePresence
        );
        skyColor.g = static_cast<Uint8>(
            nightSky.g + (skyColor.g - nightSky.g) * daytimePresence
        );
        skyColor.b = static_cast<Uint8>(
            nightSky.b + (skyColor.b - nightSky.b) * daytimePresence
        );
        const float rainDarkening = std::min(1.0f, seasonVisual.rainIntensity) * 0.18f;
        const float skyBrightness = 1.0f - rainDarkening * daytimePresence;
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

        // Las estrellas aparecen gradualmente conforme disminuye la luz.
        const float nightPresence = std::clamp(
            (1.0f - daylight) / 0.70f,
            0.0f,
            1.0f
        );
        renderStarField(
            renderer,
            starTextures,
            stars,
            screenWidth,
            groundY,
            currentTicks,
            maximumStarCount * nightPresence
        );

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
        const float groundShadow = 0.78f + daytimePresence * 0.22f;
        groundColor.r = static_cast<Uint8>(groundColor.r * groundShadow);
        groundColor.g = static_cast<Uint8>(groundColor.g * groundShadow);
        groundColor.b = static_cast<Uint8>(groundColor.b * groundShadow);

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

        std::size_t visibleLeaves = 0;
        Uint8 leafOpacity = 255;
        if (seasonSystem.current == Season::Spring) {
            visibleLeaves = 72;
        } else if (seasonSystem.current == Season::Autumn) {
            visibleLeaves = 48;
            // Las pilas se desvanecen durante el cambio de otono a invierno.
            leafOpacity = static_cast<Uint8>(
                (1.0f - seasonVisual.transitionProgress) * 255.0f
            );
        }
        renderLeaves(renderer, leafTextures, leaves, leafSeason,
                     visibleLeaves, currentTicks, leafOpacity);

        if (isDaytime) {
            renderFlyingAnimal(
                renderer,
                bee,
                currentTicks,
                static_cast<Uint8>(seasonVisual.beePresence * daytimePresence * 255.0f)
            );
            renderFlyingAnimal(
                renderer,
                butterfly,
                currentTicks,
                static_cast<Uint8>(seasonVisual.butterflyPresence * daytimePresence * 255.0f)
            );

            for (const Bird& currentBird : birds) {
                renderBird(renderer, currentBird);
            }
        }

        // El clima se dibuja al final para que caiga delante del escenario.
        SDL_RenderPresent(renderer);
    }

    destroyWeatherSystem(snow);
    destroyWeatherSystem(rain);
    destroyLeafTextures(leafTextures);
    destroyStarTextures(starTextures);
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
