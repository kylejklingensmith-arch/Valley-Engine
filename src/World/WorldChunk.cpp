#include "Valley/World/WorldChunk.h"

namespace Valley::World {

std::string_view to_string(ChunkState state)
{
    switch (state) {
    case ChunkState::Requested:
        return "Requested";
    case ChunkState::Loading:
        return "Loading";
    case ChunkState::Loaded:
        return "Loaded";
    case ChunkState::UnloadRequested:
        return "UnloadRequested";
    case ChunkState::Unloading:
        return "Unloading";
    }

    return "Unknown";
}

} // namespace Valley::World
