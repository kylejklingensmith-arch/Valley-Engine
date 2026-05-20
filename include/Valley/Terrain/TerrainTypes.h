#pragma once

#include "Valley/World/WorldCoordinates.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Valley::Terrain {

enum class BiomeId : std::uint8_t {
    Water,
    Grassland,
    Forest,
    Highlands,
    Snow,
};

struct TerrainSample {
    double height_meters = 0.0;
    double moisture = 0.0;
    double temperature = 0.0;
    BiomeId biome = BiomeId::Grassland;
};

struct TerrainLod {
    unsigned int level = 0;
    unsigned int quads_per_axis = 32;
    double sample_spacing_meters = 8.0;
};

struct TerrainChunkData {
    World::ChunkCoord coord;
    TerrainLod lod;
    std::vector<TerrainSample> samples;

    [[nodiscard]] std::size_t samples_per_axis() const;
};

struct TerrainVertex {
    World::WorldVec3 position;
    BiomeId biome = BiomeId::Grassland;
};

struct TerrainMesh {
    World::ChunkCoord coord;
    TerrainLod lod;
    std::vector<TerrainVertex> vertices;
    std::vector<std::uint32_t> indices;
};

[[nodiscard]] TerrainLod terrain_lod_for_distance(double distance_meters, double chunk_size_meters = 256.0);
[[nodiscard]] std::string_view to_string(BiomeId biome);
[[nodiscard]] char debug_biome_glyph(BiomeId biome);

} // namespace Valley::Terrain
