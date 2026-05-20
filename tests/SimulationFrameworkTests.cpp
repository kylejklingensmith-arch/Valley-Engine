#include "Valley/Core/EngineClock.h"
#include "Valley/Simulation/SimulationScheduler.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

using namespace Valley;

namespace {
class RecordingSystem final : public Simulation::SimulationSystem {
public:
    explicit RecordingSystem(std::string_view system_name)
        : m_name(system_name)
    {
    }

    [[nodiscard]] std::string_view name() const override
    {
        return m_name;
    }

    void fixed_update(const Simulation::SimulationTick& tick) override
    {
        ticks.push_back(tick);
    }

    std::vector<Simulation::SimulationTick> ticks;

private:
    std::string m_name;
};

void scheduler_runs_systems_in_fixed_tick_order()
{
    Core::EngineClock clock({ .fixed_timestep_seconds = 0.10 });
    Simulation::SimulationScheduler scheduler;
    auto recorder = std::make_unique<RecordingSystem>("Recorder");
    auto* recorder_ptr = recorder.get();
    scheduler.add_system(std::move(recorder));

    const auto frame = clock.advance(0.35);
    assert(frame.fixed_update_count == 3);

    for (unsigned int index = 0; index < frame.fixed_update_count; ++index) {
        Core::TimeStep fixed_step = frame;
        fixed_step.delta_seconds = frame.fixed_delta_seconds;
        fixed_step.fixed_frame_index = frame.fixed_frame_index + index;
        scheduler.fixed_update(fixed_step);
    }

    assert(recorder_ptr->ticks.size() == 3);
    assert(recorder_ptr->ticks[0].tick_index == 0);
    assert(recorder_ptr->ticks[1].tick_index == 1);
    assert(recorder_ptr->ticks[2].tick_index == 2);
    assert(scheduler.tick_count() == 3);
}

void deterministic_seeds_are_stable_per_tick()
{
    Simulation::SimulationScheduler first({ .deterministic_seed = 42 });
    Simulation::SimulationScheduler second({ .deterministic_seed = 42 });
    Simulation::SimulationScheduler different({ .deterministic_seed = 84 });

    assert(first.deterministic_seed_for_tick(100) == second.deterministic_seed_for_tick(100));
    assert(first.deterministic_seed_for_tick(100) != first.deterministic_seed_for_tick(101));
    assert(first.deterministic_seed_for_tick(100) != different.deterministic_seed_for_tick(100));
}

void paused_debug_steps_feed_scheduler()
{
    Core::EngineClock clock({ .fixed_timestep_seconds = 0.25, .start_paused = true, .queued_debug_steps = 2 });
    Simulation::SimulationScheduler scheduler;
    auto recorder = std::make_unique<RecordingSystem>("DebugRecorder");
    auto* recorder_ptr = recorder.get();
    scheduler.add_system(std::move(recorder));

    const auto frame = clock.advance(0.0);
    assert(frame.paused);
    assert(frame.debug_step);
    assert(frame.fixed_update_count == 2);

    for (unsigned int index = 0; index < frame.fixed_update_count; ++index) {
        Core::TimeStep fixed_step = frame;
        fixed_step.delta_seconds = frame.fixed_delta_seconds;
        fixed_step.fixed_frame_index = frame.fixed_frame_index + index;
        scheduler.fixed_update(fixed_step);
    }

    assert(recorder_ptr->ticks.size() == 2);
    assert(recorder_ptr->ticks[0].debug_step);
    assert(recorder_ptr->ticks[1].debug_step);
}

void profiler_records_tick_and_system_work()
{
    Simulation::SimulationScheduler scheduler;
    scheduler.add_system(std::make_unique<RecordingSystem>("ProfiledSystem"));
    scheduler.fixed_update({ .fixed_delta_seconds = 0.016, .fixed_frame_index = 0 });

    const auto profile = scheduler.profiler().snapshot();
    bool saw_tick = false;
    bool saw_system = false;
    for (const auto& record : profile) {
        if (record.name == "SimulationTick") {
            saw_tick = record.sample_count == 1;
        }
        if (record.name == "ProfiledSystem") {
            saw_system = record.sample_count == 1;
        }
    }

    assert(saw_tick);
    assert(saw_system);
}
} // namespace

int main()
{
    scheduler_runs_systems_in_fixed_tick_order();
    deterministic_seeds_are_stable_per_tick();
    paused_debug_steps_feed_scheduler();
    profiler_records_tick_and_system_work();
    std::cout << "Simulation framework tests passed.\n";
    return 0;
}
