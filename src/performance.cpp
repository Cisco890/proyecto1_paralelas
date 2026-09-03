#include "performance.hpp"

#include "cloud.hpp"
#include "flower.hpp"
#include "leaf.hpp"
#include "tulip.hpp"
#include "weather.hpp"

#include <SDL2/SDL.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;

template <typename Callback>
double measureMilliseconds(Callback&& callback) {
    const Clock::time_point start = Clock::now();
    callback();
    const Clock::time_point end = Clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void printSummary(const BenchmarkSummary& summary) {
    const double speedup = summary.parallelMilliseconds > 0.0
        ? summary.sequentialMilliseconds / summary.parallelMilliseconds
        : 0.0;
    std::cout << summary.label
              << " | elementos: " << summary.elements
              << " | iteraciones: " << summary.iterations
              << " | secuencial: " << std::fixed << std::setprecision(3)
              << summary.sequentialMilliseconds << " ms"
              << " | paralelo: " << summary.parallelMilliseconds << " ms"
              << " | aceleracion: x" << speedup
              << std::endl;
}

void printSingleModeSummary(const char* label, std::size_t elements,
                            std::size_t iterations, const char* modeLabel,
                            double milliseconds) {
    std::cout << label
              << " | elementos: " << elements
              << " | iteraciones: " << iterations
              << " | modo: " << modeLabel
              << " | tiempo: " << std::fixed << std::setprecision(3)
              << milliseconds << " ms"
              << std::endl;
}

BenchmarkSummary benchmarkFlowers() {
    constexpr std::size_t elementCount = 120000;
    constexpr std::size_t iterations = 180;
    FlowerTextures textures = {};
    textures.width = 12;
    textures.height = 12;
    std::vector<Flower> sequential = createFlowerField(elementCount);
    std::vector<Flower> parallel = sequential;

    const double sequentialMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateFlowerPositionsSequential(sequential, textures, 1920, 1080, 880);
        }
    });
    const double parallelMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateFlowerPositionsParallel(parallel, textures, 1920, 1080, 880);
        }
    });

    return {"Flores", elementCount, iterations, sequentialMs, parallelMs};
}

BenchmarkSummary benchmarkTulips() {
    constexpr std::size_t elementCount = 120000;
    constexpr std::size_t iterations = 180;
    TulipTextures textures = {};
    textures.red.width = 12;
    textures.red.height = 12;
    textures.orange.width = 12;
    textures.orange.height = 12;
    std::vector<Tulip> sequential = createTulipField(elementCount);
    std::vector<Tulip> parallel = sequential;

    const double sequentialMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateTulipPositionsSequential(sequential, textures, 1920, 1080, 880);
        }
    });
    const double parallelMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateTulipPositionsParallel(parallel, textures, 1920, 1080, 880);
        }
    });

    return {"Tulipanes", elementCount, iterations, sequentialMs, parallelMs};
}

BenchmarkSummary benchmarkClouds() {
    constexpr std::size_t elementCount = 60000;
    constexpr std::size_t iterations = 360;
    CloudTextures textures = {};
    textures.width = 32;
    textures.height = 16;
    std::vector<Cloud> sequential = createCloudField(elementCount);
    std::vector<Cloud> parallel = sequential;

    const double sequentialMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateCloudPositionsSequential(sequential, textures, 1920, 1080, 0.016f);
        }
    });
    const double parallelMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateCloudPositionsParallel(parallel, textures, 1920, 1080, 0.016f);
        }
    });

    return {"Nubes", elementCount, iterations, sequentialMs, parallelMs};
}

BenchmarkSummary benchmarkLeaves() {
    constexpr std::size_t elementCount = 150000;
    constexpr std::size_t iterations = 220;
    LeafTextures textures = {};
    textures.width = 10;
    textures.height = 10;
    const SDL_Rect treeDest = {760, 220, 420, 620};
    std::vector<Leaf> sequential = createLeafField(elementCount);
    std::vector<Leaf> parallel = sequential;

    const double sequentialMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateLeavesSequential(
                sequential, textures, treeDest, LeafSeason::Autumn, 0.016f,
                static_cast<Uint32>(iteration * 16), 1.0f
            );
        }
    });
    const double parallelMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateLeavesParallel(
                parallel, textures, treeDest, LeafSeason::Autumn, 0.016f,
                static_cast<Uint32>(iteration * 16), 1.0f
            );
        }
    });

    return {"Hojas", elementCount, iterations, sequentialMs, parallelMs};
}

WeatherSystem createBenchmarkWeather(std::size_t elementCount) {
    WeatherSystem system = {};
    system.scale = 1;
    system.spawnY = -0.05f;
    system.groundY = 0.85f;
    system.impactAnimation = false;
    system.accumulate = false;
    system.particles.reserve(elementCount);
    for (std::size_t index = 0; index < elementCount; ++index) {
        system.particles.push_back({
            static_cast<float>((index * 37) % 1000) / 1000.0f,
            -0.05f + static_cast<float>((index * 19) % 30) / 1000.0f,
            180.0f + static_cast<float>((index * 13) % 80),
            static_cast<float>((index * 7) % 20) - 10.0f,
            0, 0.0f, false
        });
    }
    return system;
}

BenchmarkSummary benchmarkWeather() {
    constexpr std::size_t elementCount = 120000;
    constexpr std::size_t iterations = 180;
    WeatherSystem sequential = createBenchmarkWeather(elementCount);
    WeatherSystem parallel = sequential;

    const double sequentialMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateWeatherSystem(sequential, 1920, 1080, 0.016f, 1.0f);
        }
    });
    const double parallelMs = measureMilliseconds([&]() {
        for (std::size_t iteration = 0; iteration < iterations; ++iteration) {
            updateWeatherSystemParallel(parallel, 1920, 1080, 0.016f, 1.0f);
        }
    });

    return {"Clima", elementCount, iterations, sequentialMs, parallelMs};
}
}

int runPerformanceBenchmark(UpdateExecutionMode mode) {
    const std::vector<BenchmarkSummary> results = {
        benchmarkFlowers(),
        benchmarkTulips(),
        benchmarkClouds(),
        benchmarkLeaves(),
        benchmarkWeather()
    };

    if (mode == UpdateExecutionMode::Compare) {
        std::cout
            << "Benchmark paralelo vs secuencial"
            << std::endl;
        for (const BenchmarkSummary& result : results) {
            printSummary(result);
        }
        return 0;
    }

    std::cout
        << "Benchmark de un solo modo"
        << std::endl;
    const char* modeLabel = mode == UpdateExecutionMode::Sequential
        ? "secuencial"
        : "paralelo";
    for (const BenchmarkSummary& result : results) {
        const double milliseconds = mode == UpdateExecutionMode::Sequential
            ? result.sequentialMilliseconds
            : result.parallelMilliseconds;
        printSingleModeSummary(
            result.label, result.elements, result.iterations, modeLabel, milliseconds
        );
    }

    return 0;
}
