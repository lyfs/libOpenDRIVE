#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

struct OrbitControls
{
    OrbitControls(glm::vec3 cam_position, glm::vec3 cam_target, glm::vec3 up);

    bool rotate(float left, float up);
    bool pan(float x, float y);
    bool zoom(float r);

    glm::vec3 cam_position = {0, 0, 0};
    glm::vec3 cam_target = {0, 0, 0};

    const float min_polar_angle = 0.0;
    const float max_polar_angle = M_PI;
    const float phi_eps = 0.000001;

    const glm::vec3 world_up;
    const glm::quat quat_y_up;
};