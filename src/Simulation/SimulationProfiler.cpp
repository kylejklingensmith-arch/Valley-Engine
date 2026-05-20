#include "Valley/Simulation/SimulationProfiler.h"

#include <algorithm>
#include <utility>

namespace Valley::Simulation {

void SimulationProfiler::record(std::string_view name, double duration_ms)
{
    auto& accumulator = m_records[std::string(name)];
    accumulator.last_duration_ms = duration_ms;
    accumulator.total_duration_ms += duration_ms;
    accumulator.max_duration_ms = std::max(accumulator.max_duration_ms, duration_ms);
    ++accumulator.sample_count;
}

void SimulationProfiler::reset()
{
    m_records.clear();
}

std::vector<SimulationProfileRecord> SimulationProfiler::snapshot() const
{
    std::vector<SimulationProfileRecord> records;
    records.reserve(m_records.size());

    for (const auto& [name, accumulator] : m_records) {
        records.push_back({
            .name = name,
            .last_duration_ms = accumulator.last_duration_ms,
            .average_duration_ms = accumulator.sample_count == 0 ? 0.0 : accumulator.total_duration_ms / static_cast<double>(accumulator.sample_count),
            .max_duration_ms = accumulator.max_duration_ms,
            .sample_count = accumulator.sample_count,
        });
    }

    std::sort(records.begin(), records.end(), [](const SimulationProfileRecord& lhs, const SimulationProfileRecord& rhs) {
        return lhs.name < rhs.name;
    });
    return records;
}

bool SimulationProfiler::empty() const
{
    return m_records.empty();
}

ScopedSimulationProfile::ScopedSimulationProfile(SimulationProfiler& profiler, std::string_view name)
    : m_profiler(profiler), m_name(name), m_started_at(std::chrono::steady_clock::now())
{
}

ScopedSimulationProfile::~ScopedSimulationProfile()
{
    const auto finished_at = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> duration = finished_at - m_started_at;
    m_profiler.record(m_name, duration.count());
}

} // namespace Valley::Simulation
