#include "player.h"
#include <cppgl.h>
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "config.h"
#include "util.h"

#include <cstdlib>
#include <iostream>

using namespace cppgl;


Player::Player(glm::vec3 _position) : position(_position) {
    prototype = DrawCollection::findOrCreate("render-data/fratz/Fratz.obj", "render-data/fratz/Fratz.obj");
    util::print_mesh_info(prototype);
    for(auto &prot : prototype->prototype) {
            for(auto pair : prot->mesh->material->texture_map) {
                auto name = pair.first;
                auto tex = pair.second;
                glTextureParameteri(tex->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(tex->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }
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
    velocity += acceleration_input * config::player::acceleration * dt;

    float speed = glm::length(velocity);
    // slowing down
    if(speed > 0 ) {
        speed -= config::player::deceleration*dt;
        speed = std::max(0.f, speed);
        
        // we are not on the autobahn, obey to speed limits
        speed = std::min(speed, config::player::max_speed);
        velocity *= speed/glm::length(velocity);
    }

    position += velocity * dt;
    // adjust rotation
    if(speed > 0.1) {
        rotation = glm::orientedAngle(glm::normalize(glm::vec2(velocity.x, velocity.z)), glm::vec2(0,1));
    }
    
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
