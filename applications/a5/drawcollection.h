#pragma once

#include <cppgl.h>
#include <string>
#include <vector>
#include "named_handle.h"

class DrawCollectionImpl {
public:
    DrawCollectionImpl(const std::string& name = "", const std::string& path = "", const glm::mat4 &model = glm::mat4(1));

    std::vector<cppgl::Drawelement> prototype;

    std::string name;
    glm::mat4 model;
    glm::vec3 mod_col = glm::vec3(1.f);

    void draw_geometry(cppgl::Shader &shader, const glm::mat4 &model, const std::vector<std::pair<std::string, glm::vec3>> &tints = {});
    // Apply DrawCollection::model manually to shader if using this method.
    void draw_geometry(cppgl::Shader &shader, const std::vector<std::pair<std::string, glm::vec3>> &tints = {});

    void draw_geometry_instanced(cppgl::Shader &shader, const std::vector<glm::mat4> &instance_models, const std::vector<std::pair<std::string, glm::vec3>> &tints = {});
};

using DrawCollection = cppgl::NamedHandle<DrawCollectionImpl>;
