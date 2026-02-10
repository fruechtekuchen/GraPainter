#include "player.h"
#include <cppgl.h>
#include <glm/glm.hpp>


#include <cstdlib>
#include <iostream>

using namespace std;
using namespace cppgl;


Player::Player(){
    prototype = DrawCollection::findOrCreate("render-data/Fratz.obj", "render-data/Fratz.obj");
}


void Player::update() {

}

void Player::draw(cppgl::Shader &shader) {
    glm::mat4 model = glm::scale(glm::mat4(1), glm::vec3(1.));
    model = model * glm::rotate(glm::mat4(1), rotation, glm::vec3(0,1,0));
    model = model * glm::translate(glm::mat4(1), position);

    shader->uniform("model", model);
    shader->uniform("model_normal", glm::transpose(glm::inverse(model)));
    
    prototype->draw_geometry(shader);

}
