#include "Valley/Simulation/SimulationScheduler.h"

#include <stdexcept>
#include <string>
#include <utility>

namespace Valley::Simulation {
namespace {
[[nodiscard]] std::uint64_t splitmix64(std::uint64_t value)
{
    value += 0x9E3779B97F4A7C15ULL;
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
}
} // namespace

SimulationScheduler::SimulationScheduler(SimulationSchedulerDesc desc)
    : m_desc(desc)
{
}

void SimulationScheduler::add_system(std::unique_ptr<SimulationSystem> system)
{
    if (!system) {
        throw std::invalid_argument("Cannot add a null simulation system.");
    }
    m_systems.push_back(std::move(system));
}

void SimulationScheduler::fixed_update(const Core::TimeStep& step)
{
    const SimulationTick tick {
        .tick_index = step.fixed_frame_index,
        .delta_seconds = step.fixed_delta_seconds,
        .elapsed_seconds = static_cast<double>(step.fixed_frame_index + 1ULL) * step.fixed_delta_seconds,
        .deterministic_seed = deterministic_seed_for_tick(step.fixed_frame_index),
        .debug_step = step.debug_step,
    };

    ScopedSimulationProfile tick_profile(m_profiler, "SimulationTick");
    for (auto& system : m_systems) {
        ScopedSimulationProfile system_profile(m_profiler, system->name());
        system->fixed_update(tick);
    }

    m_tick_count = step.fixed_frame_index + 1ULL;
}

void SimulationScheduler::reset()
{
    m_tick_count = 0;
    m_profiler.reset();
}

unsigned long long SimulationScheduler::tick_count() const
{
    return m_tick_count;
}

const std::vector<std::unique_ptr<SimulationSystem>>& SimulationScheduler::systems() const
{
    return m_systems;
}

const SimulationProfiler& SimulationScheduler::profiler() const
{
    return m_profiler;
}

SimulationProfiler& SimulationScheduler::profiler()
{
    return m_profiler;
}

std::uint64_t SimulationScheduler::deterministic_seed_for_tick(unsigned long long tick_index) const
{
    return splitmix64(m_desc.deterministic_seed ^ static_cast<std::uint64_t>(tick_index));
}

} // namespace Valley::Simulation
