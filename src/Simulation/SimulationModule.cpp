#include "Valley/Simulation/SimulationModule.h"

#include "Valley/Core/Logger.h"

#include <sstream>

namespace Valley::Simulation {

std::string_view SimulationModule::name() const
{
    return "Simulation";
}

void SimulationModule::on_attach()
{
    Core::Log::info("Simulation", "Simulation module attached with deterministic fixed-timestep scheduler. Gameplay rules belong in a later game layer.");
}

void SimulationModule::on_fixed_update(const Core::TimeStep& step)
{
    m_scheduler.fixed_update(step);
}

void SimulationModule::on_update(const Core::TimeStep& step)
{
    if (m_scheduler.tick_count() == m_last_reported_tick_count || m_scheduler.profiler().empty()) {
        return;
    }

    std::ostringstream message;
    message << "Simulation debug: ticks=" << m_scheduler.tick_count()
            << ", fixed_dt=" << step.fixed_delta_seconds
            << ", paused=" << (step.paused ? "true" : "false")
            << ", time_scale=" << step.time_scale;

    for (const auto& record : m_scheduler.profiler().snapshot()) {
        message << "\n  profile " << record.name
                << ": last=" << record.last_duration_ms
                << "ms avg=" << record.average_duration_ms
                << "ms max=" << record.max_duration_ms
                << "ms samples=" << record.sample_count;
    }

    Core::Log::info("Simulation", message.str());
    m_last_reported_tick_count = m_scheduler.tick_count();
}

void SimulationModule::on_detach()
{
    Core::Log::info("Simulation", "Simulation module detached.");
}

const SimulationScheduler& SimulationModule::scheduler() const
{
    return m_scheduler;
}

SimulationScheduler& SimulationModule::scheduler()
{
    return m_scheduler;
}

} // namespace Valley::Simulation
