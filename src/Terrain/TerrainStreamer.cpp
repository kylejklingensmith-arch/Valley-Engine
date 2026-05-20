#include "Valley/Terrain/TerrainStreamer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>
#include <utility>

namespace Valley::Terrain {
namespace {
[[nodiscard]] double chunk_center_distance(const World::ChunkCoord& coord, const World::WorldVec3& observer, double chunk_size)
{
    const double center_x = (static_cast<double>(coord.x) + 0.5) * chunk_size;
    const double center_z = (static_cast<double>(coord.z) + 0.5) * chunk_size;
    const double dx = center_x - observer.x;
    const double dz = center_z - observer.z;
    return std::sqrt(dx * dx + dz * dz);
}

[[nodiscard]] BiomeId dominant_biome(const TerrainMesh& mesh)
{
    std::array<std::size_t, 5> counts {};
    for (const auto& vertex : mesh.vertices) {
        ++counts[static_cast<std::size_t>(vertex.biome)];
    }
    const auto it = std::max_element(counts.begin(), counts.end());
    return static_cast<BiomeId>(std::distance(counts.begin(), it));
}
} // namespace

TerrainStreamer::TerrainStreamer(TerrainStreamerDesc desc)
    : m_desc(desc), m_mesher(TerrainGenerator({ .grid = desc.grid }))
{
}

TerrainStreamer::~TerrainStreamer()
{
    shutdown();
}

void TerrainStreamer::start()
{
    m_mesher.start();
}

void TerrainStreamer::shutdown()
{
    m_mesher.shutdown();
    m_loaded_meshes.clear();
    m_requested_meshes.clear();
}

void TerrainStreamer::sync_with_world(const World::WorldRegistry& registry, const World::WorldVec3& observer_position)
{
    for (const auto& record : registry.records_with_state(World::ChunkState::Loaded)) {
        if (m_loaded_meshes.contains(record.coord) || m_requested_meshes.contains(record.coord)) {
            continue;
        }
        const double distance = chunk_center_distance(record.coord, observer_position, m_desc.grid.chunk_size_meters);
        m_requested_meshes.insert(record.coord);
        m_mesher.request_mesh({ .coord = record.coord, .lod = terrain_lod_for_distance(distance, m_desc.grid.chunk_size_meters) });
    }

    for (auto it = m_loaded_meshes.begin(); it != m_loaded_meshes.end();) {
        const auto record = registry.find(it->first);
        if (!record || record->state != World::ChunkState::Loaded) {
            m_requested_meshes.erase(it->first);
            it = m_loaded_meshes.erase(it);
        } else {
            ++it;
        }
    }
}

void TerrainStreamer::service_completed_meshes()
{
    for (auto& mesh : m_mesher.collect_completed_meshes()) {
        m_requested_meshes.erase(mesh.coord);
        m_loaded_meshes.insert_or_assign(mesh.coord, std::move(mesh));
    }
}

void TerrainStreamer::wait_until_idle()
{
    m_mesher.wait_until_idle();
}

const std::unordered_map<World::ChunkCoord, TerrainMesh>& TerrainStreamer::loaded_meshes() const
{
    return m_loaded_meshes;
}

TerrainDebugSnapshot TerrainStreamer::debug_snapshot(int y_layer) const
{
    return {
        .requested_mesh_count = m_requested_meshes.size(),
        .loaded_mesh_count = m_loaded_meshes.size(),
        .biome_map = create_debug_biome_map(m_loaded_meshes, y_layer),
    };
}

std::string create_debug_biome_map(const std::unordered_map<World::ChunkCoord, TerrainMesh>& meshes, int y_layer)
{
    if (meshes.empty()) {
        return "<no terrain meshes>";
    }

    std::int64_t min_x = 0;
    std::int64_t max_x = 0;
    std::int64_t min_z = 0;
    std::int64_t max_z = 0;
    bool initialized = false;
    for (const auto& [coord, mesh] : meshes) {
        (void)mesh;
        if (coord.y != y_layer) {
            continue;
        }
        if (!initialized) {
            min_x = max_x = coord.x;
            min_z = max_z = coord.z;
            initialized = true;
        } else {
            min_x = std::min(min_x, coord.x);
            max_x = std::max(max_x, coord.x);
            min_z = std::min(min_z, coord.z);
            max_z = std::max(max_z, coord.z);
        }
    }

    if (!initialized) {
        return "<no terrain meshes on layer>";
    }

    std::ostringstream out;
    for (std::int64_t z = max_z; z >= min_z; --z) {
        for (std::int64_t x = min_x; x <= max_x; ++x) {
            const auto it = meshes.find({ .x = x, .y = y_layer, .z = z });
            out << (it == meshes.end() ? ' ' : debug_biome_glyph(dominant_biome(it->second)));
        }
        if (z != min_z) {
            out << '\n';
        }
    }
    return out.str();
}

} // namespace Valley::Terrain
