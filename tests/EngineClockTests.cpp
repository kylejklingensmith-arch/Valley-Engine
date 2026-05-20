#include "Valley/Core/EngineClock.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

void require_near(double actual, double expected, std::string_view message)
{
    constexpr double epsilon = 0.000000001;
    if (std::fabs(actual - expected) > epsilon) {
        std::cerr << message << ": expected " << expected << ", got " << actual << '\n';
        throw std::runtime_error(std::string(message));
    }
}

void advances_real_and_simulation_time_from_one_clock()
{
    Valley::Core::EngineClock clock({ .fixed_timestep_seconds = 0.25 });

    const auto first = clock.advance(0.10);
    require_near(first.real_delta_seconds, 0.10, "real delta is reported");
    require_near(first.delta_seconds, 0.10, "simulation delta follows real time at scale 1");
    require_near(first.real_elapsed_seconds, 0.10, "real elapsed advances");
    require_near(first.elapsed_seconds, 0.10, "simulation elapsed advances");
    require(first.fixed_update_count == 0, "fixed update does not run before accumulator reaches the step");
    require(first.frame_index == 0, "first frame index starts at zero");

    const auto second = clock.advance(0.15);
    require_near(second.real_elapsed_seconds, 0.25, "real elapsed accumulates");
    require_near(second.elapsed_seconds, 0.25, "simulation elapsed accumulates");
    require(second.fixed_update_count == 1, "fixed update runs once at the configured step");
    require(second.fixed_frame_index == 0, "first fixed frame index starts at zero");
    require(clock.fixed_frame_index() == 1, "clock tracks completed fixed updates");
}

void pause_blocks_simulation_but_not_real_time()
{
    Valley::Core::EngineClock clock({ .fixed_timestep_seconds = 0.10 });

    [[maybe_unused]] const auto initial = clock.advance(0.20);
    clock.pause();
    const auto paused = clock.advance(0.50);

    require(paused.paused, "paused flag is reported");
    require_near(paused.real_delta_seconds, 0.50, "real delta continues while paused");
    require_near(paused.delta_seconds, 0.0, "simulation delta is zero while paused");
    require_near(paused.real_elapsed_seconds, 0.70, "real elapsed continues while paused");
    require_near(paused.elapsed_seconds, 0.20, "simulation elapsed stops while paused");
    require(paused.fixed_update_count == 0, "fixed updates do not run while paused");

    clock.resume();
    const auto resumed = clock.advance(0.10);
    require(!resumed.paused, "resume clears paused flag");
    require_near(resumed.delta_seconds, 0.10, "simulation delta resumes");
    require(resumed.fixed_update_count == 1, "fixed updates resume");
}

void time_scaling_controls_simulation_progression()
{
    Valley::Core::EngineClock clock({ .fixed_timestep_seconds = 0.10, .time_scale = 0.5 });

    const auto slow = clock.advance(0.40);
    require_near(slow.real_delta_seconds, 0.40, "real delta is unscaled");
    require_near(slow.delta_seconds, 0.20, "simulation delta is scaled");
    require_near(slow.elapsed_seconds, 0.20, "simulation elapsed is scaled");
    require(slow.fixed_update_count == 2, "fixed updates are based on scaled time");

    clock.set_time_scale(2.0);
    const auto fast = clock.advance(0.05);
    require_near(fast.delta_seconds, 0.10, "time scale can speed up simulation");
    require(fast.fixed_update_count == 1, "fixed updates use the latest scale");
}

void paused_debug_steps_advance_only_fixed_simulation()
{
    Valley::Core::EngineClock clock({ .fixed_timestep_seconds = 0.25, .start_paused = true });
    clock.request_debug_steps(3);

    const auto stepped = clock.advance(1.0);
    require(stepped.paused, "debug stepping preserves paused state");
    require(stepped.debug_step, "debug step flag is reported");
    require_near(stepped.real_elapsed_seconds, 1.0, "real time still advances during debug stepping");
    require_near(stepped.delta_seconds, 0.75, "debug steps advance simulation by fixed ticks");
    require_near(stepped.elapsed_seconds, 0.75, "debug steps advance authoritative simulation elapsed time");
    require(stepped.fixed_update_count == 3, "queued debug steps dispatch fixed updates while paused");
    require(clock.fixed_frame_index() == 3, "debug steps increment the fixed frame index");
    require(clock.queued_debug_steps() == 0, "debug steps are consumed after advance");
}

void invalid_clock_values_are_rejected()
{
    bool rejected_fixed_step = false;
    try {
        Valley::Core::EngineClock clock({ .fixed_timestep_seconds = 0.0 });
    } catch (const std::invalid_argument&) {
        rejected_fixed_step = true;
    }

    bool rejected_time_scale = false;
    try {
        Valley::Core::EngineClock clock;
        clock.set_time_scale(-1.0);
    } catch (const std::invalid_argument&) {
        rejected_time_scale = true;
    }

    require(rejected_fixed_step, "invalid fixed timestep is rejected");
    require(rejected_time_scale, "invalid time scale is rejected");
}

} // namespace

int main()
{
    try {
        advances_real_and_simulation_time_from_one_clock();
        pause_blocks_simulation_but_not_real_time();
        time_scaling_controls_simulation_progression();
        paused_debug_steps_advance_only_fixed_simulation();
        invalid_clock_values_are_rejected();
    } catch (const std::exception& exception) {
        std::cerr << "EngineClockTests failed: " << exception.what() << '\n';
        return 1;
    }

    std::cout << "EngineClockTests passed.\n";
    return 0;
}
