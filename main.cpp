#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cmath>
#include <iostream>
#include <vector>

#include "src/bird.hpp"
#include "src/flower.hpp"
#include "src/flying_animal.hpp"
#include "src/grass.hpp"
#include "src/season.hpp"
#include "src/tree.hpp"
#include "src/weather.hpp"

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

    bool running = true;
    Uint32 previousTicks = SDL_GetTicks();
    SeasonSystem seasonSystem = createSeasonSystem(previousTicks);
    std::vector<Flower> flowers = createFlowerField(36);
    std::vector<GrassBlade> grass = createGrassField(72);
    int previousFlowerScreenWidth = -1;
    int previousFlowerScreenHeight = -1;
    int previousFlowerGroundY = -1;

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

        // Cielo
        SDL_SetRenderDrawColor(
            renderer,
            seasonVisual.skyColor.r,
            seasonVisual.skyColor.g,
            seasonVisual.skyColor.b,
            seasonVisual.skyColor.a
        );
        SDL_RenderClear(renderer);

        // Suelo temporal
        SDL_Rect ground = {
            0,
            groundY,
            screenWidth,
            screenHeight - groundY
        };

        SDL_SetRenderDrawColor(
            renderer,
            seasonVisual.groundColor.r,
            seasonVisual.groundColor.g,
            seasonVisual.groundColor.b,
            seasonVisual.groundColor.a
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
        renderWeatherSystem(renderer, rain, seasonVisual.rainIntensity);
        renderWeatherSystem(renderer, snow, seasonVisual.snowIntensity);

        SDL_RenderPresent(renderer);
    }

    destroyWeatherSystem(snow);
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

    return 0;
}
