#include "Valley/World/FloatingOrigin.h"

#include <cmath>
#include <stdexcept>

namespace Valley::World {
namespace {
double distance_squared(const WorldVec3& value)
{
    return value.x * value.x + value.y * value.y + value.z * value.z;
}
} // namespace

FloatingOrigin::FloatingOrigin(FloatingOriginDesc desc)
    : m_desc(desc)
{
    if (!std::isfinite(m_desc.rebase_threshold_meters) || m_desc.rebase_threshold_meters <= 0.0) {
        throw std::invalid_argument("Floating origin threshold must be finite and positive.");
    }
}

FloatingOriginRebase FloatingOrigin::update(const WorldVec3& observer_world_position)
{
    const WorldVec3 relative {
        .x = observer_world_position.x - m_origin_offset.x,
        .y = observer_world_position.y - m_origin_offset.y,
        .z = observer_world_position.z - m_origin_offset.z,
    };

    const double threshold_squared = m_desc.rebase_threshold_meters * m_desc.rebase_threshold_meters;
    if (distance_squared(relative) < threshold_squared) {
        return {};
    }

    m_origin_offset = observer_world_position;
    return {
        .rebased = true,
        .offset = relative,
    };
}

const WorldVec3& FloatingOrigin::origin_offset() const
{
    return m_origin_offset;
}

double FloatingOrigin::rebase_threshold_meters() const
{
    return m_desc.rebase_threshold_meters;
}

} // namespace Valley::World
