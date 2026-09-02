#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "src/bird.hpp"
#include "src/cat.hpp"
#include "src/celestial_body.hpp"
#include "src/cloud.hpp"
#include "src/flower.hpp"
#include "src/flying_animal.hpp"
#include "src/grass.hpp"
#include "src/leaf.hpp"
#include "src/performance.hpp"
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

void renderSkyGradient(
    SDL_Renderer* renderer,
    int screenWidth,
    int groundY,
    SDL_Color topColor,
    SDL_Color horizonColor
) {
    if (screenWidth <= 0 || groundY <= 0) return;

    for (int y = 0; y < groundY; ++y) {
        const float progress = static_cast<float>(y) /
                               static_cast<float>(groundY);
        const SDL_Color color = {
            static_cast<Uint8>(topColor.r +
                (horizonColor.r - topColor.r) * progress),
            static_cast<Uint8>(topColor.g +
                (horizonColor.g - topColor.g) * progress),
            static_cast<Uint8>(topColor.b +
                (horizonColor.b - topColor.b) * progress),
            255
        };
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(renderer, 0, y, screenWidth, y);
    }
}

void renderFpsOverlay(SDL_Renderer* renderer, int screenWidth, int fps) {
    const std::string text = "FPS:" + std::to_string(fps);
    constexpr int scale = 3;
    constexpr int spacing = 1;
    constexpr int top = 12;
    int textWidth = 0;
    for (char character : text) {
        textWidth += (character == ':') ? 2 * scale : 4 * scale;
        textWidth += spacing;
    }

    int cursorX = screenWidth - textWidth - 12;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);

    for (char character : text) {
        const char* glyph[5] = {"", "", "", "", ""};
        switch (character) {
            case 'F': glyph[0] = "111"; glyph[1] = "100"; glyph[2] = "110"; glyph[3] = "100"; glyph[4] = "100"; break;
            case 'P': glyph[0] = "110"; glyph[1] = "101"; glyph[2] = "110"; glyph[3] = "100"; glyph[4] = "100"; break;
            case 'S': glyph[0] = "111"; glyph[1] = "100"; glyph[2] = "111"; glyph[3] = "001"; glyph[4] = "111"; break;
            case ':': glyph[0] = "0"; glyph[1] = "0"; glyph[2] = "1"; glyph[3] = "0"; glyph[4] = "1"; break;
            case '0': glyph[0] = "111"; glyph[1] = "101"; glyph[2] = "101"; glyph[3] = "101"; glyph[4] = "111"; break;
            case '1': glyph[0] = "010"; glyph[1] = "110"; glyph[2] = "010"; glyph[3] = "010"; glyph[4] = "111"; break;
            case '2': glyph[0] = "111"; glyph[1] = "001"; glyph[2] = "111"; glyph[3] = "100"; glyph[4] = "111"; break;
            case '3': glyph[0] = "111"; glyph[1] = "001"; glyph[2] = "111"; glyph[3] = "001"; glyph[4] = "111"; break;
            case '4': glyph[0] = "101"; glyph[1] = "101"; glyph[2] = "111"; glyph[3] = "001"; glyph[4] = "001"; break;
            case '5': glyph[0] = "111"; glyph[1] = "100"; glyph[2] = "111"; glyph[3] = "001"; glyph[4] = "111"; break;
            case '6': glyph[0] = "111"; glyph[1] = "100"; glyph[2] = "111"; glyph[3] = "101"; glyph[4] = "111"; break;
            case '7': glyph[0] = "111"; glyph[1] = "001"; glyph[2] = "001"; glyph[3] = "001"; glyph[4] = "001"; break;
            case '8': glyph[0] = "111"; glyph[1] = "101"; glyph[2] = "111"; glyph[3] = "101"; glyph[4] = "111"; break;
            case '9': glyph[0] = "111"; glyph[1] = "101"; glyph[2] = "111"; glyph[3] = "001"; glyph[4] = "111"; break;
            default: break;
        }
        for (int row = 0; row < 5; ++row) {
            for (int column = 0; glyph[row][column] != '\0'; ++column) {
                if (glyph[row][column] == '1') {
                    SDL_Rect pixel = {cursorX + column * scale,
                                      top + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
        cursorX += ((character == ':') ? 2 : 4) * scale + spacing;
    }
}

struct SeasonMetric {
    std::size_t frames = 0;
    double totalMilliseconds = 0.0;
};

std::size_t countActiveElements(const SeasonVisualState& state,
                                Season season, bool daytime) {
    std::size_t elements = static_cast<std::size_t>(
        std::ceil(state.flowerCount) + std::ceil(state.grassCount)
    );
    elements += static_cast<std::size_t>(std::ceil(state.rainIntensity * 220.0f));
    elements += static_cast<std::size_t>(std::ceil(state.snowIntensity * 150.0f));
    if (season == Season::Spring) elements += 20 + 6;
    if (season == Season::Autumn) elements += 48;
    if (season != Season::Winter) elements += 1; // gato
    if (daytime) {
        elements += 2; // abeja y mariposa
        elements += 2; // pajaros
    }
    return elements;
}
}

int main(int argc, char* argv[]) {
    bool runBenchmark = std::getenv("SCREENSAVER_BENCHMARK") != nullptr;
    UpdateExecutionMode executionMode = UpdateExecutionMode::Parallel;
    UpdateExecutionMode benchmarkMode = UpdateExecutionMode::Compare;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--benchmark") {
            runBenchmark = true;
        } else if (argument == "--sequential") {
            executionMode = UpdateExecutionMode::Sequential;
            benchmarkMode = UpdateExecutionMode::Sequential;
        } else if (argument == "--parallel") {
            executionMode = UpdateExecutionMode::Parallel;
            benchmarkMode = UpdateExecutionMode::Parallel;
        }
    }

    if (runBenchmark) {
        return runPerformanceBenchmark(benchmarkMode);
    }

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
        SDL_RENDERER_ACCELERATED
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
    Cat cat;
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

    if (!loadStarTextures(
            starTextures,
            renderer,
            {
                "assets/sky/blinking_star_1.png",
                "assets/sky/blinking_star_2.png"
            },
            {
                "assets/sky/shooting_star_frame_1.png",
                "assets/sky/shooting_star_frame_2.png"
            }
        )) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

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

    Bird blueBird;
    if (!loadAnimatedBird(
            blueBird,
            renderer,
            {{
                "assets/animals/blue_bird_frame_1.png",
                "assets/animals/blue_bird_frame_2.png",
                "assets/animals/blue_bird_frame_2.png"
            }},
            static_cast<float>(displayBounds.w) * 0.62f,
            static_cast<float>(displayBounds.h) * 0.18f,
            180
        )) {
        for (Bird& currentBird : birds) destroyBird(currentBird);
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    // Los cuadros azules tambien son de 1254x1254; 90x90 mantiene la misma
    // escala visual del pajaro rojo y queda apenas por encima de los insectos.
    blueBird.width = 90;
    blueBird.height = 90;
    birds.push_back(blueBird);

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
            static_cast<float>(displayBounds.w) * 0.80f,
            static_cast<float>(displayBounds.h) * 0.75f,
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
    // La abeja cruza el escenario de derecha a izquierda y se mantiene cerca
    // de la altura de las flores.
    bee.velocityX = -75.0f;
    bee.baseY = static_cast<float>(displayBounds.h) * 0.75f;

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
                {
                    "assets/grass/spring_grass_frame_1.png",
                    "assets/grass/spring_grass_frame_2.png",
                    "assets/grass/spring_grass_frame_3.png"
                },
                {
                    "assets/grass/summer_grass_frame_1.png",
                    "assets/grass/summer_grass_grame_2.png",
                    "assets/grass/summer_grass_grame_2.png"
                },
                {
                    "assets/grass/autum_grass_frame_1.png",
                    "assets/grass/autum_grass_frame_2.png",
                    "assets/grass/autum_grass_frame_2.png"
                }
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
            {
                "assets/rain/water_drop_frame_1.png",
                "assets/rain/water_drop_frame_2.png",
                "assets/rain/water_drop_frame_3.png"
            },
            220,
            4,
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
    setWeatherImpactAnimation(rain, true, 0.85f);

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
    setWeatherAccumulation(snow, true, 0.85f);

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

    if (!loadCat(cat, renderer)) {
        destroyLeafTextures(leafTextures);
        destroyTulipTextures(tulipTextures);
        destroyCloudTextures(cloudTextures);
        destroyWeatherSystem(snow);
        destroyWeatherSystem(rain);
        destroyGrassTextures(grassTextures);
        destroyFlyingAnimal(butterfly);
        destroyFlyingAnimal(bee);
        destroyFlowerTextures(flowerTextures);
        for (Bird& currentBird : birds) destroyBird(currentBird);
        destroyStarTextures(starTextures);
        destroyTree(tree);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    Uint32 previousTicks = SDL_GetTicks();
    Uint32 fpsSampleStart = previousTicks;
    int renderedFrames = 0;
    int displayedFps = 0;
    const unsigned int availableThreads = std::max(
        1u, std::thread::hardware_concurrency()
    );
    std::array<SeasonMetric, 4> seasonMetrics{};
    auto metricsSampleStart = std::chrono::steady_clock::now();
    auto frameStart = metricsSampleStart;
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
        if (executionMode == UpdateExecutionMode::Sequential) {
            updateCloudPositionsSequential(
                clouds, cloudTextures, screenWidth, screenHeight, deltaSeconds
            );
        } else {
            updateCloudPositionsParallel(
                clouds, cloudTextures, screenWidth, screenHeight, deltaSeconds
            );
        }

        updateTreePosition(tree, screenWidth, groundY);
        updateCat(cat, screenWidth, groundY);

        LeafSeason leafSeason = LeafSeason::Spring;
        if (seasonSystem.current == Season::Autumn) {
            leafSeason = LeafSeason::Autumn;
        }
        // Las hojas se actualizan en paralelo: cada hilo trabaja sobre un bloque
        // independiente y la fase de animacion no toca SDL.
        if (executionMode == UpdateExecutionMode::Sequential) {
            updateLeavesSequential(
                leaves, leafTextures, tree.dest, leafSeason, deltaSeconds, currentTicks
            );
        } else {
            updateLeavesParallel(
                leaves, leafTextures, tree.dest, leafSeason, deltaSeconds, currentTicks
            );
        }

        // Cada flor actualiza su posicion en paralelo cuando cambia la ventana.
        if (
            screenWidth != previousFlowerScreenWidth ||
            screenHeight != previousFlowerScreenHeight ||
            groundY != previousFlowerGroundY
        ) {
            if (executionMode == UpdateExecutionMode::Sequential) {
                updateFlowerPositionsSequential(
                    flowers, flowerTextures, screenWidth, screenHeight, groundY
                );
            } else {
                updateFlowerPositionsParallel(
                    flowers, flowerTextures, screenWidth, screenHeight, groundY
                );
            }

            previousFlowerScreenWidth = screenWidth;
            previousFlowerScreenHeight = screenHeight;
            previousFlowerGroundY = groundY;
        }

        if (
            screenWidth != previousTulipScreenWidth ||
            screenHeight != previousTulipScreenHeight ||
            groundY != previousTulipGroundY
        ) {
            if (executionMode == UpdateExecutionMode::Sequential) {
                updateTulipPositionsSequential(
                    tulips, tulipTextures, screenWidth, screenHeight, groundY
                );
            } else {
                updateTulipPositionsParallel(
                    tulips, tulipTextures, screenWidth, screenHeight, groundY
                );
            }
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

        const float twilight = std::clamp(
            1.0f - std::abs(daylight - 0.50f) / 0.22f,
            0.0f,
            1.0f
        );
        const SDL_Color warmHorizon = {255, 145, 95, 255};
        SDL_Color horizonColor = {
            static_cast<Uint8>(skyColor.r * 0.68f),
            static_cast<Uint8>(skyColor.g * 0.68f),
            static_cast<Uint8>(skyColor.b * 0.68f),
            255
        };
        horizonColor.r = static_cast<Uint8>(
            horizonColor.r + (warmHorizon.r - horizonColor.r) * twilight * 0.45f
        );
        horizonColor.g = static_cast<Uint8>(
            horizonColor.g + (warmHorizon.g - horizonColor.g) * twilight * 0.45f
        );
        horizonColor.b = static_cast<Uint8>(
            horizonColor.b + (warmHorizon.b - horizonColor.b) * twilight * 0.45f
        );
        renderSkyGradient(renderer, screenWidth, groundY, skyColor, horizonColor);

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

        // La lluvia queda detras del pasto y las flores, pero encima del
        // suelo para que el impacto de la gota siga siendo visible.
        renderWeatherSystem(renderer, rain, seasonVisual.rainIntensity);
        renderWeatherSystem(renderer, snow, seasonVisual.snowIntensity);

        std::size_t grassSeason = 0;
        if (seasonSystem.current == Season::Summer) grassSeason = 1;
        if (seasonSystem.current == Season::Autumn) grassSeason = 2;
        renderGrassField(
            renderer,
            grassTextures,
            grass,
            screenWidth,
            screenHeight,
            groundY,
            currentTicks,
            seasonVisual.grassCount,
            grassSeason
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
        for (const Cloud& cloud : clouds) {
            renderCloud(renderer, cloudTextures, cloud, stormy);
        }

        renderTree(renderer, tree);

        if (seasonSystem.current != Season::Winter) {
            renderCat(renderer, cat, currentTicks, !isDaytime);
        }

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

        ++renderedFrames;
        const Uint32 fpsElapsed = currentTicks - fpsSampleStart;
        if (fpsElapsed >= 500) {
            displayedFps = static_cast<int>(
                std::round(renderedFrames * 1000.0f / fpsElapsed)
            );
            renderedFrames = 0;
            fpsSampleStart = currentTicks;
        }
        renderFpsOverlay(renderer, screenWidth, displayedFps);
        SDL_RenderPresent(renderer);

        const auto frameEnd = std::chrono::steady_clock::now();
        const double frameMilliseconds = std::chrono::duration<double, std::milli>(
            frameEnd - frameStart
        ).count();
        SeasonMetric& currentMetric = seasonMetrics[
            static_cast<std::size_t>(seasonSystem.current)
        ];
        ++currentMetric.frames;
        currentMetric.totalMilliseconds += frameMilliseconds;

        const auto metricsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            frameEnd - metricsSampleStart
        ).count();
        if (metricsElapsed >= 1000) {
            const std::size_t metricIndex = static_cast<std::size_t>(seasonSystem.current);
            const SeasonMetric& metric = seasonMetrics[metricIndex];
            const double averageMilliseconds = metric.frames > 0
                ? metric.totalMilliseconds / metric.frames
                : 0.0;
            const double averageFps = averageMilliseconds > 0.0
                ? 1000.0 / averageMilliseconds
                : 0.0;
            const char* modeLabel = executionMode == UpdateExecutionMode::Parallel
                ? "paralelo" : "secuencial";
            std::cout << "[metrica] " << getSeasonProfile(seasonSystem.current).name
                      << " | modo: " << modeLabel
                      << " | FPS promedio: " << std::fixed << std::setprecision(1)
                      << averageFps
                      << " | cuadro: " << averageMilliseconds << " ms"
                      << " | elementos: " << countActiveElements(
                             seasonVisual, seasonSystem.current, isDaytime)
                      << " | hilos CPU: " << availableThreads
                      << " | hilos hojas: 8"
                      << std::endl;
            metricsSampleStart = frameEnd;
        }
        frameStart = frameEnd;
    }

    destroyWeatherSystem(snow);
    destroyWeatherSystem(rain);
    destroyLeafTextures(leafTextures);
    destroyCat(cat);
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
