#pragma once

#include "Valley/Core/Module.h"
#include "Valley/Terrain/TerrainStreamer.h"
#include "Valley/World/FloatingOrigin.h"
#include "Valley/World/WorldStreamer.h"

namespace Valley::World {

class WorldModule final : public Core::Module {
public:
    [[nodiscard]] std::string_view name() const override;
    void on_attach() override;
    void on_update(const Core::TimeStep& step) override;
    void on_detach() override;

    [[nodiscard]] const WorldStreamer& streamer() const;
    [[nodiscard]] const FloatingOrigin& floating_origin() const;
    [[nodiscard]] const Terrain::TerrainStreamer& terrain_streamer() const;

private:
    WorldStreamer m_streamer { WorldStreamerDesc { .grid = { .chunk_size_meters = 256.0 }, .load_radius_chunks = 1 } };
    FloatingOrigin m_floating_origin { FloatingOriginDesc { .rebase_threshold_meters = 4096.0 } };
    Terrain::TerrainStreamer m_terrain_streamer { Terrain::TerrainStreamerDesc { .grid = { .chunk_size_meters = 256.0 } } };
    WorldVec3 m_observer_position;
    std::size_t m_last_debug_loaded_count = 0;
    std::size_t m_last_terrain_mesh_count = 0;
};

} // namespace Valley::World
