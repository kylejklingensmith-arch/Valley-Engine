#include "Valley/Terrain/AsyncTerrainMesher.h"
#include "Valley/Terrain/TerrainStreamer.h"
#include "Valley/World/WorldRegistry.h"

#include <cassert>
#include <iostream>

using namespace Valley;

namespace {
void test_layered_noise_is_deterministic()
{
    const Terrain::LayeredNoiseGenerator noise;
    const World::WorldVec3 position { .x = -128.25, .y = 0.0, .z = 512.75 };
    assert(noise.height_at(position) == noise.height_at(position));
    assert(noise.moisture_at(position) >= 0.0 && noise.moisture_at(position) <= 1.0);
    assert(noise.temperature_at(position) >= 0.0 && noise.temperature_at(position) <= 1.0);
}

void test_biome_classifier_rules()
{
    const Terrain::BiomeClassifier classifier;
    assert(classifier.classify(-20.0, 0.2, 0.8) == Terrain::BiomeId::Water);
    assert(classifier.classify(130.0, 0.2, 0.8) == Terrain::BiomeId::Highlands);
    assert(classifier.classify(20.0, 0.8, 0.7) == Terrain::BiomeId::Forest);
}

void test_lod_and_mesh_generation()
{
    const Terrain::TerrainGenerator generator;
    const auto lod0 = Terrain::terrain_lod_for_distance(10.0);
    const auto lod2 = Terrain::terrain_lod_for_distance(5000.0);
    assert(lod0.level == 0);
    assert(lod2.level == 2);
    assert(lod0.quads_per_axis > lod2.quads_per_axis);

    const auto chunk = generator.generate_chunk({ .x = -1, .y = 0, .z = 2 }, lod2);
    assert(chunk.samples.size() == (lod2.quads_per_axis + 1U) * (lod2.quads_per_axis + 1U));
    const auto mesh = generator.build_mesh(chunk);
    assert(mesh.vertices.size() == chunk.samples.size());
    assert(mesh.indices.size() == lod2.quads_per_axis * lod2.quads_per_axis * 6U);
}

void test_async_mesher()
{
    Terrain::AsyncTerrainMesher mesher;
    mesher.start();
    mesher.request_mesh({ .coord = { .x = 0, .y = 0, .z = 0 }, .lod = Terrain::terrain_lod_for_distance(0.0) });
    mesher.wait_until_idle();
    auto meshes = mesher.collect_completed_meshes();
    assert(meshes.size() == 1);
    assert(!meshes.front().vertices.empty());
    mesher.shutdown();
}

void test_terrain_streamer_follows_world_registry_and_debugs_biomes()
{
    World::WorldRegistry registry;
    registry.upsert({ .coord = { .x = 0, .y = 0, .z = 0 }, .state = World::ChunkState::Loaded, .revision = 1, .debug_name = "origin" });
    registry.upsert({ .coord = { .x = 1, .y = 0, .z = 0 }, .state = World::ChunkState::Loaded, .revision = 1, .debug_name = "east" });

    Terrain::TerrainStreamer streamer;
    streamer.start();
    streamer.sync_with_world(registry);
    streamer.wait_until_idle();
    streamer.service_completed_meshes();

    assert(streamer.loaded_meshes().size() == 2);
    const auto debug = streamer.debug_snapshot();
    assert(debug.loaded_mesh_count == 2);
    assert(debug.biome_map != "<no terrain meshes>");

    registry.erase({ .x = 1, .y = 0, .z = 0 });
    streamer.sync_with_world(registry);
    assert(streamer.loaded_meshes().size() == 1);
    streamer.shutdown();
}
} // namespace

int main()
{
    test_layered_noise_is_deterministic();
    test_biome_classifier_rules();
    test_lod_and_mesh_generation();
    test_async_mesher();
    test_terrain_streamer_follows_world_registry_and_debugs_biomes();
    std::cout << "Terrain framework tests passed.\n";
    return 0;
}
