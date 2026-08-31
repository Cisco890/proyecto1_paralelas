#ifndef PERFORMANCE_HPP
#define PERFORMANCE_HPP

#include <cstddef>

enum class UpdateExecutionMode {
    Sequential,
    Parallel,
    Compare
};

struct BenchmarkSummary {
    const char* label;
    std::size_t elements;
    std::size_t iterations;
    double sequentialMilliseconds;
    double parallelMilliseconds;
};

int runPerformanceBenchmark(UpdateExecutionMode mode);

#endif
