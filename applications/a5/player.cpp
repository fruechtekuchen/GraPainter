#include "player.h"
#include <cppgl.h>
#include <glm/gtc/matrix_transform.hpp>


#include <cstdlib>
#include <iostream>

using namespace std;
using namespace cppgl;


Player::Player(){
    prototype = DrawCollection::findOrCreate("render-data/Fratz.obj", "render-data/Fratz.obj");
}


void Player::update() {

}

void Player::draw() {
    
}
