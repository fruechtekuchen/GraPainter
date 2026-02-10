#include "rendering.h"
#include <cppgl.h>

using namespace std;
using namespace cppgl;

void setup_geometry_shader(cppgl::Shader &shader) {
    auto cam = current_camera();
    shader->uniform("cam.near", cam->near);
    shader->uniform("cam.far",  cam->far);
    shader->uniform("cam.pos",  cam->pos);
    shader->uniform("cam.dir",  cam->dir);
    shader->uniform("view", cam->view);
    shader->uniform("view_normal", cam->view_normal);
    shader->uniform("proj", cam->proj);
}

void setup_light(const Shader& shader) {
	shader->uniform("ambient_col", glm::vec3(0.12f, 0.14f, 0.16f));
	shader->uniform("light_dir", glm::normalize(glm::vec3(1.f, -0.6f, -0.4f)));
	shader->uniform("light_col", glm::vec3(0.6f, 0.7f, 0.8f));
}

void deferred_debug_pass(const Framebuffer& gbuffer) {
    static auto shader = Shader("deferred-debug", "shader/copytex.vs", "shader/deferred-debug.fs");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->bind();
    setup_light(shader);
    shader->uniform("gbuf_depth", gbuffer->depth_texture, 0);
    shader->uniform("gbuf_diff", gbuffer->color_textures[0], 1);
    shader->uniform("gbuf_pos", gbuffer->color_textures[1], 2);
    shader->uniform("gbuf_norm", gbuffer->color_textures[2], 3);
    shader->uniform("near_far", glm::vec2(current_camera()->near, current_camera()->far));
	glDisable(GL_DEPTH_TEST);
    Quad::draw();
	glEnable(GL_DEPTH_TEST);
    shader->unbind();
}

void deferred_shading_pass(const Framebuffer& gbuffer) {
    static auto shader = Shader("deferred-lighting", "shader/copytex.vs", "shader/deferred-lighting.fs");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->bind();
    setup_light(shader);
    shader->uniform("gbuf_depth", gbuffer->depth_texture, 0);
    shader->uniform("gbuf_diff", gbuffer->color_textures[0], 1);
    shader->uniform("gbuf_pos", gbuffer->color_textures[1], 2);
    shader->uniform("gbuf_norm", gbuffer->color_textures[2], 3);
    shader->uniform("near_far", glm::vec2(current_camera()->near, current_camera()->far));
    shader->uniform("screenres", glm::vec2(Context::resolution()));
    Quad::draw();
    shader->unbind();
}

void blit(const std::shared_ptr<Texture2D>& tex) {
    blit(*tex);
}

void blit(const Texture2D& tex) {
	static auto shader = Shader("blit", "shader/copytex.vs", "shader/copytex.fs");
	shader->bind();
    shader->uniform("tex", tex, 0);
	glDisable(GL_DEPTH_TEST);
    Quad::draw();
	glEnable(GL_DEPTH_TEST);
    shader->unbind();
}
