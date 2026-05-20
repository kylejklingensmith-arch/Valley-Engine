#include "Valley/Core/EngineClock.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Valley::Core {
namespace {
constexpr double min_fixed_timestep_seconds = 0.000001;
constexpr double max_time_scale = 100.0;
constexpr double fixed_timestep_epsilon = 0.000000000001;

void validate_finite_non_negative(double value, const char* name)
{
    if (!std::isfinite(value) || value < 0.0) {
        throw std::invalid_argument(name);
    }
}
} // namespace

EngineClock::EngineClock(const EngineClockDesc& desc)
    : m_fixed_timestep_seconds(desc.fixed_timestep_seconds), m_time_scale(desc.time_scale), m_queued_debug_steps(desc.queued_debug_steps), m_paused(desc.start_paused)
{
    set_fixed_timestep(desc.fixed_timestep_seconds);
    set_time_scale(desc.time_scale);
}

TimeStep EngineClock::advance(double real_delta_seconds)
{
    validate_finite_non_negative(real_delta_seconds, "EngineClock real delta must be finite and non-negative.");

    m_real_elapsed_seconds += real_delta_seconds;

    const unsigned int debug_step_count = m_queued_debug_steps;
    m_queued_debug_steps = 0;

    const double scaled_delta_seconds = m_paused ? 0.0 : real_delta_seconds * m_time_scale;
    const double debug_step_seconds = static_cast<double>(debug_step_count) * m_fixed_timestep_seconds;
    m_simulation_elapsed_seconds += scaled_delta_seconds + debug_step_seconds;
    m_fixed_accumulator_seconds += scaled_delta_seconds;

    unsigned int fixed_update_count = debug_step_count;
    while (m_fixed_accumulator_seconds + fixed_timestep_epsilon >= m_fixed_timestep_seconds) {
        m_fixed_accumulator_seconds = std::max(0.0, m_fixed_accumulator_seconds - m_fixed_timestep_seconds);
        ++fixed_update_count;
    }

    const TimeStep step {
        .delta_seconds = scaled_delta_seconds + debug_step_seconds,
        .elapsed_seconds = m_simulation_elapsed_seconds,
        .real_delta_seconds = real_delta_seconds,
        .real_elapsed_seconds = m_real_elapsed_seconds,
        .fixed_delta_seconds = m_fixed_timestep_seconds,
        .time_scale = m_time_scale,
        .fixed_update_count = fixed_update_count,
        .frame_index = m_frame_index,
        .fixed_frame_index = m_fixed_frame_index,
        .paused = m_paused,
        .debug_step = debug_step_count > 0,
    };

    m_fixed_frame_index += fixed_update_count;
    ++m_frame_index;

    return step;
}

void EngineClock::pause()
{
    set_paused(true);
}

void EngineClock::resume()
{
    set_paused(false);
}

void EngineClock::set_paused(bool paused)
{
    m_paused = paused;
}

void EngineClock::set_time_scale(double scale)
{
    validate_finite_non_negative(scale, "EngineClock time scale must be finite and non-negative.");
    m_time_scale = std::min(scale, max_time_scale);
}

void EngineClock::set_fixed_timestep(double fixed_timestep_seconds)
{
    if (!std::isfinite(fixed_timestep_seconds) || fixed_timestep_seconds < min_fixed_timestep_seconds) {
        throw std::invalid_argument("EngineClock fixed timestep must be finite and greater than zero.");
    }

    m_fixed_timestep_seconds = fixed_timestep_seconds;
}

void EngineClock::reset()
{
    m_real_elapsed_seconds = 0.0;
    m_simulation_elapsed_seconds = 0.0;
    m_fixed_accumulator_seconds = 0.0;
    m_frame_index = 0;
    m_fixed_frame_index = 0;
    m_queued_debug_steps = 0;
}

void EngineClock::request_debug_step()
{
    request_debug_steps(1);
}

void EngineClock::request_debug_steps(unsigned int step_count)
{
    m_queued_debug_steps += step_count;
}

bool EngineClock::is_paused() const
{
    return m_paused;
}

double EngineClock::time_scale() const
{
    return m_time_scale;
}

double EngineClock::fixed_timestep_seconds() const
{
    return m_fixed_timestep_seconds;
}

double EngineClock::real_elapsed_seconds() const
{
    return m_real_elapsed_seconds;
}

double EngineClock::simulation_elapsed_seconds() const
{
    return m_simulation_elapsed_seconds;
}

double EngineClock::fixed_accumulator_seconds() const
{
    return m_fixed_accumulator_seconds;
}

unsigned long long EngineClock::frame_index() const
{
    return m_frame_index;
}

unsigned long long EngineClock::fixed_frame_index() const
{
    return m_fixed_frame_index;
}

unsigned int EngineClock::queued_debug_steps() const
{
    return m_queued_debug_steps;
}

} // namespace Valley::Core
