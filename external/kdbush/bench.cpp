#include "include/kdbush.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <vector>

int main() {
    std::mt19937 gen(0);
    std::uniform_int_distribution<> dis(-10000, 10000);

    using Point = std::pair<int, int>;

    const std::size_t num_points = 1000000;

    std::vector<Point> points;
    points.reserve(num_points);

    for (std::size_t i = 0; i < num_points; i++) {
        points.emplace_back(dis(gen), dis(gen));
    }

    const auto started = std::chrono::high_resolution_clock::now();
    kdbush::KDBush<Point> index(points);
    const auto finished = std::chrono::high_resolution_clock::now();

    const auto duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(finished - started).count();

    std::cerr << "indexed 1M points in " << (duration / 1e6) << "ms\n";

    return 0;
}
