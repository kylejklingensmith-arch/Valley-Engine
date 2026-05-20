#include "Valley/Terrain/TerrainTypes.h"

#include <algorithm>

namespace Valley::Terrain {

std::size_t TerrainChunkData::samples_per_axis() const
{
    return static_cast<std::size_t>(lod.quads_per_axis) + 1U;
}

TerrainLod terrain_lod_for_distance(double distance_meters, double chunk_size_meters)
{
    const double safe_chunk_size = chunk_size_meters > 0.0 ? chunk_size_meters : 256.0;
    if (distance_meters < safe_chunk_size * 1.5) {
        return { .level = 0, .quads_per_axis = 32, .sample_spacing_meters = safe_chunk_size / 32.0 };
    }
    if (distance_meters < safe_chunk_size * 4.0) {
        return { .level = 1, .quads_per_axis = 16, .sample_spacing_meters = safe_chunk_size / 16.0 };
    }
    return { .level = 2, .quads_per_axis = 8, .sample_spacing_meters = safe_chunk_size / 8.0 };
}

std::string_view to_string(BiomeId biome)
{
    switch (biome) {
    case BiomeId::Water:
        return "Water";
    case BiomeId::Grassland:
        return "Grassland";
    case BiomeId::Forest:
        return "Forest";
    case BiomeId::Highlands:
        return "Highlands";
    case BiomeId::Snow:
        return "Snow";
    }
    return "Unknown";
}

char debug_biome_glyph(BiomeId biome)
{
    switch (biome) {
    case BiomeId::Water:
        return '~';
    case BiomeId::Grassland:
        return '.';
    case BiomeId::Forest:
        return 'F';
    case BiomeId::Highlands:
        return '^';
    case BiomeId::Snow:
        return '*';
    }
    return '?';
}

} // namespace Valley::Terrain
