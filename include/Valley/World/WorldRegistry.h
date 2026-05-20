#pragma once

#include "Valley/World/WorldChunk.h"

#include <cstddef>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Valley::World {

class WorldRegistry {
public:
    ChunkRecord& upsert(const ChunkRecord& record);
    bool erase(const ChunkCoord& coord);
    bool contains(const ChunkCoord& coord) const;
    void set_state(const ChunkCoord& coord, ChunkState state);
    void clear();

    [[nodiscard]] std::optional<ChunkRecord> find(const ChunkCoord& coord) const;
    [[nodiscard]] std::vector<ChunkRecord> records() const;
    [[nodiscard]] std::vector<ChunkRecord> records_with_state(ChunkState state) const;
    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] std::size_t count(ChunkState state) const;

private:
    std::unordered_map<ChunkCoord, ChunkRecord> m_chunks;
};

} // namespace Valley::World
