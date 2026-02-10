#include "canvas.h"





Canvas::Canvas(glm::vec3 _pos, glm::vec3 _extents, glm::ivec2 _dims) 
: pos(_pos), extents(_extents), dims(_dims) {
    // create texture on gpu and fill it white
}

Canvas::~Canvas() {
    // delete texture
}


void Canvas::draw(cppgl::Shader &shader) {

}
