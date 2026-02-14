#pragma once

#include <cppgl.h>

class Canvas {

public:
    Canvas(glm::vec3 pos, glm::vec2 scale={10,10}, glm::ivec2 texture_size={100,100});
    ~Canvas();
    void draw(cppgl::Shader &shader);
    void try_paint(glm::vec3 world_pos, glm::vec3 color);
    void set_brush_size(float new_size) { brush_size = std::max(1.f,new_size); }
    float get_brush_size() { return brush_size; }
    int get_score(cppgl::Texture2D image);

private:
    cppgl::Drawelement prototype;
    glm::vec3 position;
    glm::vec2 scale;

    cppgl::Shader canvas_drawing_shader;
    cppgl::Texture2D canvas_texture;
    float brush_size = 5;
    cppgl::Shader score_calculation_shader;
    cppgl::SSBO m_score_buffer;
    bool is_in_bounds(glm::vec3 world_pos);

};