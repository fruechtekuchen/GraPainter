#include "canvas.h"
#include <vector>
#include "util.h"



Canvas::Canvas(glm::vec3 _position, glm::vec2 _scale, glm::ivec2 _texture_size) 
: position(_position), scale(_scale), canvas_texture_size(_texture_size) {
    
    // create white texture
    std::vector<float> data = std::vector<float>(canvas_texture_size.x*canvas_texture_size.y*4, 1.);
    canvas_texture = cppgl::Texture2D("grapainter_canvas_tex", canvas_texture_size.x, canvas_texture_size.y, GL_RGBA32F, GL_RGBA, GL_FLOAT, static_cast<void *>(data.data()));

    glTextureParameteri(canvas_texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(canvas_texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(canvas_texture->id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(canvas_texture->id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
    mat->add_texture("diffuse", canvas_texture);
    mesh->material = mat;
    
    // setup drawing shader
    auto shader = cppgl::Shader("grapainter_canvas_shader", "shader/canvas.vs", "shader/canvas.fs");
    prototype = cppgl::Drawelement("canvas", shader, mesh);

    // setup compute shader for painting
    canvas_shader = cppgl::Shader::findOrCreate("grapainter_drawing_shader", "shader/grapainter_canvas_painting.cs");
    
}

Canvas::~Canvas() {
    // delete texture
}


void Canvas::draw(cppgl::Shader &shader) {
    glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(scale.x,1,scale.y));
    model = glm::translate(glm::mat4(1.f), position) * model;
    shader->uniform("model", model);
    shader->uniform("model_normal", transpose(inverse(model)));
    
    cppgl::Mesh &mesh = prototype->mesh;
    mesh->bind(shader);
    mesh->draw();
    mesh->unbind();
}

bool Canvas::is_in_bounds(glm::vec3 world_pos) {
    return world_pos.x > position.x 
    && world_pos.z > position.z 
    && world_pos.x < (position.x+scale.x) 
    && world_pos.z < (position.z+scale.y);
}

void Canvas::try_paint(glm::vec3 world_pos, glm::vec3 color) {
    if(!is_in_bounds(world_pos)) return;
    const auto delta_time = static_cast<float>(cppgl::Context::frame_time()/1000.);
    

    glm::vec2 brush_pos_board = glm::vec2(world_pos.x-position.x, world_pos.z-position.z);
    glm::ivec2 brush_pixel_pos = glm::vec2(canvas_texture_size) * brush_pos_board / scale;
    glm::ivec2 start_pixel = brush_pixel_pos - glm::ivec2(brush_size);
    glm::ivec2 end_pixel = start_pixel + glm::ivec2(2*brush_size);
    start_pixel = glm::clamp(start_pixel, glm::ivec2(0), canvas_texture_size);
    end_pixel = glm::clamp(end_pixel, glm::ivec2(0), canvas_texture_size);
    glm::ivec2 dimensions = end_pixel - start_pixel;


    // launch compute shader

    canvas_shader->bind();
    canvas_texture->bind_image(0, GL_READ_WRITE, GL_RGBA32F);

    canvas_shader->uniform("start_corner", start_pixel);
    canvas_shader->uniform("brush_center", brush_pixel_pos);
    canvas_shader->uniform("brush_size", brush_size);
    canvas_shader->uniform("paint_color", color);
    canvas_shader->uniform("delta_time", delta_time);

    canvas_shader->dispatch_compute(dimensions.x, dimensions.y, 1);

    canvas_shader->unbind();
}