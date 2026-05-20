#include "Valley/World/WorldCoordinates.h"

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace Valley::World {
namespace {
void validate_grid(const WorldGridDesc& grid)
{
    if (!std::isfinite(grid.chunk_size_meters) || grid.chunk_size_meters <= 0.0) {
        throw std::invalid_argument("World chunk size must be finite and positive.");
    }
}
} // namespace

std::int64_t floor_divide_to_i64(double value, double divisor)
{
    if (!std::isfinite(value) || !std::isfinite(divisor) || divisor <= 0.0) {
        throw std::invalid_argument("World coordinate division values are invalid.");
    }

    return static_cast<std::int64_t>(std::floor(value / divisor));
}

ChunkCoord chunk_from_world_position(const WorldVec3& position, const WorldGridDesc& grid)
{
    validate_grid(grid);
    return {
        .x = floor_divide_to_i64(position.x, grid.chunk_size_meters),
        .y = floor_divide_to_i64(position.y, grid.chunk_size_meters),
        .z = floor_divide_to_i64(position.z, grid.chunk_size_meters),
    };
}

WorldPosition split_world_position(const WorldVec3& position, const WorldGridDesc& grid)
{
    const ChunkCoord chunk = chunk_from_world_position(position, grid);
    return {
        .chunk = chunk,
        .local = {
            .x = position.x - static_cast<double>(chunk.x) * grid.chunk_size_meters,
            .y = position.y - static_cast<double>(chunk.y) * grid.chunk_size_meters,
            .z = position.z - static_cast<double>(chunk.z) * grid.chunk_size_meters,
        },
    };
}

WorldVec3 compose_world_position(const WorldPosition& position, const WorldGridDesc& grid)
{
    validate_grid(grid);
    return {
        .x = static_cast<double>(position.chunk.x) * grid.chunk_size_meters + position.local.x,
        .y = static_cast<double>(position.chunk.y) * grid.chunk_size_meters + position.local.y,
        .z = static_cast<double>(position.chunk.z) * grid.chunk_size_meters + position.local.z,
    };
}

std::string to_string(const ChunkCoord& coord)
{
    std::ostringstream stream;
    stream << '(' << coord.x << ',' << coord.y << ',' << coord.z << ')';
    return stream.str();
}

} // namespace Valley::World

namespace std {

std::size_t hash<Valley::World::ChunkCoord>::operator()(const Valley::World::ChunkCoord& coord) const noexcept
{
    const auto mix = [](std::uint64_t value) {
        value ^= value >> 30U;
        value *= 0xbf58476d1ce4e5b9ULL;
        value ^= value >> 27U;
        value *= 0x94d049bb133111ebULL;
        value ^= value >> 31U;
        return value;
    };

    const auto x = mix(static_cast<std::uint64_t>(coord.x));
    const auto y = mix(static_cast<std::uint64_t>(coord.y));
    const auto z = mix(static_cast<std::uint64_t>(coord.z));
    return static_cast<std::size_t>(x ^ (y << 1U) ^ (z << 2U));
}

} // namespace std
