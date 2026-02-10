#pragma once

#include <cppgl.h>

class Canvas {

public:
    Canvas(glm::vec3 pos, glm::vec3 extents, glm::ivec2 dims);
    ~Canvas();
    void draw(cppgl::Shader &shader);

private:
    cppgl::Shader shader;
    glm::vec3 pos;
    glm::vec3 extents;
    glm::ivec2 dims;
};