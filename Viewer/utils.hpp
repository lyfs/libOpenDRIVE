#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <array>

inline glm::quat quaternion_from_unit_vectors(glm::vec3 v_from, glm::vec3 v_to)
{
    glm::quat out_quat;
    float radius = glm::dot(v_from, v_to) + 1.0f;
    if (radius < 2.7e-16)
    {
        radius = 0;
        if (std::abs(v_from.x) > std::abs(v_from.z))
        {
            out_quat = glm::quat(radius, -v_from.y, v_from.x, 0);
        }
        else
        {
            out_quat = glm::quat(radius, 0, -v_from.z, v_from.y);
        }
    }
    else
    {
        out_quat = glm::quat(radius, v_from.y * v_to.z - v_from.z * v_to.y, v_from.z * v_to.x - v_from.x * v_to.z, v_from.x * v_to.y - v_from.y * v_to.x);
    }

    return glm::normalize(out_quat);
}

inline glm::vec3 from_cartesian_coords_to_spherical(float x, float y, float z)
{

    const float radius = std::sqrt(x * x + y * y + z * z);
    if (radius == 0)
    {
        return glm::vec3(0, 0, 0);
    }

    const float theta = std::atan2(x, z);
    const float phi = std::acos(std::min<float>(std::max<float>(y / radius, -1), 1));

    return glm::vec3(radius, theta, phi);
}

inline glm::vec3 from_spherical_coords_to_cartesian(float radius, float theta, float phi)
{
    const float sin_phi_radius = std::sin(phi) * radius;
    return glm::vec3(sin_phi_radius * std::sin(theta), std::cos(phi) * radius, sin_phi_radius * std::cos(theta));
}

inline glm::vec3 apply_quaternion(glm::quat q, glm::vec3 xyz)
{
    const float ix = q.w * xyz.x + q.y * xyz.z - q.z * xyz.y;
    const float iy = q.w * xyz.y + q.z * xyz.x - q.x * xyz.z;
    const float iz = q.w * xyz.z + q.x * xyz.y - q.y * xyz.x;
    const float iw = -q.x * xyz.x - q.y * xyz.y - q.z * xyz.z;

    return glm::vec3(ix * q.w + iw * -q.x + iy * -q.z - iz * -q.y, iy * q.w + iw * -q.y + iz * -q.x - ix * -q.z, iz * q.w + iw * -q.z + ix * -q.y - iy * -q.x);
}