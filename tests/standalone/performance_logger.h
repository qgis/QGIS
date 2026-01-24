/**
 * Performance Logger for GPU Rendering
 * Can be integrated into QGIS for real-time performance monitoring
 */

#ifndef QGIS_PERFORMANCE_LOGGER_H
#define QGIS_PERFORMANCE_LOGGER_H

#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>

namespace QgsGPUPerformance {

struct RenderStats {
    std::chrono::milliseconds gpu_time;
    std::chrono::milliseconds cpu_time;
    int tiles_rendered;
    std::string renderer_type; // "GPU" or "CPU"
    std::chrono::system_clock::time_point timestamp;
};

class PerformanceLogger {
public:
    static PerformanceLogger& instance() {
        static PerformanceLogger logger;
        return logger;
    }

    void logRender(const RenderStats& stats) {
        mHistory.push_back(stats);

        // Keep last 100 frames
        if (mHistory.size() > 100) {
            mHistory.erase(mHistory.begin());
        }

        // Calculate running average FPS
        updateFPS();
    }

    double getCurrentFPS() const {
        return mCurrentFPS;
    }

    double getAverageSpeedup() const {
        if (mHistory.empty()) return 1.0;

        std::vector<double> speedups;
        for (const auto& stat : mHistory) {
            if (stat.renderer_type == "GPU" && stat.cpu_time.count() > 0) {
                double speedup = static_cast<double>(stat.cpu_time.count()) /
                                 static_cast<double>(stat.gpu_time.count());
                speedups.push_back(speedup);
            }
        }

        if (speedups.empty()) return 1.0;

        return std::accumulate(speedups.begin(), speedups.end(), 0.0) / speedups.size();
    }

    void printSummary() const {
        if (mHistory.empty()) {
            std::cout << "No performance data collected" << std::endl;
            return;
        }

        std::cout << "\n=== GPU Rendering Performance Summary ===" << std::endl;
        std::cout << "Frames logged: " << mHistory.size() << std::endl;
        std::cout << "Current FPS: " << mCurrentFPS << std::endl;
        std::cout << "Average GPU speedup: " << getAverageSpeedup() << "x" << std::endl;

        // Calculate percentiles
        std::vector<int> render_times;
        for (const auto& stat : mHistory) {
            render_times.push_back(stat.gpu_time.count());
        }

        std::sort(render_times.begin(), render_times.end());
        size_t p50 = render_times.size() / 2;
        size_t p95 = static_cast<size_t>(render_times.size() * 0.95);
        size_t p99 = static_cast<size_t>(render_times.size() * 0.99);

        std::cout << "Render time p50: " << render_times[p50] << "ms" << std::endl;
        std::cout << "Render time p95: " << render_times[p95] << "ms" << std::endl;
        std::cout << "Render time p99: " << render_times[p99] << "ms" << std::endl;
    }

    void exportCSV(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << filename << std::endl;
            return;
        }

        file << "timestamp,renderer_type,tiles,gpu_time_ms,cpu_time_ms,speedup\n";

        for (const auto& stat : mHistory) {
            auto epoch = std::chrono::system_clock::to_time_t(stat.timestamp);
            double speedup = stat.cpu_time.count() > 0 ?
                static_cast<double>(stat.cpu_time.count()) / stat.gpu_time.count() : 0.0;

            file << epoch << ","
                 << stat.renderer_type << ","
                 << stat.tiles_rendered << ","
                 << stat.gpu_time.count() << ","
                 << stat.cpu_time.count() << ","
                 << speedup << "\n";
        }

        file.close();
        std::cout << "Performance data exported to " << filename << std::endl;
    }

private:
    PerformanceLogger() : mCurrentFPS(0.0) {}

    void updateFPS() {
        if (mHistory.size() < 2) return;

        // Calculate FPS from last 10 frames
        size_t count = std::min(size_t(10), mHistory.size());
        auto total_time = std::chrono::milliseconds(0);

        for (size_t i = mHistory.size() - count; i < mHistory.size(); ++i) {
            total_time += mHistory[i].gpu_time;
        }

        if (total_time.count() > 0) {
            mCurrentFPS = (count * 1000.0) / total_time.count();
        }
    }

    std::vector<RenderStats> mHistory;
    double mCurrentFPS;
};

} // namespace QgsGPUPerformance

#endif // QGIS_PERFORMANCE_LOGGER_H
