#include "Valley/World/WorldRegistry.h"

namespace Valley::World {

ChunkRecord& WorldRegistry::upsert(const ChunkRecord& record)
{
    auto [iterator, inserted] = m_chunks.insert_or_assign(record.coord, record);
    (void)inserted;
    return iterator->second;
}

bool WorldRegistry::erase(const ChunkCoord& coord)
{
    return m_chunks.erase(coord) > 0;
}

bool WorldRegistry::contains(const ChunkCoord& coord) const
{
    return m_chunks.contains(coord);
}

void WorldRegistry::set_state(const ChunkCoord& coord, ChunkState state)
{
    if (auto iterator = m_chunks.find(coord); iterator != m_chunks.end()) {
        iterator->second.state = state;
    }
}

void WorldRegistry::clear()
{
    m_chunks.clear();
}

std::optional<ChunkRecord> WorldRegistry::find(const ChunkCoord& coord) const
{
    if (auto iterator = m_chunks.find(coord); iterator != m_chunks.end()) {
        return iterator->second;
    }

    return std::nullopt;
}

std::vector<ChunkRecord> WorldRegistry::records() const
{
    std::vector<ChunkRecord> result;
    result.reserve(m_chunks.size());
    for (const auto& [coord, record] : m_chunks) {
        (void)coord;
        result.push_back(record);
    }
    return result;
}

std::vector<ChunkRecord> WorldRegistry::records_with_state(ChunkState state) const
{
    std::vector<ChunkRecord> result;
    for (const auto& [coord, record] : m_chunks) {
        (void)coord;
        if (record.state == state) {
            result.push_back(record);
        }
    }
    return result;
}

std::size_t WorldRegistry::size() const
{
    return m_chunks.size();
}

std::size_t WorldRegistry::count(ChunkState state) const
{
    std::size_t result = 0;
    for (const auto& [coord, record] : m_chunks) {
        (void)coord;
        if (record.state == state) {
            ++result;
        }
    }
    return result;
}

} // namespace Valley::World
