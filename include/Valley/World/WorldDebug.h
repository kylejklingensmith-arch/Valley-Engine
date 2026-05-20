#pragma once

#include "Valley/World/WorldRegistry.h"

#include <string>

namespace Valley::World {

struct WorldDebugSnapshot {
    std::size_t requested_count = 0;
    std::size_t loading_count = 0;
    std::size_t loaded_count = 0;
    std::size_t unloading_count = 0;
    std::string loaded_chunk_map;
};

[[nodiscard]] WorldDebugSnapshot create_debug_snapshot(const WorldRegistry& registry, int y_layer = 0);
[[nodiscard]] std::string create_loaded_chunk_ascii_map(const WorldRegistry& registry, int y_layer = 0);

} // namespace Valley::World
