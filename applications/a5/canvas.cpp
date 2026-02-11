#include "canvas.h"
#include <vector>
#include "util.h"





Canvas::Canvas(glm::vec3 _pos, glm::vec3 _scale, glm::ivec2 _dims) 
: pos(_pos), scale(_scale), dims(_dims) {
    
    // create white texture
    std::vector<float> data = std::vector<float>(dims.x*dims.y*3, 1.);
    float color_val = 0;
    for(size_t i = 0; i < data.size(); i++) {
        data[i] = color_val;
        color_val += 0.1358239034;
        if(color_val > 1.f) color_val -= 1.f;
    }
    texture = cppgl::Texture2D("grapainter_canvas_tex", dims.x, dims.y, GL_RGB32F, GL_RGB, GL_FLOAT, static_cast<void *>(data.data()));

    glTextureParameteri(texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // setup mesh
    glm::vec3 vertices[4] = { {0,0,0}, {0,0,1}, {1,0,1}, {1,0,0} };
    glm::vec3 normals[4] = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
    glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
    unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
    auto mesh = cppgl::Mesh("grapainter_canvas_mesh");
    mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);
    mesh->add_vertex_buffer(GL_FLOAT, 3, 4, normals);
    mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
    mesh->add_index_buffer(6, indices);

    // setup material
    auto mat = cppgl::Material("grapainter_canvas_mat");
    mat->add_texture("diffuse", texture);
    mesh->material = mat;
    
    // setup shader
    auto shader = cppgl::Shader("grapainter_canvas_shader", "shader/canvas.vs", "shader/canvas.fs");
    prototype = cppgl::Drawelement("canvas", shader, mesh);
}

Canvas::~Canvas() {
    // delete texture
}


void Canvas::draw(cppgl::Shader &shader) {
    glm::mat4 model = glm::scale(glm::mat4(1.f), scale);
    model = glm::translate(glm:mat4(1.f), pos);
    shader->uniform("model", model);
    shader->uniform("model_normal", transpose(inverse(model)));
    
    cppgl::Mesh &mesh = prototype->mesh;
    mesh->bind(shader);
    mesh->draw();
    mesh->unbind();
}
