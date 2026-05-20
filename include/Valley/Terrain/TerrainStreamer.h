#pragma once

#include "Valley/Terrain/AsyncTerrainMesher.h"
#include "Valley/World/WorldRegistry.h"

#include <unordered_map>
#include <unordered_set>

namespace Valley::Terrain {

struct TerrainStreamerDesc {
    World::WorldGridDesc grid;
};

struct TerrainDebugSnapshot {
    std::size_t requested_mesh_count = 0;
    std::size_t loaded_mesh_count = 0;
    std::string biome_map;
};

class TerrainStreamer {
public:
    explicit TerrainStreamer(TerrainStreamerDesc desc = {});
    ~TerrainStreamer();

    TerrainStreamer(const TerrainStreamer&) = delete;
    TerrainStreamer& operator=(const TerrainStreamer&) = delete;

    void start();
    void shutdown();
    void sync_with_world(const World::WorldRegistry& registry, const World::WorldVec3& observer_position = {});
    void service_completed_meshes();
    void wait_until_idle();

    [[nodiscard]] const std::unordered_map<World::ChunkCoord, TerrainMesh>& loaded_meshes() const;
    [[nodiscard]] TerrainDebugSnapshot debug_snapshot(int y_layer = 0) const;

private:
    TerrainStreamerDesc m_desc;
    AsyncTerrainMesher m_mesher;
    std::unordered_map<World::ChunkCoord, TerrainMesh> m_loaded_meshes;
    std::unordered_set<World::ChunkCoord> m_requested_meshes;
};

[[nodiscard]] std::string create_debug_biome_map(const std::unordered_map<World::ChunkCoord, TerrainMesh>& meshes, int y_layer = 0);

} // namespace Valley::Terrain
