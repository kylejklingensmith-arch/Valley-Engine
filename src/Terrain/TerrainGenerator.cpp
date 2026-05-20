#include "Valley/Terrain/TerrainGenerator.h"

#include <stdexcept>
#include <utility>

namespace Valley::Terrain {

TerrainGenerator::TerrainGenerator(TerrainGeneratorDesc desc)
    : m_desc(std::move(desc)), m_noise(m_desc.noise)
{
    if (m_desc.grid.chunk_size_meters <= 0.0) {
        throw std::invalid_argument("Terrain chunk size must be positive.");
    }
}

TerrainChunkData TerrainGenerator::generate_chunk(const World::ChunkCoord& coord, TerrainLod lod) const
{
    if (lod.quads_per_axis == 0) {
        throw std::invalid_argument("Terrain LOD must contain at least one quad.");
    }

    TerrainChunkData chunk { .coord = coord, .lod = lod, .samples = {} };
    const std::size_t axis = chunk.samples_per_axis();
    chunk.samples.reserve(axis * axis);

    const double origin_x = static_cast<double>(coord.x) * m_desc.grid.chunk_size_meters;
    const double origin_z = static_cast<double>(coord.z) * m_desc.grid.chunk_size_meters;
    const double spacing = m_desc.grid.chunk_size_meters / static_cast<double>(lod.quads_per_axis);

    for (std::size_t z = 0; z < axis; ++z) {
        for (std::size_t x = 0; x < axis; ++x) {
            const World::WorldVec3 position {
                .x = origin_x + static_cast<double>(x) * spacing,
                .y = 0.0,
                .z = origin_z + static_cast<double>(z) * spacing,
            };
            const double height = m_noise.height_at(position);
            const double moisture = m_noise.moisture_at(position);
            const double temperature = m_noise.temperature_at(position);
            chunk.samples.push_back({
                .height_meters = height,
                .moisture = moisture,
                .temperature = temperature,
                .biome = m_biomes.classify(height, moisture, temperature),
            });
        }
    }

    return chunk;
}

TerrainMesh TerrainGenerator::build_mesh(const TerrainChunkData& chunk) const
{
    TerrainMesh mesh { .coord = chunk.coord, .lod = chunk.lod, .vertices = {}, .indices = {} };
    const std::size_t axis = chunk.samples_per_axis();
    if (chunk.samples.size() != axis * axis) {
        throw std::invalid_argument("Terrain sample count does not match its LOD resolution.");
    }

    const double origin_x = static_cast<double>(chunk.coord.x) * m_desc.grid.chunk_size_meters;
    const double origin_z = static_cast<double>(chunk.coord.z) * m_desc.grid.chunk_size_meters;
    const double spacing = m_desc.grid.chunk_size_meters / static_cast<double>(chunk.lod.quads_per_axis);

    mesh.vertices.reserve(chunk.samples.size());
    for (std::size_t z = 0; z < axis; ++z) {
        for (std::size_t x = 0; x < axis; ++x) {
            const auto& sample = chunk.samples[z * axis + x];
            mesh.vertices.push_back({
                .position = {
                    .x = origin_x + static_cast<double>(x) * spacing,
                    .y = sample.height_meters,
                    .z = origin_z + static_cast<double>(z) * spacing,
                },
                .biome = sample.biome,
            });
        }
    }

    mesh.indices.reserve(static_cast<std::size_t>(chunk.lod.quads_per_axis) * chunk.lod.quads_per_axis * 6U);
    for (std::uint32_t z = 0; z < chunk.lod.quads_per_axis; ++z) {
        for (std::uint32_t x = 0; x < chunk.lod.quads_per_axis; ++x) {
            const std::uint32_t top_left = z * static_cast<std::uint32_t>(axis) + x;
            const std::uint32_t top_right = top_left + 1U;
            const std::uint32_t bottom_left = top_left + static_cast<std::uint32_t>(axis);
            const std::uint32_t bottom_right = bottom_left + 1U;
            mesh.indices.insert(mesh.indices.end(), { top_left, bottom_left, top_right, top_right, bottom_left, bottom_right });
        }
    }

    return mesh;
}

const TerrainGeneratorDesc& TerrainGenerator::desc() const
{
    return m_desc;
}

} // namespace Valley::Terrain
