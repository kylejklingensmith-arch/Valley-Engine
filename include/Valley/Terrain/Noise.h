#pragma once

#include "Valley/World/WorldCoordinates.h"

#include <cstdint>
#include <vector>

namespace Valley::Terrain {

struct NoiseLayer {
    double amplitude = 1.0;
    double frequency = 0.01;
    std::uint32_t seed = 1;
};

struct LayeredNoiseDesc {
    std::vector<NoiseLayer> height_layers;
    std::vector<NoiseLayer> moisture_layers;
    std::vector<NoiseLayer> temperature_layers;
};

class LayeredNoiseGenerator {
public:
    explicit LayeredNoiseGenerator(LayeredNoiseDesc desc = default_desc());

    [[nodiscard]] double height_at(const World::WorldVec3& position) const;
    [[nodiscard]] double moisture_at(const World::WorldVec3& position) const;
    [[nodiscard]] double temperature_at(const World::WorldVec3& position) const;
    [[nodiscard]] const LayeredNoiseDesc& desc() const;

    [[nodiscard]] static LayeredNoiseDesc default_desc();

private:
    [[nodiscard]] static double layered_value(const std::vector<NoiseLayer>& layers, double x, double z);

    LayeredNoiseDesc m_desc;
};

} // namespace Valley::Terrain
