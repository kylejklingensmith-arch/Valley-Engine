#pragma once

#include "Valley/Core/TimeStep.h"
#include "Valley/Renderer/Math.h"

namespace Valley::Renderer {

class Camera {
public:
    void set_perspective(double vertical_fov_degrees, double near_clip, double far_clip);
    void look_at(const Vec3& position, const Vec3& target);
    void orbit_origin(double elapsed_seconds, double radius, double height);

    [[nodiscard]] const Vec3& position() const;
    [[nodiscard]] const Vec3& forward() const;
    [[nodiscard]] double vertical_fov_degrees() const;
    [[nodiscard]] double near_clip() const;
    [[nodiscard]] double far_clip() const;

private:
    Vec3 m_position { 0.0, 4.0, 6.0 };
    Vec3 m_forward { 0.0, -0.4, -1.0 };
    double m_vertical_fov_degrees = 60.0;
    double m_near_clip = 0.1;
    double m_far_clip = 1000.0;
};

} // namespace Valley::Renderer
