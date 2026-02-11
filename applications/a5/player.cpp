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

void Player::start_moving_in_dir(int dir) {
    switch(dir) {
        case 0:
            acceleration_input.z = 1; break;
        case 1:
            acceleration_input.x = 1; break;
        case 2:
            acceleration_input.z = -1; break;
        case 3:
            acceleration_input.x = -1; break;
    }
}
void Player::stop_moving_in_dir(int dir) {
    switch(dir) {
        case 0:
            if(acceleration_input.z > 0) {
                acceleration_input.z = 0;
                break;
            }
        case 1:
            if(acceleration_input.x > 0) {
                acceleration_input.x = 0;
                break;
            }
        case 2:
            if(acceleration_input.z < 0) {
                acceleration_input.z = 0;
                break;
            }
        case 3:
            if(acceleration_input.x < 0) {
                acceleration_input.x = 0;
                break;
            }
    }
}

void Player::start_rotating_in_dir(int dir) {
    switch(dir) {
        case -1:
            rotation_input += 1; break;
        case 1:
            rotation_input -= 1; break;
    }
}
void Player::stop_rotating_in_dir(int dir) {
    switch(dir) {
        case -1:
            rotation_input -= 1; break;
        case 1:
            rotation_input += 1; break;
    }
}

void Player::update(float dt) {
    // rotational speed
    rotation_vel += rotation_input*config::player::rotation_acceleration*dt;
    if(abs(rotation_vel) > config::player::max_rotation_speed) {
        rotation_vel =  config::player::max_rotation_speed / abs(rotation_vel) * rotation_vel;
    }
    rotation += rotation_vel*dt;

    
    glm::vec3 rotated_acceleration = glm::rotate(glm::mat4(1.), rotation, glm::vec3(0,1,0)) * glm::vec4(acceleration_input,0);
    
    velocity += config::player::acceleration*dt* rotated_acceleration;

    // damping
    glm::vec3 local_vel = glm::rotate(glm::mat4(1.), -rotation, glm::vec3(0,1,0)) * glm::vec4(velocity, 0);
    std::cout << local_vel << "\n";
    if(acceleration_input.z == 0 && local_vel.z != 0) {
        int sign = local_vel.z/abs(local_vel.z);
        local_vel.z += -sign * config::player::deceleration*dt;
        if(sign > 0 && local_vel.z < 0) local_vel.z = 0;
        if(sign < 0 && local_vel.z > 0) local_vel.z = 0;
        std::cout << local_vel << "\n";
    }
    /* if(acceleration_input.x == 0) {
        int sign = local_vel.x/abs(local_vel.x);
        local_vel.x += -sign * config::player::deceleration*dt;
        if(sign > 0 && local_vel.x < 0) local_vel.x = 0;
        if(sign < 0 && local_vel.x > 0) local_vel.x = 0;
        
    } */
    velocity = glm::rotate(glm::mat4(1.), rotation, glm::vec3(0,1,0)) * glm::vec4(local_vel, 0);

    // clamping
    if(velocity.length() > config::player::max_speed) {
        velocity = config::player::max_speed / velocity.length() * velocity;
    }
    position += dt * velocity;
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
