#pragma once
#include <cppgl.h>
#include "player.h"
#include "playercam.h"
#include "canvas.h"
#include "imageframe.h"
#include <memory>

class Grapainter {
public:
    Grapainter();
    void keyboard_callback(int key, int scancode, int action, int mods);
    void update();
    void draw(cppgl::Shader &shader);

private:
    bool m_is_drawing = false;
    glm::vec3 m_drawing_color{0,1,1};
    std::shared_ptr<Player> m_the_player;
    std::shared_ptr<Playercam> m_the_camera;
    std::shared_ptr<Canvas> m_the_canvas;
    std::shared_ptr<Imageframe> m_the_image;
};