#pragma once

#include "Valley/Terrain/Biome.h"
#include "Valley/Terrain/Noise.h"
#include "Valley/Terrain/TerrainTypes.h"

namespace Valley::Terrain {

struct TerrainGeneratorDesc {
    World::WorldGridDesc grid;
    LayeredNoiseDesc noise = LayeredNoiseGenerator::default_desc();
};

class TerrainGenerator {
public:
    explicit TerrainGenerator(TerrainGeneratorDesc desc = {});

    [[nodiscard]] TerrainChunkData generate_chunk(const World::ChunkCoord& coord, TerrainLod lod) const;
    [[nodiscard]] TerrainMesh build_mesh(const TerrainChunkData& chunk) const;
    [[nodiscard]] const TerrainGeneratorDesc& desc() const;

private:
    TerrainGeneratorDesc m_desc;
    LayeredNoiseGenerator m_noise;
    BiomeClassifier m_biomes;
};

} // namespace Valley::Terrain
