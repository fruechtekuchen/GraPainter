#include <sstream>
#include "drawcollection.h"

using namespace cppgl;

DrawCollectionImpl::DrawCollectionImpl(const std::string& name, const std::string& path, const glm::mat4 &model)
    : name(name), model(model) {

    if (path.size() == 0) return;

    std::stringstream ss(path);
    std::string p;
    while (std::getline(ss, p, ';')) {
        auto meshes = load_meshes_gpu(p);
        for (auto &m : meshes) {
            prototype.push_back(Drawelement::findOrCreate(m->name, Shader(), m));
        }
    }
}

void DrawCollectionImpl::draw_geometry(cppgl::Shader &shader, const glm::mat4 &model, const std::vector<std::pair<std::string, glm::vec3>> &tints) {
    const auto m = model * this->model;
    shader->uniform("model", m);
    shader->uniform("model_normal", transpose(inverse(m)));
    draw_geometry(shader, tints);
}

void DrawCollectionImpl::draw_geometry(cppgl::Shader &shader, const std::vector<std::pair<std::string, glm::vec3>> &tints) {
    for (auto &elem : prototype) {
        auto &mesh = elem->mesh;
        mesh->bind(shader);

        for (const auto& tint : tints) {
            if (elem->name.find(tint.first) != std::string::npos) {
                mod_col = tint.second;
                break;
            }
        }
        shader->uniform("mod_col", mod_col);

        mesh->draw();
        mesh->unbind();
    }
}

void DrawCollectionImpl::draw_geometry_instanced(cppgl::Shader &shader, const std::vector<glm::mat4> &instance_models, const std::vector<std::pair<std::string, glm::vec3>> &tints) {
    if (instance_models.empty()) return;

    shader->uniform("use_instancing", true);

    for (auto &elem : prototype) {
        auto &mesh = elem->mesh;

        mesh->setup_instancing(instance_models);
        mesh->bind(shader);

        for (const auto& tint : tints) {
            if (elem->name.find(tint.first) != std::string::npos) {
                mod_col = tint.second;
                break;
            }
        }
        shader->uniform("mod_col", mod_col);

        mesh->draw_instanced(instance_models.size());
        mesh->unbind();
        mesh->cleanup_instancing();
    }

    shader->uniform("use_instancing", false);
}
