#include "performance.hpp"

#include "cloud.hpp"
#include "flower.hpp"
#include "leaf.hpp"
#include "tulip.hpp"
#include "weather.hpp"

#include <SDL2/SDL.h>

#include <chrono>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
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

struct BenchmarkStatistics {
    BenchmarkSummary average;
    double sequentialMinimum;
    double sequentialMaximum;
    double sequentialDeviation;
    double parallelMinimum;
    double parallelMaximum;
    double parallelDeviation;
};

BenchmarkSummary benchmarkFlowers(
    std::size_t elementCount = 120000, std::size_t iterations = 180
);
BenchmarkSummary benchmarkTulips(
    std::size_t elementCount = 120000, std::size_t iterations = 180
);
BenchmarkSummary benchmarkClouds(
    std::size_t elementCount = 60000, std::size_t iterations = 360
);
BenchmarkSummary benchmarkLeaves(
    std::size_t elementCount = 150000, std::size_t iterations = 220
);

std::vector<BenchmarkSummary> runBenchmarkSuite() {
    return {
        benchmarkFlowers(),
        benchmarkTulips(),
        benchmarkClouds(),
        benchmarkLeaves()
    };
}

double standardDeviation(const std::vector<double>& values, double average) {
    double sum = 0.0;
    for (const double value : values) {
        const double difference = value - average;
        sum += difference * difference;
    }
    return values.empty() ? 0.0 : std::sqrt(sum / values.size());
}

std::vector<BenchmarkStatistics> calculateStatistics(
    const std::vector<std::vector<BenchmarkSummary>>& samples
) {
    std::vector<BenchmarkStatistics> statistics;
    if (samples.empty()) return statistics;

    for (std::size_t index = 0; index < samples.front().size(); ++index) {
        std::vector<double> sequentialValues;
        std::vector<double> parallelValues;
        for (const std::vector<BenchmarkSummary>& sample : samples) {
            sequentialValues.push_back(sample[index].sequentialMilliseconds);
            parallelValues.push_back(sample[index].parallelMilliseconds);
        }
        double sequentialTotal = 0.0;
        double parallelTotal = 0.0;
        for (const double value : sequentialValues) sequentialTotal += value;
        for (const double value : parallelValues) parallelTotal += value;
        const double sequentialAverage = sequentialTotal / sequentialValues.size();
        const double parallelAverage = parallelTotal / parallelValues.size();
        const BenchmarkSummary& first = samples.front()[index];
        statistics.push_back({
            {first.label, first.elements, first.iterations, sequentialAverage, parallelAverage},
            *std::min_element(sequentialValues.begin(), sequentialValues.end()),
            *std::max_element(sequentialValues.begin(), sequentialValues.end()),
            standardDeviation(sequentialValues, sequentialAverage),
            *std::min_element(parallelValues.begin(), parallelValues.end()),
            *std::max_element(parallelValues.begin(), parallelValues.end()),
            standardDeviation(parallelValues, parallelAverage)
        });
    }
    return statistics;
}

void writeDetailedReport(const std::vector<std::vector<BenchmarkSummary>>& samples) {
    constexpr std::size_t sampleCount = 3;
    const std::vector<BenchmarkStatistics> results = calculateStatistics(samples);
    std::ofstream csv("benchmark_results.csv");
    if (csv) {
        csv << "algoritmo,elementos,iteraciones,repeticiones,secuencial_promedio_ms,"
            << "secuencial_minimo_ms,secuencial_maximo_ms,secuencial_desviacion_ms,"
            << "paralelo_promedio_ms,paralelo_minimo_ms,paralelo_maximo_ms,"
            << "paralelo_desviacion_ms,aceleracion,mejora_porcentaje\n";
        csv << std::fixed << std::setprecision(3);
        for (const BenchmarkStatistics& statistic : results) {
            const BenchmarkSummary& result = statistic.average;
            const double speedup = result.parallelMilliseconds > 0.0
                ? result.sequentialMilliseconds / result.parallelMilliseconds
                : 0.0;
            const double improvement = result.sequentialMilliseconds > 0.0
                ? (1.0 - result.parallelMilliseconds / result.sequentialMilliseconds) * 100.0
                : 0.0;
            csv << result.label << ',' << result.elements << ',' << result.iterations << ','
                << sampleCount << ',' << result.sequentialMilliseconds << ','
                << statistic.sequentialMinimum << ',' << statistic.sequentialMaximum << ','
                << statistic.sequentialDeviation << ',' << result.parallelMilliseconds << ','
                << statistic.parallelMinimum << ',' << statistic.parallelMaximum << ','
                << statistic.parallelDeviation << ',' << speedup << ',' << improvement << '\n';
        }
    }

    double maximumTime = 1.0;
    for (const BenchmarkStatistics& statistic : results) {
        const BenchmarkSummary& result = statistic.average;
        maximumTime = std::max(maximumTime, result.sequentialMilliseconds);
        maximumTime = std::max(maximumTime, result.parallelMilliseconds);
    }

    constexpr int width = 1180;
    constexpr int chartLeft = 290;
    constexpr int chartWidth = 650;
    constexpr int rowHeight = 108;
    const int height = 195 + static_cast<int>(results.size()) * rowHeight;
    std::ofstream chart("benchmark_chart.svg");
    if (chart) {
        chart << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width
              << "\" height=\"" << height << "\" viewBox=\"0 0 " << width << ' '
              << height << "\">\n"
              << "<rect width=\"100%\" height=\"100%\" fill=\"#fffdf6\"/>\n"
              << "<text x=\"40\" y=\"45\" font-family=\"sans-serif\" font-size=\"24\" "
              << "font-weight=\"bold\" fill=\"#24352a\">Comparacion de rendimiento</text>\n"
              << "<text x=\"40\" y=\"70\" font-family=\"sans-serif\" font-size=\"15\" "
              << "fill=\"#526257\">Promedio de 3 ejecuciones. Una barra mas corta es mejor.</text>\n"
              << "<rect x=\"40\" y=\"82\" width=\"16\" height=\"16\" fill=\"#d97706\"/>\n"
              << "<text x=\"63\" y=\"96\" font-family=\"sans-serif\" "
              << "font-size=\"14\">Secuencial</text>\n"
              << "<rect x=\"180\" y=\"82\" width=\"16\" height=\"16\" fill=\"#15803d\"/>\n"
              << "<text x=\"203\" y=\"96\" font-family=\"sans-serif\" "
              << "font-size=\"14\">Paralelo</text>\n";

        for (int tick = 0; tick <= 4; ++tick) {
            const double value = maximumTime * tick / 4.0;
            const int x = chartLeft + chartWidth * tick / 4;
            chart << "<line x1=\"" << x << "\" y1=\"122\" x2=\"" << x
                  << "\" y2=\"" << height - 35
                  << "\" stroke=\"#d8ddd5\" stroke-dasharray=\"4 4\"/>\n"
                  << "<text x=\"" << x << "\" y=\"118\" text-anchor=\"middle\" "
                  << "font-family=\"sans-serif\" font-size=\"12\" fill=\"#526257\">"
                  << value << " ms</text>\n";
        }

        chart << std::fixed << std::setprecision(1);
        for (std::size_t index = 0; index < results.size(); ++index) {
            const BenchmarkSummary& result = results[index].average;
            const int y = 142 + static_cast<int>(index) * rowHeight;
            const double sequentialWidth = result.sequentialMilliseconds / maximumTime * chartWidth;
            const double parallelWidth = result.parallelMilliseconds / maximumTime * chartWidth;
            const double speedup = result.parallelMilliseconds > 0.0
                ? result.sequentialMilliseconds / result.parallelMilliseconds
                : 0.0;
            const double improvement = result.sequentialMilliseconds > 0.0
                ? (1.0 - result.parallelMilliseconds / result.sequentialMilliseconds) * 100.0
                : 0.0;
            chart << "<text x=\"40\" y=\"" << y + 24
                  << "\" font-family=\"sans-serif\" font-size=\"18\" fill=\"#24352a\">"
                  << result.label << "</text>\n"
                  << "<text x=\"40\" y=\"" << y + 46
                  << "\" font-family=\"sans-serif\" font-size=\"12\" fill=\"#526257\">"
                  << result.elements << " elementos, " << result.iterations << " iteraciones</text>\n"
                  << "<rect x=\"" << chartLeft << "\" y=\"" << y
                  << "\" width=\"" << sequentialWidth << "\" height=\"24\" fill=\"#d97706\"/>\n"
                  << "<rect x=\"" << chartLeft << "\" y=\"" << y + 31
                  << "\" width=\"" << parallelWidth << "\" height=\"24\" fill=\"#15803d\"/>\n"
                  << "<text x=\"" << chartLeft + chartWidth + 12 << "\" y=\"" << y + 20
                  << "\" font-family=\"monospace\" font-size=\"13\">"
                  << result.sequentialMilliseconds << " ms</text>\n"
                  << "<text x=\"" << chartLeft + chartWidth + 12 << "\" y=\"" << y + 51
                  << "\" font-family=\"monospace\" font-size=\"13\">"
                  << result.parallelMilliseconds << " ms | x" << speedup << " | "
                  << improvement << "% menos</text>\n";
        }
        chart << "</svg>\n";
    }

    if (csv && chart) {
        std::cout << "Analisis detallado generado con 3 repeticiones: "
                  << "benchmark_results.csv y benchmark_chart.svg" << std::endl;
    } else {
        std::cerr << "No se pudo generar el analisis detallado." << std::endl;
    }
}

BenchmarkSummary benchmarkFlowers(std::size_t elementCount, std::size_t iterations) {
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

BenchmarkSummary benchmarkTulips(std::size_t elementCount, std::size_t iterations) {
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

BenchmarkSummary benchmarkClouds(std::size_t elementCount, std::size_t iterations) {
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

BenchmarkSummary benchmarkLeaves(std::size_t elementCount, std::size_t iterations) {
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
}

int runPerformanceBenchmark(UpdateExecutionMode mode, bool detailed) {
    std::vector<std::vector<BenchmarkSummary>> samples;
    samples.push_back(runBenchmarkSuite());
    if (detailed) {
        samples.push_back(runBenchmarkSuite());
        samples.push_back(runBenchmarkSuite());
    }
    const std::vector<BenchmarkSummary>& results = samples.front();

    if (mode == UpdateExecutionMode::Compare) {
        std::cout
            << "Benchmark paralelo vs secuencial"
            << std::endl;
        for (const BenchmarkSummary& result : results) {
            printSummary(result);
        }
        if (detailed) writeDetailedReport(samples);
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

    if (detailed) writeDetailedReport(samples);

    return 0;
}

int runScalabilityBenchmark() {
    constexpr std::array<std::size_t, 3> workloads = {20000, 60000, 120000};
    constexpr std::size_t iterations = 90;
    const std::array<BenchmarkSummary (*)(std::size_t, std::size_t), 4> benchmarks = {
        benchmarkFlowers, benchmarkTulips, benchmarkClouds, benchmarkLeaves
    };

    std::vector<BenchmarkSummary> results;
    for (const auto benchmark : benchmarks) {
        for (const std::size_t workload : workloads) {
            results.push_back(benchmark(workload, iterations));
        }
    }

    std::ofstream csv("benchmark_scalability.csv");
    csv << "algoritmo,elementos,iteraciones,secuencial_ms,paralelo_ms,aceleracion,mejora_porcentaje\n";
    csv << std::fixed << std::setprecision(3);
    double maximumSpeedup = 1.0;
    for (const BenchmarkSummary& result : results) {
        const double speedup = result.parallelMilliseconds > 0.0
            ? result.sequentialMilliseconds / result.parallelMilliseconds
            : 0.0;
        const double improvement = result.sequentialMilliseconds > 0.0
            ? (1.0 - result.parallelMilliseconds / result.sequentialMilliseconds) * 100.0
            : 0.0;
        maximumSpeedup = std::max(maximumSpeedup, speedup);
        csv << result.label << ',' << result.elements << ',' << result.iterations << ','
            << result.sequentialMilliseconds << ',' << result.parallelMilliseconds << ','
            << speedup << ',' << improvement << '\n';
        std::cout << result.label << " | " << result.elements << " elementos | x"
                  << speedup << " | ";
        if (improvement >= 0.0) {
            std::cout << improvement << "% menos";
        } else {
            std::cout << -improvement << "% mas lento";
        }
        std::cout << std::endl;
    }

    constexpr int width = 900;
    constexpr int height = 540;
    constexpr int chartLeft = 105;
    constexpr int chartRight = 830;
    constexpr int chartTop = 135;
    constexpr int chartBottom = 445;
    const std::array<const char*, 4> colors = {
        "#b45309", "#15803d", "#2563eb", "#9333ea"
    };
    std::ofstream chart("benchmark_scalability.svg");
    if (chart) {
        chart << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width
              << "\" height=\"" << height << "\" viewBox=\"0 0 " << width << ' '
              << height << "\">\n<rect width=\"100%\" height=\"100%\" fill=\"#fffdf6\"/>\n"
              << "<text x=\"40\" y=\"45\" font-family=\"sans-serif\" font-size=\"24\" "
              << "font-weight=\"bold\" fill=\"#24352a\">Escalabilidad del paralelismo</text>\n"
              << "<text x=\"40\" y=\"70\" font-family=\"sans-serif\" font-size=\"15\" "
              << "fill=\"#526257\">Aceleracion (x): mayor es mejor.</text>\n";
        for (std::size_t index = 0; index < benchmarks.size(); ++index) {
            const int x = 40 + static_cast<int>(index) * 170;
            chart << "<rect x=\"" << x << "\" y=\"88\" width=\"14\" height=\"14\" fill=\""
                  << colors[index] << "\"/>\n<text x=\"" << x + 20
                  << "\" y=\"100\" font-family=\"sans-serif\" font-size=\"13\">"
                  << results[index * workloads.size()].label << "</text>\n";
        }
        for (int tick = 0; tick <= 4; ++tick) {
            const double value = maximumSpeedup * tick / 4.0;
            const int y = chartBottom - (chartBottom - chartTop) * tick / 4;
            chart << "<line x1=\"" << chartLeft << "\" y1=\"" << y << "\" x2=\""
                  << chartRight << "\" y2=\"" << y << "\" stroke=\"#d8ddd5\" "
                  << "stroke-dasharray=\"4 4\"/>\n<text x=\"" << chartLeft - 12
                  << "\" y=\"" << y + 4 << "\" text-anchor=\"end\" font-family=\"sans-serif\" "
                  << "font-size=\"12\" fill=\"#526257\">x" << value << "</text>\n";
        }
        for (std::size_t group = 0; group < workloads.size(); ++group) {
            const int x = chartLeft + static_cast<int>(group) * (chartRight - chartLeft) /
                static_cast<int>(workloads.size() - 1);
            chart << "<text x=\"" << x << "\" y=\"475\" text-anchor=\"middle\" "
                  << "font-family=\"sans-serif\" font-size=\"13\">" << workloads[group]
                  << " elementos</text>\n";
        }
        for (std::size_t benchmark = 0; benchmark < benchmarks.size(); ++benchmark) {
            chart << "<polyline fill=\"none\" stroke=\"" << colors[benchmark]
                  << "\" stroke-width=\"3\" points=\"";
            for (std::size_t group = 0; group < workloads.size(); ++group) {
                const BenchmarkSummary& result = results[benchmark * workloads.size() + group];
                const double speedup = result.parallelMilliseconds > 0.0
                    ? result.sequentialMilliseconds / result.parallelMilliseconds
                    : 0.0;
                const int x = chartLeft + static_cast<int>(group) * (chartRight - chartLeft) /
                    static_cast<int>(workloads.size() - 1);
                const int y = chartBottom - static_cast<int>(
                    speedup / maximumSpeedup * (chartBottom - chartTop)
                );
                chart << x << ',' << y << ' ';
            }
            chart << "\"/>\n";
        }
        chart << "</svg>\n";
    }

    std::cout << "Prueba de escalabilidad generada: benchmark_scalability.csv y "
              << "benchmark_scalability.svg" << std::endl;
    return 0;
}

int runConsistencyValidation() {
    constexpr float tolerance = 0.0001f;
    const auto sameFloat = [=](float first, float second) {
        return std::fabs(first - second) < tolerance;
    };
    const auto sameRect = [](const SDL_Rect& first, const SDL_Rect& second) {
        return first.x == second.x && first.y == second.y &&
            first.w == second.w && first.h == second.h;
    };

    FlowerTextures flowerTextures = {};
    flowerTextures.width = 12;
    flowerTextures.height = 12;
    std::vector<Flower> sequentialFlowers = createFlowerField(4096);
    std::vector<Flower> parallelFlowers = sequentialFlowers;
    updateFlowerPositionsSequential(sequentialFlowers, flowerTextures, 1920, 1080, 880);
    updateFlowerPositionsParallel(parallelFlowers, flowerTextures, 1920, 1080, 880);
    bool flowersMatch = true;
    for (std::size_t index = 0; index < sequentialFlowers.size(); ++index) {
        flowersMatch = flowersMatch && sameRect(
            sequentialFlowers[index].dest, parallelFlowers[index].dest
        );
    }

    TulipTextures tulipTextures = {};
    tulipTextures.red.width = 12;
    tulipTextures.red.height = 12;
    tulipTextures.orange.width = 12;
    tulipTextures.orange.height = 12;
    std::vector<Tulip> sequentialTulips = createTulipField(4096);
    std::vector<Tulip> parallelTulips = sequentialTulips;
    updateTulipPositionsSequential(sequentialTulips, tulipTextures, 1920, 1080, 880);
    updateTulipPositionsParallel(parallelTulips, tulipTextures, 1920, 1080, 880);
    bool tulipsMatch = true;
    for (std::size_t index = 0; index < sequentialTulips.size(); ++index) {
        tulipsMatch = tulipsMatch && sameRect(
            sequentialTulips[index].placement.dest, parallelTulips[index].placement.dest
        );
    }

    CloudTextures cloudTextures = {};
    cloudTextures.width = 32;
    cloudTextures.height = 16;
    std::vector<Cloud> sequentialClouds = createCloudField(4096);
    std::vector<Cloud> parallelClouds = sequentialClouds;
    updateCloudPositionsSequential(sequentialClouds, cloudTextures, 1920, 1080, 0.016f);
    updateCloudPositionsParallel(parallelClouds, cloudTextures, 1920, 1080, 0.016f);
    bool cloudsMatch = true;
    for (std::size_t index = 0; index < sequentialClouds.size(); ++index) {
        cloudsMatch = cloudsMatch && sameFloat(
            sequentialClouds[index].x, parallelClouds[index].x
        ) && sameRect(sequentialClouds[index].dest, parallelClouds[index].dest);
    }

    LeafTextures leafTextures = {};
    leafTextures.width = 10;
    leafTextures.height = 10;
    const SDL_Rect treeDest = {760, 220, 420, 620};
    std::vector<Leaf> sequentialLeaves = createLeafField(4096);
    std::vector<Leaf> parallelLeaves = sequentialLeaves;
    updateLeavesSequential(
        sequentialLeaves, leafTextures, treeDest, LeafSeason::Autumn, 0.016f, 20000, 0.5f
    );
    updateLeavesParallel(
        parallelLeaves, leafTextures, treeDest, LeafSeason::Autumn, 0.016f, 20000, 0.5f
    );
    bool leavesMatch = true;
    for (std::size_t index = 0; index < sequentialLeaves.size(); ++index) {
        leavesMatch = leavesMatch && sameFloat(sequentialLeaves[index].y, parallelLeaves[index].y) &&
            sequentialLeaves[index].falling == parallelLeaves[index].falling &&
            sequentialLeaves[index].settled == parallelLeaves[index].settled &&
            sequentialLeaves[index].visible == parallelLeaves[index].visible &&
            sameRect(sequentialLeaves[index].dest, parallelLeaves[index].dest);
    }

    WeatherSystem sequentialWeather = {};
    WeatherSystem parallelWeather = {};
    sequentialWeather.spawnY = 0.20f;
    for (std::size_t index = 0; index < 4096; ++index) {
        const WeatherParticle particle = {
            static_cast<float>(index % 100) / 100.0f, 0.20f,
            250.0f + static_cast<float>(index % 20),
            static_cast<float>(static_cast<int>(index % 9) - 4), 0
        };
        sequentialWeather.particles.push_back(particle);
    }
    parallelWeather = sequentialWeather;
    updateWeatherSystemSequential(sequentialWeather, 1920, 1080, 0.016f, 1.0f);
    updateWeatherSystemParallel(parallelWeather, 1920, 1080, 0.016f, 1.0f);
    bool weatherMatch = true;
    for (std::size_t index = 0; index < sequentialWeather.particles.size(); ++index) {
        weatherMatch = weatherMatch && sameFloat(
            sequentialWeather.particles[index].x, parallelWeather.particles[index].x
        ) && sameFloat(sequentialWeather.particles[index].y, parallelWeather.particles[index].y);
    }

    const bool valid = flowersMatch && tulipsMatch && cloudsMatch && leavesMatch && weatherMatch;
    std::cout << "Validacion de consistencia" << std::endl
              << "Flores: " << (flowersMatch ? "OK" : "FALLO") << std::endl
              << "Tulipanes: " << (tulipsMatch ? "OK" : "FALLO") << std::endl
              << "Nubes: " << (cloudsMatch ? "OK" : "FALLO") << std::endl
              << "Hojas: " << (leavesMatch ? "OK" : "FALLO") << std::endl
              << "Lluvia: " << (weatherMatch ? "OK" : "FALLO") << std::endl;
    return valid ? 0 : 1;
}
