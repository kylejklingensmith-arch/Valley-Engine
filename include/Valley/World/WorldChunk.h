#pragma once

#include "Valley/World/WorldCoordinates.h"

#include <string>
#include <string_view>

namespace Valley::World {

enum class ChunkState {
    Requested,
    Loading,
    Loaded,
    UnloadRequested,
    Unloading,
};

struct ChunkRecord {
    ChunkCoord coord;
    ChunkState state = ChunkState::Requested;
    unsigned int revision = 0;
    std::string debug_name;
};

[[nodiscard]] std::string_view to_string(ChunkState state);

} // namespace Valley::World
