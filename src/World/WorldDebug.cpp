#include "Valley/World/WorldDebug.h"

#include <algorithm>
#include <sstream>

namespace Valley::World {

WorldDebugSnapshot create_debug_snapshot(const WorldRegistry& registry, int y_layer)
{
    return {
        .requested_count = registry.count(ChunkState::Requested),
        .loading_count = registry.count(ChunkState::Loading),
        .loaded_count = registry.count(ChunkState::Loaded),
        .unloading_count = registry.count(ChunkState::UnloadRequested) + registry.count(ChunkState::Unloading),
        .loaded_chunk_map = create_loaded_chunk_ascii_map(registry, y_layer),
    };
}

std::string create_loaded_chunk_ascii_map(const WorldRegistry& registry, int y_layer)
{
    const auto loaded = registry.records_with_state(ChunkState::Loaded);
    if (loaded.empty()) {
        return "<no loaded chunks>";
    }

    std::int64_t min_x = 0;
    std::int64_t max_x = 0;
    std::int64_t min_z = 0;
    std::int64_t max_z = 0;
    bool found_layer = false;

    for (const auto& record : loaded) {
        if (record.coord.y != y_layer) {
            continue;
        }

        if (!found_layer) {
            min_x = max_x = record.coord.x;
            min_z = max_z = record.coord.z;
            found_layer = true;
            continue;
        }

        min_x = std::min(min_x, record.coord.x);
        max_x = std::max(max_x, record.coord.x);
        min_z = std::min(min_z, record.coord.z);
        max_z = std::max(max_z, record.coord.z);
    }

    if (!found_layer) {
        return "<no loaded chunks on layer>";
    }

    std::ostringstream stream;
    stream << "loaded chunks y=" << y_layer << '\n';
    for (std::int64_t z = min_z; z <= max_z; ++z) {
        for (std::int64_t x = min_x; x <= max_x; ++x) {
            const auto record = registry.find({ .x = x, .y = y_layer, .z = z });
            stream << (record && record->state == ChunkState::Loaded ? '#' : '.');
        }
        if (z != max_z) {
            stream << '\n';
        }
    }

    return stream.str();
}

} // namespace Valley::World
