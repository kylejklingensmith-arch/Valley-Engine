#include "Valley/Renderer/Camera.h"

#include <cmath>
#include <stdexcept>

namespace Valley::Renderer {

void Camera::set_perspective(double vertical_fov_degrees, double near_clip, double far_clip)
{
    if (vertical_fov_degrees <= 0.0 || near_clip <= 0.0 || far_clip <= near_clip) {
        throw std::invalid_argument("Camera perspective values are invalid.");
    }

    m_vertical_fov_degrees = vertical_fov_degrees;
    m_near_clip = near_clip;
    m_far_clip = far_clip;
}

void Camera::look_at(const Vec3& position, const Vec3& target)
{
    m_position = position;
    m_forward = normalized(target - position);
}

void Camera::orbit_origin(double elapsed_seconds, double radius, double height)
{
    const double angle = elapsed_seconds * 0.35;
    const Vec3 position {
        std::sin(angle) * radius,
        height,
        std::cos(angle) * radius,
    };

    look_at(position, { 0.0, 0.6, 0.0 });
}

const Vec3& Camera::position() const
{
    return m_position;
}

const Vec3& Camera::forward() const
{
    return m_forward;
}

double Camera::vertical_fov_degrees() const
{
    return m_vertical_fov_degrees;
}

double Camera::near_clip() const
{
    return m_near_clip;
}

double Camera::far_clip() const
{
    return m_far_clip;
}

} // namespace Valley::Renderer
