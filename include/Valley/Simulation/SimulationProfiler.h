#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Valley::Simulation {

struct SimulationProfileRecord {
    std::string name;
    double last_duration_ms = 0.0;
    double average_duration_ms = 0.0;
    double max_duration_ms = 0.0;
    unsigned long long sample_count = 0;
};

class SimulationProfiler {
public:
    void record(std::string_view name, double duration_ms);
    void reset();

    [[nodiscard]] std::vector<SimulationProfileRecord> snapshot() const;
    [[nodiscard]] bool empty() const;

private:
    struct Accumulator {
        double last_duration_ms = 0.0;
        double total_duration_ms = 0.0;
        double max_duration_ms = 0.0;
        unsigned long long sample_count = 0;
    };

    std::unordered_map<std::string, Accumulator> m_records;
};

class ScopedSimulationProfile {
public:
    ScopedSimulationProfile(SimulationProfiler& profiler, std::string_view name);
    ~ScopedSimulationProfile();

    ScopedSimulationProfile(const ScopedSimulationProfile&) = delete;
    ScopedSimulationProfile& operator=(const ScopedSimulationProfile&) = delete;

private:
    SimulationProfiler& m_profiler;
    std::string m_name;
    std::chrono::steady_clock::time_point m_started_at;
};

} // namespace Valley::Simulation
