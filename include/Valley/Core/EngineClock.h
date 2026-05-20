#pragma once

#include "Valley/Core/TimeStep.h"

namespace Valley::Core {

struct EngineClockDesc {
    double fixed_timestep_seconds = 1.0 / 60.0;
    double time_scale = 1.0;
    bool start_paused = false;
    unsigned int queued_debug_steps = 0;
};

class EngineClock {
public:
    explicit EngineClock(const EngineClockDesc& desc = {});

    [[nodiscard]] TimeStep advance(double real_delta_seconds);

    void pause();
    void resume();
    void set_paused(bool paused);
    void set_time_scale(double scale);
    void set_fixed_timestep(double fixed_timestep_seconds);
    void reset();
    void request_debug_step();
    void request_debug_steps(unsigned int step_count);

    [[nodiscard]] bool is_paused() const;
    [[nodiscard]] double time_scale() const;
    [[nodiscard]] double fixed_timestep_seconds() const;
    [[nodiscard]] double real_elapsed_seconds() const;
    [[nodiscard]] double simulation_elapsed_seconds() const;
    [[nodiscard]] double fixed_accumulator_seconds() const;
    [[nodiscard]] unsigned long long frame_index() const;
    [[nodiscard]] unsigned long long fixed_frame_index() const;
    [[nodiscard]] unsigned int queued_debug_steps() const;

private:
    double m_fixed_timestep_seconds = 1.0 / 60.0;
    double m_time_scale = 1.0;
    double m_real_elapsed_seconds = 0.0;
    double m_simulation_elapsed_seconds = 0.0;
    double m_fixed_accumulator_seconds = 0.0;
    unsigned long long m_frame_index = 0;
    unsigned long long m_fixed_frame_index = 0;
    unsigned int m_queued_debug_steps = 0;
    bool m_paused = false;
};

} // namespace Valley::Core
