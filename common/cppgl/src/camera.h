#pragma once

#include <string>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include "named_handle.h"

#undef far
#undef near

CPPGL_NAMESPACE_BEGIN

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Plane {
    // unit vector
    glm::vec3 normal = { 0.f, 1.f, 0.f };

    // distance from origin to the nearest point in the plane
    float distance = 0.f;

    Plane() {};
    Plane(const glm::vec3 &pos, const glm::vec3 &normal)
    : normal(normal), distance(glm::dot(pos, normal)) {};

    float signed_dist_to_point(const glm::vec3 &point) const {
        return glm::dot(normal, point) - distance;
    }
};
struct Frustum {
    Plane topFace;
    Plane bottomFace;
    Plane rightFace;
    Plane leftFace;
    Plane farFace;
    Plane nearFace;
};

// ----------------------------------------------------
// Camera

class CameraImpl {
public:
    CameraImpl(const std::string& name);
    virtual ~CameraImpl();

    void update();
    void update_view();
    void update_proj();
    void update_frustum();

    bool is_sphere_on_frustum(const glm::vec3 &center, const float radius) const;

    // { near, far }
    std::array<glm::vec3, 8> frustum_corners() const;

    // move
    void forward(float by);
    void backward(float by);
    void leftward(float by);
    void rightward(float by);
    void upward(float by);
    void downward(float by);

    // rotate
    void yaw(float angle);
    void pitch(float angle);
    void roll(float angle);

    // load/store
    void store(glm::vec3& pos, glm::quat& rot) const; // TODO
    void load(const glm::vec3& pos, const glm::quat& rot); // TODO
    void from_lookat(const glm::vec3& pos, const glm::vec3& lookat, const glm::vec3& up = glm::vec3(0, 1, 0));

    // compute aspect ratio from current viewport
    static float aspect_ratio();

    // default camera keyboard/mouse handler for basic movement
    static float default_camera_movement_speed;
    static bool default_input_handler(double dt_ms);

    // data
    const std::string name;
    glm::vec3 pos, dir, up;             // camera coordinate system
    float fov_degree;
    float near, far;        // perspective projection
    float left, right, bottom, top;     // orthographic projection
    bool perspective;                   // switch between perspective and orthographic (default: perspective)
    bool skewed;                        // switcg between normal perspective and skewed frustum (default: normal)
    bool fix_up_vector;                 // keep up vector fixed to avoid camera drift
    glm::mat4 view, view_normal, proj;  // camera matrices (computed via a call update())
    Frustum frustum;
};

using Camera = NamedHandle<CameraImpl>;

// Camera explicitly belonging to the currently selected camera
Camera main_camera();
// Camera that might also be used for render passes
Camera current_camera();
void make_camera_current(const Camera& cam);
void make_camera_main(const Camera& cam);

CPPGL_NAMESPACE_END
