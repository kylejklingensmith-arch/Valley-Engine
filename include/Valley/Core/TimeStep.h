#pragma once

namespace Valley::Core {

struct TimeStep {
    double delta_seconds = 0.0;
    double elapsed_seconds = 0.0;
    double real_delta_seconds = 0.0;
    double real_elapsed_seconds = 0.0;
    double fixed_delta_seconds = 0.0;
    double time_scale = 1.0;
    unsigned int fixed_update_count = 0;
    unsigned long long frame_index = 0;
    unsigned long long fixed_frame_index = 0;
    bool paused = false;
    bool debug_step = false;
};

} // namespace Valley::Core
