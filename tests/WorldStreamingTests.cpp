#include "Valley/World/FloatingOrigin.h"
#include "Valley/World/WorldCoordinates.h"
#include "Valley/World/WorldDebug.h"
#include "Valley/World/WorldStreamer.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

void require_near(double actual, double expected, std::string_view message)
{
    constexpr double epsilon = 0.000000001;
    if (std::fabs(actual - expected) > epsilon) {
        throw std::runtime_error(std::string(message));
    }
}

void deterministic_coordinates_handle_negative_world_space()
{
    const Valley::World::WorldGridDesc grid { .chunk_size_meters = 256.0 };

    const auto positive = Valley::World::split_world_position({ .x = 511.0, .y = 0.0, .z = 256.0 }, grid);
    require(positive.chunk.x == 1 && positive.chunk.z == 1, "positive coordinates map to expected chunks");
    require_near(positive.local.x, 255.0, "positive local x is stable");

    const auto negative = Valley::World::split_world_position({ .x = -0.25, .y = 0.0, .z = -256.25 }, grid);
    require(negative.chunk.x == -1 && negative.chunk.z == -2, "negative coordinates use floor division");
    require_near(negative.local.x, 255.75, "negative local x remains positive within chunk");
    require_near(negative.local.z, 255.75, "negative local z remains positive within chunk");

    const auto recomposed = Valley::World::compose_world_position(negative, grid);
    require_near(recomposed.x, -0.25, "split and compose roundtrip x");
    require_near(recomposed.z, -256.25, "split and compose roundtrip z");
}

void streaming_loads_and_unloads_chunk_sets_asynchronously()
{
    Valley::World::WorldStreamer streamer({ .grid = { .chunk_size_meters = 256.0 }, .load_radius_chunks = 1 });
    streamer.start();

    streamer.update_observer({ .x = 0.0, .y = 0.0, .z = 0.0 });
    streamer.wait_until_idle();
    streamer.service_completed_requests();
    require(streamer.registry().count(Valley::World::ChunkState::Loaded) == 9, "radius-one observer loads a 3x3 chunk set");

    const auto snapshot = Valley::World::create_debug_snapshot(streamer.registry());
    require(snapshot.loaded_count == 9, "debug snapshot reports loaded chunks");
    require(snapshot.loaded_chunk_map.find('#') != std::string::npos, "debug map visualizes loaded chunks");

    streamer.update_observer({ .x = 1024.0, .y = 0.0, .z = 0.0 });
    streamer.wait_until_idle();
    streamer.service_completed_requests();
    streamer.wait_until_idle();
    streamer.service_completed_requests();
    require(streamer.registry().count(Valley::World::ChunkState::Loaded) == 9, "moving observer keeps bounded loaded set");
    require(streamer.registry().contains({ .x = 4, .y = 0, .z = 0 }), "new observer center chunk is loaded");
    require(!streamer.registry().contains({ .x = 0, .y = 0, .z = 0 }), "old distant chunk is unloaded");

    streamer.shutdown();
}

void floating_origin_rebases_only_after_threshold()
{
    Valley::World::FloatingOrigin origin({ .rebase_threshold_meters = 1000.0 });

    const auto first = origin.update({ .x = 100.0, .y = 0.0, .z = 0.0 });
    require(!first.rebased, "origin does not rebase inside threshold");

    const auto second = origin.update({ .x = 1200.0, .y = 0.0, .z = 0.0 });
    require(second.rebased, "origin rebases outside threshold");
    require_near(second.offset.x, 1200.0, "rebase offset describes previous relative distance");
    require_near(origin.origin_offset().x, 1200.0, "origin offset is updated");
}

} // namespace

int main()
{
    try {
        deterministic_coordinates_handle_negative_world_space();
        streaming_loads_and_unloads_chunk_sets_asynchronously();
        floating_origin_rebases_only_after_threshold();
    } catch (const std::exception& exception) {
        std::cerr << "WorldStreamingTests failed: " << exception.what() << '\n';
        return 1;
    }

    std::cout << "WorldStreamingTests passed.\n";
    return 0;
}
