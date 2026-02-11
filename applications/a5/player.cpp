#include "player.h"
#include <cppgl.h>
#include <glm/glm.hpp>
#include "config.h"

#include <cstdlib>
#include <iostream>

using namespace cppgl;


Player::Player(glm::vec3 _position) : position(_position) {
    prototype = DrawCollection::findOrCreate("render-data/Fratz.obj", "render-data/Fratz.obj");
}

void Player::recalculate_acceleration_input() {
    acceleration_input = glm::vec3(0);

    if(is_moving_forward) {
        acceleration_input.z -= 1;
    }
    if(is_moving_right) {
        acceleration_input.x += 1;
    }
    if(is_moving_backward) {
        acceleration_input.z += 1;
    }
    if(is_moving_left) {
        acceleration_input.x -= 1;
    }
    if(acceleration_input != glm::vec3(0)) {
        acceleration_input = glm::normalize(acceleration_input);
    }
}

void Player::start_moving_in_dir(int dir) {
    switch(dir) {
        case 0:
            is_moving_forward = true;
            break;
        case 1:
            is_moving_right = true;
            break;
        case 2:
            is_moving_backward = true;
            break;
        case 3:
            is_moving_left = true;
            break;
    }
    recalculate_acceleration_input();
}
void Player::stop_moving_in_dir(int dir) {
    switch(dir) {
    case 0:
        is_moving_forward = false;
        break;
    case 1:
        is_moving_right = false;
        break;
    case 2:
        is_moving_backward = false;
        break;
    case 3:
        is_moving_left = false;
        break;
    }
    recalculate_acceleration_input();    
}


void Player::update(float dt) {
    velocity += acceleration_input * config::player::deceleration * dt;
    std::cout << "input: " << acceleration_input << "\n";

    // damping
    if(acceleration_input != glm::vec3(0)) {
        // find orthogonal axis
        glm::vec3 orthogonal_axis = glm::vec3(acceleration_input.z,0, -acceleration_input.x);
        float curr_orthogonal_speed = glm::dot(orthogonal_axis, velocity);
        /* float new_orthogonal_speed = curr_orthogonal_speed - config::player::deceleration*dt;
        new_orthogonal_speed = std::max(new_orthogonal_speed, 0.f);
        float delta_speed = curr_orthogonal_speed - new_orthogonal_speed; */
        float delta_speed2 = std::min(curr_orthogonal_speed, config::player::deceleration*dt);
        glm::vec3 deceleration = -delta_speed2 * orthogonal_axis;
        velocity += deceleration;


    } else {
        float curr_speed = velocity.length();
        float new_speed = curr_speed-config::player::deceleration*dt;
        new_speed = std::max(new_speed, 0.f);
        velocity *= new_speed / velocity.length();
    }

    position += velocity * dt;
}

void Player::draw(cppgl::Shader &shader) {
    glm::mat4 model = glm::scale(glm::mat4(1), glm::vec3(0.3));
    model = glm::rotate(glm::mat4(1), rotation, glm::vec3(0,1,0)) * model;
    model = glm::translate(glm::mat4(1), position) * model;
    model = glm::translate(glm::mat4(1), position) * glm::rotate(glm::mat4(1), rotation, glm::vec3(0,1,0)) * glm::scale(glm::mat4(1), glm::vec3(0.3));
    shader->uniform("model", model);
    shader->uniform("model_normal", glm::transpose(glm::inverse(model)));

    prototype->draw_geometry(shader);

}
