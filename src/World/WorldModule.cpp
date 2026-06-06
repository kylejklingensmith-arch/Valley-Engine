#include "Valley/World/WorldModule.h"

#include "Valley/Core/Logger.h"

#include <chrono>
#include <sstream>

namespace Valley::World {

std::string_view WorldModule::name() const
{
    return "World";
}

void WorldModule::on_attach()
{
    Core::Log::info("World", "World module attached with chunk streaming and procedural terrain infrastructure; no final terrain visuals or gameplay content are generated.");
    m_streamer.start();
    m_terrain_streamer.start();
    m_streamer.update_observer(m_observer_position);
}

void WorldModule::on_update(const Core::TimeStep&)
{
    const auto chunk_start = std::chrono::steady_clock::now();
    m_streamer.service_completed_requests();
    const auto chunk_end = std::chrono::steady_clock::now();
    m_chunk_timing.record("world_streaming", std::chrono::duration<double, std::milli>(chunk_end - chunk_start).count());

    const auto terrain_start = std::chrono::steady_clock::now();
    m_terrain_streamer.sync_with_world(m_streamer.registry(), m_observer_position);
    m_terrain_streamer.service_completed_meshes();
    const auto terrain_end = std::chrono::steady_clock::now();
    m_terrain_timing.record("terrain_generation", std::chrono::duration<double, std::milli>(terrain_end - terrain_start).count());

    const auto rebase = m_floating_origin.update(m_observer_position);
    if (rebase.rebased) {
        Core::Log::info("World", "Floating origin rebase prepared for large-world precision.");
    }

    const auto snapshot = create_debug_snapshot(m_streamer.registry());
    if (snapshot.loaded_count != m_last_debug_loaded_count) {
        std::ostringstream message;
        message << "Chunk debug: loaded=" << snapshot.loaded_count
                << ", loading=" << snapshot.loading_count
                << ", unloading=" << snapshot.unloading_count << '\n'
                << snapshot.loaded_chunk_map;
        Core::Log::info("World", message.str());
        const auto c = m_chunk_timing.snapshot();
        const auto t = m_terrain_timing.snapshot();
        if (!c.empty() && !t.empty()) {
            std::ostringstream prof;
            prof << "Streaming timings ms: chunk=" << c.front().last_ms << ", terrain=" << t.front().last_ms;
            Core::Log::info("World", prof.str());
        }
        m_last_debug_loaded_count = snapshot.loaded_count;
    }

    const auto terrain_snapshot = m_terrain_streamer.debug_snapshot();
    if (terrain_snapshot.loaded_mesh_count != m_last_terrain_mesh_count) {
        std::ostringstream message;
        message << "Terrain debug: meshes=" << terrain_snapshot.loaded_mesh_count
                << ", requested=" << terrain_snapshot.requested_mesh_count << '\n'
                << terrain_snapshot.biome_map;
        Core::Log::info("World", message.str());
        const auto c = m_chunk_timing.snapshot();
        const auto t = m_terrain_timing.snapshot();
        if (!c.empty() && !t.empty()) {
            std::ostringstream prof;
            prof << "Streaming timings ms: chunk=" << c.front().last_ms << ", terrain=" << t.front().last_ms;
            Core::Log::info("World", prof.str());
        }
        m_last_terrain_mesh_count = terrain_snapshot.loaded_mesh_count;
    }
}

void WorldModule::on_detach()
{
    m_terrain_streamer.shutdown();
    m_streamer.shutdown();
    Core::Log::info("World", "World module detached.");
}

const WorldStreamer& WorldModule::streamer() const
{
    return m_streamer;
}

const FloatingOrigin& WorldModule::floating_origin() const
{
    return m_floating_origin;
}

const Terrain::TerrainStreamer& WorldModule::terrain_streamer() const
{
    return m_terrain_streamer;
}

} // namespace Valley::World
