#include "OrbitControls.h"

#include "utils.hpp"

OrbitControls::OrbitControls(glm::vec3 cam_position, glm::vec3 cam_target, glm::vec3 up)
    : cam_position(cam_position), cam_target(cam_target), world_up(up), quat_y_up(quaternion_from_unit_vectors(world_up, glm::vec3(0, 1, 0))){};

bool OrbitControls::rotate(float left, float up)
{
    const glm::vec3 offset_xyz = apply_quaternion(quat_y_up, this->cam_position - this->cam_target);

    glm::vec3 spherical = from_cartesian_coords_to_spherical(offset_xyz.x, offset_xyz.y, offset_xyz.z);
    spherical.y -= left;
    spherical.z -= up;
    spherical.z = std::max<float>(this->min_polar_angle, std::min<float>(this->max_polar_angle, spherical.z));
    spherical.z = std::max<float>(this->phi_eps, std::min<float>(M_PI - this->phi_eps, spherical.z));

    const glm::vec3 offset_xyz_new = apply_quaternion(glm::inverse(quat_y_up), from_spherical_coords_to_cartesian(spherical.x, spherical.y, spherical.z));
    this->cam_position = this->cam_target + offset_xyz_new;

    return true;
};

bool OrbitControls::pan(float x, float y)
{
    glm::vec3 f(glm::normalize(this->cam_target - this->cam_position));
    glm::vec3 s(glm::normalize(glm::cross(this->world_up, f)));
    glm::vec3 pan_offset = (s * x) + glm::cross(this->world_up, s) * -y;

    const glm::vec3 offset_xyz = this->cam_position - this->cam_target;
    this->cam_target += pan_offset;
    this->cam_position = this->cam_target + offset_xyz;

    return true;
};

bool OrbitControls::zoom(float zoom)
{
    const glm::vec3 offset_xyz = this->cam_position - this->cam_target;
    glm::vec3 spherical = from_cartesian_coords_to_spherical(offset_xyz.x, offset_xyz.y, offset_xyz.z);
    spherical.x -= zoom;

    const glm::vec3 offset_xyz_new = from_spherical_coords_to_cartesian(spherical.x, spherical.y, spherical.z);
    this->cam_position = this->cam_target + offset_xyz_new;

    return true;
};