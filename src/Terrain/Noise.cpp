#include "Valley/Terrain/Noise.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace Valley::Terrain {
namespace {
[[nodiscard]] double fade(double t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

[[nodiscard]] double lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

[[nodiscard]] std::uint32_t hash_lattice(std::int64_t x, std::int64_t z, std::uint32_t seed)
{
    std::uint64_t value = static_cast<std::uint64_t>(x) * 0x9E3779B185EBCA87ULL;
    value ^= static_cast<std::uint64_t>(z) * 0xC2B2AE3D27D4EB4FULL;
    value ^= static_cast<std::uint64_t>(seed) * 0x165667B19E3779F9ULL;
    value ^= value >> 33U;
    value *= 0xff51afd7ed558ccdULL;
    value ^= value >> 33U;
    return static_cast<std::uint32_t>(value & 0xffffffffU);
}

[[nodiscard]] double lattice_value(std::int64_t x, std::int64_t z, std::uint32_t seed)
{
    return static_cast<double>(hash_lattice(x, z, seed)) / static_cast<double>(0xffffffffU);
}

[[nodiscard]] double smooth_value_noise(double x, double z, std::uint32_t seed)
{
    const auto x0 = static_cast<std::int64_t>(std::floor(x));
    const auto z0 = static_cast<std::int64_t>(std::floor(z));
    const double tx = fade(x - static_cast<double>(x0));
    const double tz = fade(z - static_cast<double>(z0));

    const double a = lattice_value(x0, z0, seed);
    const double b = lattice_value(x0 + 1, z0, seed);
    const double c = lattice_value(x0, z0 + 1, seed);
    const double d = lattice_value(x0 + 1, z0 + 1, seed);
    return lerp(lerp(a, b, tx), lerp(c, d, tx), tz);
}
} // namespace

LayeredNoiseGenerator::LayeredNoiseGenerator(LayeredNoiseDesc desc)
    : m_desc(std::move(desc))
{
}

double LayeredNoiseGenerator::height_at(const World::WorldVec3& position) const
{
    return layered_value(m_desc.height_layers, position.x, position.z);
}

double LayeredNoiseGenerator::moisture_at(const World::WorldVec3& position) const
{
    return std::clamp(layered_value(m_desc.moisture_layers, position.x, position.z), 0.0, 1.0);
}

double LayeredNoiseGenerator::temperature_at(const World::WorldVec3& position) const
{
    const double latitude_cooling = std::clamp(std::abs(position.z) / 12000.0, 0.0, 0.4);
    return std::clamp(layered_value(m_desc.temperature_layers, position.x, position.z) - latitude_cooling, 0.0, 1.0);
}

const LayeredNoiseDesc& LayeredNoiseGenerator::desc() const
{
    return m_desc;
}

LayeredNoiseDesc LayeredNoiseGenerator::default_desc()
{
    return {
        .height_layers = {
            { .amplitude = 80.0, .frequency = 0.0015, .seed = 17 },
            { .amplitude = 22.0, .frequency = 0.0060, .seed = 31 },
            { .amplitude = 6.0, .frequency = 0.0200, .seed = 53 },
        },
        .moisture_layers = {
            { .amplitude = 0.65, .frequency = 0.0025, .seed = 101 },
            { .amplitude = 0.35, .frequency = 0.0100, .seed = 103 },
        },
        .temperature_layers = {
            { .amplitude = 0.70, .frequency = 0.0018, .seed = 211 },
            { .amplitude = 0.30, .frequency = 0.0075, .seed = 223 },
        },
    };
}

double LayeredNoiseGenerator::layered_value(const std::vector<NoiseLayer>& layers, double x, double z)
{
    double value = 0.0;
    double amplitude_sum = 0.0;
    for (const auto& layer : layers) {
        value += smooth_value_noise(x * layer.frequency, z * layer.frequency, layer.seed) * layer.amplitude;
        amplitude_sum += std::abs(layer.amplitude);
    }

    if (amplitude_sum <= 0.0) {
        return 0.0;
    }

    const bool normalized = amplitude_sum <= 1.000001;
    return normalized ? value / amplitude_sum : value - amplitude_sum * 0.5;
}

} // namespace Valley::Terrain
