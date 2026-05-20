#pragma once

#include <cmath>

namespace Valley::Renderer {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs)
{
    return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs)
{
    return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

inline Vec3 operator*(const Vec3& value, double scalar)
{
    return { value.x * scalar, value.y * scalar, value.z * scalar };
}

inline double dot(const Vec3& lhs, const Vec3& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline double length(const Vec3& value)
{
    return std::sqrt(dot(value, value));
}

inline Vec3 normalized(const Vec3& value)
{
    const double value_length = length(value);
    if (value_length <= 0.000001) {
        return {};
    }

    return value * (1.0 / value_length);
}

} // namespace Valley::Renderer
