#pragma once

#include <cppgl.h>

// draw random float in [-1, 1]
inline float random_float() { return (float(rand() % 32768) / 16384.0f) - 1.0f; }
inline glm::vec2 random_vec2() { return glm::vec2(random_float(), random_float()); }
inline glm::vec3 random_vec3() { return glm::vec3(random_float(), random_float(), random_float()); }

void setup_geometry_shader(cppgl::Shader &shader);
void setup_light(const cppgl::Shader& shader);
void deferred_debug_pass(const cppgl::Framebuffer& gbuffer);
void deferred_shading_pass(const cppgl::Framebuffer& gbuffer);

void blit(const cppgl::Texture2D& tex);
void blit(const std::shared_ptr<cppgl::Texture2D>& tex);
