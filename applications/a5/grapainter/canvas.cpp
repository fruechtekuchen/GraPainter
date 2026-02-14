#include "canvas.h"
#include <vector>
#include "util.h"



Canvas::Canvas(glm::vec3 _position, glm::vec2 _scale, glm::ivec2 texture_size) 
: position(_position-0.5f*glm::vec3(_scale.x, 0, scale.y)), scale(_scale) {
    // create white texture
    std::vector<float> data = std::vector<float>(texture_size.x*texture_size.y*4, 1.);
    canvas_texture = cppgl::Texture2D("grapainter_canvas_tex", texture_size.x, texture_size.y, GL_RGBA32F, GL_RGBA, GL_FLOAT, static_cast<void *>(data.data()));

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

    // setup compute shaders
    canvas_drawing_shader = cppgl::Shader::findOrCreate("grapainter_drawing_shader", "shader/grapainter_canvas_painting.cs");

    score_calculation_shader = cppgl::Shader::findOrCreate("score_calculation_shader", "shader/grapainter_score_calculation.cs");

    std::vector<uint> init_data(texture_size.x, 0);
    m_score_buffer = cppgl::SSBO("score_buffer", sizeof(uint) * init_data.size());
    m_score_buffer->upload_data(init_data.data(), m_score_buffer->size_bytes);
    
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
    
    auto canvas_texture_size = glm::ivec2(canvas_texture->w, canvas_texture->h);
    glm::vec2 brush_pos_board = glm::vec2(world_pos.x-position.x, world_pos.z-position.z);
    glm::ivec2 brush_pixel_pos = glm::vec2(canvas_texture_size) * brush_pos_board / scale;
    glm::ivec2 start_pixel = brush_pixel_pos - glm::ivec2(brush_size);
    glm::ivec2 end_pixel = start_pixel + glm::ivec2(2*brush_size);
    start_pixel = glm::clamp(start_pixel, glm::ivec2(0), canvas_texture_size);
    end_pixel = glm::clamp(end_pixel, glm::ivec2(0), canvas_texture_size);
    glm::ivec2 dimensions = end_pixel - start_pixel;


    // launch compute shader

    canvas_drawing_shader->bind();
    canvas_texture->bind_image(0, GL_READ_WRITE, GL_RGBA32F);

    canvas_drawing_shader->uniform("start_corner", start_pixel);
    canvas_drawing_shader->uniform("brush_center", brush_pixel_pos);
    canvas_drawing_shader->uniform("brush_size", brush_size);
    canvas_drawing_shader->uniform("paint_color", color);
    canvas_drawing_shader->uniform("delta_time", delta_time);

    canvas_drawing_shader->dispatch_compute(dimensions.x, dimensions.y, 1);

    canvas_drawing_shader->unbind();
}

int Canvas::get_score(cppgl::Texture2D image) {
    auto tex_size = glm::ivec2(canvas_texture->w, canvas_texture->h);

    
    
    score_calculation_shader->bind();
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_score_buffer->id);
    canvas_texture->bind_image(1, GL_READ_WRITE, GL_RGBA32F);
    score_calculation_shader->uniform("image_texture", image, 2);

    score_calculation_shader->dispatch_compute(1, tex_size.y, 1, GL_ALL_BARRIER_BITS); // automatically sets all memory bits

    uint score = 0;
    // process data from gpu
    auto score_buffer_gpu = static_cast<uint*>(m_score_buffer->map(GL_READ_ONLY));
    for (int i=0; i<tex_size.x; i++) {
        score += score_buffer_gpu[i];
    }
    m_score_buffer->unmap();

    std::cout << "score: " << score << "/" << tex_size.x*tex_size.y << "\n";
    float percentage_pixels_correct = static_cast<float>(score)/(tex_size.x*tex_size.y);
    std::cout << "percentage_pixels_correct: " << percentage_pixels_correct << "\n";

    return score;
}
