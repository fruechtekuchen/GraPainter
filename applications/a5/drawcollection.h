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

    void draw_geometry(cppgl::Shader &shader, const glm::mat4 &model);

    // Apply DrawCollection::model manually to shader if using this method.
    void draw_geometry(cppgl::Shader &shader);
};

using DrawCollection = cppgl::NamedHandle<DrawCollectionImpl>;
