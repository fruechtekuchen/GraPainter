#pragma once

#include <cppgl.h>

class Canvas {

public:
    Canvas(glm::vec3 pos, glm::vec3 scale={10,1,10}, glm::ivec2 dims={100,100});
    ~Canvas();
    void draw(cppgl::Shader &shader);

private:
    cppgl::Shader shader;
    cppgl::Texture2D texture;
    cppgl::Drawelement prototype;
    glm::vec3 pos;
    glm::vec3 scale;
    glm::ivec2 dims;
};