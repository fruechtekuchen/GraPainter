#include "camera.h"
#include "context.h"
#include "imgui/imgui.h"
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


CPPGL_NAMESPACE_BEGIN

static Camera current_cam;
static Camera main_cam;

Camera current_camera() {
    static Camera default_cam("default");
    return current_cam ? current_cam : default_cam;
}

void make_camera_current(const Camera& cam) {
    current_cam = cam;
}

Camera main_camera() {
    return main_cam ? main_cam : current_camera();
}

void make_camera_main(const Camera& cam) {
    main_cam = cam;
}

static glm::mat4 get_projection_matrix(float left, float right, float top, float bottom, float n, float f) {
    glm::mat4 proj = glm::mat4(0);
    proj[0][0] = (2.f*n) / (right - left);
    proj[1][1] = (2.f*n) / (top- bottom);
    proj[2][0] = (right + left) / (right - (left));
    proj[2][1] = (bottom+top) / (top-bottom);
    proj[2][2] = -(f + n) / (f - n);
    proj[2][3] = -1.f;
    proj[3][2] = (-2 * f * n) / (f - n);
    return proj;
}

// ----------------------------------------------------
// CameraImpl

float CameraImpl::default_camera_movement_speed = 0.025f;

CameraImpl::CameraImpl(const std::string& name) : name(name), pos(0, 0, 0), dir(1, 0, 0), up(0, 1, 0), fov_degree(70),
    near(0.01f), far(1000), left(-100), right(100), bottom(-100), top(100),
    perspective(true), skewed(false), fix_up_vector(true) {
    update();
}

CameraImpl::~CameraImpl() {}

void CameraImpl::update() {
    update_view();
    update_proj();
    update_frustum();
}

void CameraImpl::update_view() {
    dir = glm::normalize(dir);
    up = glm::normalize(up);
    view = glm::lookAt(pos, pos + dir, up);
    view_normal = glm::transpose(glm::inverse(view));
}

void CameraImpl::update_proj() {
    proj = perspective ? (skewed ? get_projection_matrix(left, right, top, bottom, near, far)
                                 : glm::perspective(glm::radians(fov_degree), aspect_ratio(), near, far))
                        : glm::ortho(left, right, bottom, top, near, far);
}

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
void CameraImpl::update_frustum() {
    // Only implemented for non-skewed perspective projections.
    if (skewed || !perspective) { return; }

    const float half_v_side = far * tanf(fov_degree * M_PI / 360);
    const float half_h_side = aspect_ratio() * half_v_side;
    const auto far_vec = far * dir;
    // overwriting right and up vector for this scope
    const auto right = glm::normalize(glm::cross(dir, up));
    const auto up = glm::normalize(glm::cross(right, dir));

    frustum.nearFace   = { pos + near * dir, dir };
    frustum.farFace    = { pos + far_vec, -dir };
    frustum.leftFace   = { pos, glm::normalize(glm::cross(up, far_vec + right * half_h_side)) };
    frustum.rightFace  = { pos, glm::normalize(glm::cross(far_vec - right * half_h_side, up)) };
    frustum.topFace    = { pos, glm::normalize(glm::cross(right, far_vec - up * half_v_side)) };
    frustum.bottomFace = { pos, glm::normalize(glm::cross(far_vec + up * half_v_side, right)) };
}

bool CameraImpl::is_sphere_on_frustum(const glm::vec3 &center, const float radius) const {
    static const auto is_on_or_forward_plane = [&center, radius](const Plane &plane) {
        return plane.signed_dist_to_point(center) > -radius;
    };
    return skewed || !perspective ||
       (is_on_or_forward_plane(frustum.nearFace) &&
        is_on_or_forward_plane(frustum.farFace) &&
        is_on_or_forward_plane(frustum.leftFace) &&
        is_on_or_forward_plane(frustum.rightFace) &&
        is_on_or_forward_plane(frustum.topFace) &&
        is_on_or_forward_plane(frustum.bottomFace));
}

std::array<glm::vec3, 8> CameraImpl::frustum_corners() const {
    // https://learnopengl.com/Guest-Articles/2021/CSM

    const auto inv = glm::inverse(proj * view);
    std::array<glm::vec3, 8> corners;
    for (size_t x = 0; x < 2; ++x) {
        for (size_t y = 0; y < 2; ++y) {
            for (size_t z = 0; z < 2; ++z) {
                const glm::vec4 pt = 
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                corners[z*4+y*2+x] = pt / pt.w;
            }
        }
    }
    return corners;
}

void CameraImpl::forward(float by) { pos += by * dir; }
void CameraImpl::backward(float by) { pos -= by * dir; }
void CameraImpl::leftward(float by) { pos -= by * glm::normalize(glm::cross(dir, up)); }
void CameraImpl::rightward(float by) { pos += by * glm::normalize(glm::cross(dir, up)); }
void CameraImpl::upward(float by) { pos += by * glm::normalize(glm::cross(glm::cross(dir, up), dir)); }
void CameraImpl::downward(float by) { pos -= by * glm::normalize(glm::cross(glm::cross(dir, up), dir)); }

void CameraImpl::yaw(float angle) { dir = glm::normalize(glm::rotate(dir, angle * float(M_PI) / 180.f, up)); }
void CameraImpl::pitch(float angle) {
    dir = glm::normalize(glm::rotate(dir, angle * float(M_PI) / 180.f, glm::normalize(glm::cross(dir, up))));
    if (!fix_up_vector) up = glm::normalize(glm::cross(glm::cross(dir, up), dir));
}
void CameraImpl::roll(float angle) { up = glm::normalize(glm::rotate(up, angle * float(M_PI) / 180.f, dir)); }

void CameraImpl::from_lookat(const glm::vec3& pos, const glm::vec3& lookat, const glm::vec3& up) {
    this->pos = pos;
    this->dir = glm::normalize(lookat - pos);
    this->up = up;
    update();
}

void CameraImpl::load(const glm::vec3& pos, const glm::quat& rot) {
    this->pos = pos;
    this->view = glm::mat4_cast(rot);
    this->dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    this->up = glm::vec3(view[0][1], view[1][1], view[2][1]);
    this->update();
}


float CameraImpl::aspect_ratio() {
    const glm::ivec2 res = Context::resolution();
    return float(res.x) / float(res.y);
}

bool CameraImpl::default_input_handler(double dt_ms) {
    bool moved = false;
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        // keyboard
        if (Context::key_pressed(GLFW_KEY_W)) {
            current_camera()->forward(float(dt_ms * default_camera_movement_speed));
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_S)) {
            current_camera()->backward(float(dt_ms * default_camera_movement_speed));
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_A)) {
            current_camera()->leftward(float(dt_ms * default_camera_movement_speed));
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_D)) {
            current_camera()->rightward(float(dt_ms * default_camera_movement_speed));
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_R)) {
            current_camera()->upward(float(dt_ms * default_camera_movement_speed));
            moved = true;
        }
        if (Context::key_pressed(GLFW_KEY_F)) {
            current_camera()->downward(float(dt_ms * default_camera_movement_speed));
            moved = true;
        }
        // if (Context::key_pressed(GLFW_KEY_Q)) {
        //     current_camera()->roll(float(dt_ms * -0.1));
        //     moved = true;
        // }
        // if (Context::key_pressed(GLFW_KEY_E)) {
        //     current_camera()->roll(float(dt_ms * 0.1));
        //     moved = true;
        // }
    }
    // mouse
    static float rot_speed = 0.05f;
    static glm::vec2 last_pos(-1);
    const glm::vec2 curr_pos = Context::mouse_pos();
    if (last_pos == glm::vec2(-1)) last_pos = curr_pos;
    const glm::vec2 diff = last_pos - curr_pos;
    if (!ImGui::GetIO().WantCaptureMouse && Context::mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (glm::length(diff) > 0.01) {
            current_camera()->pitch(diff.y * rot_speed);
            current_camera()->yaw(diff.x * rot_speed);
            moved = true;
        }
    }
    last_pos = curr_pos;
    return moved;
}

CPPGL_NAMESPACE_END
