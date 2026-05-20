#pragma once

#include "Valley/World/WorldCoordinates.h"

namespace Valley::World {

struct FloatingOriginDesc {
    double rebase_threshold_meters = 4096.0;
};

struct FloatingOriginRebase {
    bool rebased = false;
    WorldVec3 offset;
};

class FloatingOrigin {
public:
    explicit FloatingOrigin(FloatingOriginDesc desc = {});

    [[nodiscard]] FloatingOriginRebase update(const WorldVec3& observer_world_position);
    [[nodiscard]] const WorldVec3& origin_offset() const;
    [[nodiscard]] double rebase_threshold_meters() const;

private:
    FloatingOriginDesc m_desc;
    WorldVec3 m_origin_offset;
};

} // namespace Valley::World
