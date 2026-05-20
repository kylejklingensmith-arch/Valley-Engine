#pragma once

#include "Valley/Core/TimeStep.h"
#include "Valley/Simulation/SimulationProfiler.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace Valley::Simulation {

struct SimulationTick {
    unsigned long long tick_index = 0;
    double delta_seconds = 0.0;
    double elapsed_seconds = 0.0;
    std::uint64_t deterministic_seed = 0;
    bool debug_step = false;
};

class SimulationSystem {
public:
    virtual ~SimulationSystem() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;
    virtual void fixed_update(const SimulationTick& tick) = 0;
};

struct SimulationSchedulerDesc {
    std::uint64_t deterministic_seed = 0xC001D00D5EEDULL;
};

class SimulationScheduler {
public:
    explicit SimulationScheduler(SimulationSchedulerDesc desc = {});

    void add_system(std::unique_ptr<SimulationSystem> system);
    void fixed_update(const Core::TimeStep& step);
    void reset();

    [[nodiscard]] unsigned long long tick_count() const;
    [[nodiscard]] const std::vector<std::unique_ptr<SimulationSystem>>& systems() const;
    [[nodiscard]] const SimulationProfiler& profiler() const;
    [[nodiscard]] SimulationProfiler& profiler();
    [[nodiscard]] std::uint64_t deterministic_seed_for_tick(unsigned long long tick_index) const;

private:
    SimulationSchedulerDesc m_desc;
    std::vector<std::unique_ptr<SimulationSystem>> m_systems;
    SimulationProfiler m_profiler;
    unsigned long long m_tick_count = 0;
};

} // namespace Valley::Simulation
