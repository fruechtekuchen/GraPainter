#include <sstream>
#include "drawcollection.h"

using namespace cppgl;

DrawCollectionImpl::DrawCollectionImpl(const std::string& name, const std::string& path, const glm::mat4 &model) : name(name), model(model) {
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

void DrawCollectionImpl::draw_geometry(cppgl::Shader &shader, const glm::mat4 &model) {
    const auto m = model * this->model;
    shader->uniform("model", m);
    shader->uniform("model_normal", transpose(inverse(m)));
    draw_geometry(shader);
}

void DrawCollectionImpl::draw_geometry(cppgl::Shader &shader) {
    for (auto &elem : prototype) {
        auto &mesh = elem->mesh;
        mesh->bind(shader);
        mesh->draw();
        mesh->unbind();
    }
}
