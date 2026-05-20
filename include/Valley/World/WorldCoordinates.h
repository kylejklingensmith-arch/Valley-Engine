#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace Valley::World {

struct WorldVec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct ChunkCoord {
    std::int64_t x = 0;
    std::int64_t y = 0;
    std::int64_t z = 0;

    [[nodiscard]] friend bool operator==(const ChunkCoord& lhs, const ChunkCoord& rhs) = default;
};

struct WorldPosition {
    ChunkCoord chunk;
    WorldVec3 local;
};

struct WorldGridDesc {
    double chunk_size_meters = 256.0;
};

[[nodiscard]] std::int64_t floor_divide_to_i64(double value, double divisor);
[[nodiscard]] ChunkCoord chunk_from_world_position(const WorldVec3& position, const WorldGridDesc& grid = {});
[[nodiscard]] WorldPosition split_world_position(const WorldVec3& position, const WorldGridDesc& grid = {});
[[nodiscard]] WorldVec3 compose_world_position(const WorldPosition& position, const WorldGridDesc& grid = {});
[[nodiscard]] std::string to_string(const ChunkCoord& coord);

} // namespace Valley::World

namespace std {

template <>
struct hash<Valley::World::ChunkCoord> {
    [[nodiscard]] std::size_t operator()(const Valley::World::ChunkCoord& coord) const noexcept;
};

} // namespace std
