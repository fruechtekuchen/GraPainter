#include "game.h"

Grapainter::Grapainter() {
    std::string file_to_paint = "render-data/images/kirby_pixel.png";
    m_the_player = std::make_shared<Player>(glm::vec3(0));
    m_the_camera = std::make_shared<Playercam>(10.f, glm::vec3(0,-0.5,-0.70), m_the_player);
    m_the_canvas = std::make_shared<Canvas>(glm::vec3(0,0,0), glm::vec2(8,8));
    m_the_image = std::make_shared<Imageframe>(file_to_paint, glm::vec3(0,0,-6), glm::vec2(5,5), 1.5);
}

void Grapainter::keyboard_callback(int key, int scancode, int action, int mods)  {
if (key == GLFW_KEY_W && action == GLFW_PRESS) m_the_player->start_moving_in_dir(0);
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) m_the_player->stop_moving_in_dir(0);
    if (key == GLFW_KEY_D && action == GLFW_PRESS) m_the_player->start_moving_in_dir(1);
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) m_the_player->stop_moving_in_dir(1);
    if (key == GLFW_KEY_S && action == GLFW_PRESS) m_the_player->start_moving_in_dir(2);
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) m_the_player->stop_moving_in_dir(2);
    if (key == GLFW_KEY_A && action == GLFW_PRESS) m_the_player->start_moving_in_dir(3);
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) m_the_player->stop_moving_in_dir(3);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) m_is_drawing = true;
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) m_is_drawing = false;

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) m_drawing_color.x = m_drawing_color.x == 0 ? 1 : 0;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) m_drawing_color.y = m_drawing_color.y == 0 ? 1 : 0;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) m_drawing_color.z = m_drawing_color.z == 0 ? 1 : 0;
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) m_the_player->set_sprinting(true);
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) m_the_player->set_sprinting(false);

    if (key == GLFW_KEY_M && action == GLFW_RELEASE) m_the_canvas->get_score(m_the_image->get_texture());
}

void Grapainter::mouse_button_callback(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) m_is_drawing = true;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) m_is_drawing = false;
}


void Grapainter::update()  {
    double dt = cppgl::Context::instance().frame_time()/1000;
    game_duration += dt;
    if(!game_ended && game_duration > time_to_draw) {
        game_ended = true;
        int score = m_the_canvas->get_score(m_the_image->get_texture());
    }
    m_the_player->update(dt);
    m_the_camera->update(dt);
    if(m_is_drawing && !game_ended) {
        m_the_canvas->try_paint(m_the_player->position, m_drawing_color);
    }
}

void Grapainter::draw(cppgl::Shader &shader)  {
    m_the_player->draw(shader);
    m_the_canvas->draw(shader);
    m_the_image->draw(shader);
}