#pragma once
#include <vector>
#include <cppgl.h>


class Imageframe {
private:
    cppgl::Drawelement m_prototype;
    glm::vec3 m_position;
    glm::vec2 m_scale;
    cppgl::Texture2D m_texture;

public:
    Imageframe(glm::vec3 pos, glm::vec2 scale={1,1}, std::string filename) 
    :m_position(pos), m_scale(scale) {

        m_texture = cppgl::Texture2D("grapainter_imageframe_tex", filename);

        glTextureParameteri(m_texture->id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_texture->id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_texture->id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_texture->id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // setup mesh
        glm::vec3 vertices[4] = { {0,0,0}, {0,0,1}, {1,0,1}, {1,0,0} };
        glm::vec3 normals[4] = { {0,1,0}, {0,1,0}, {0,1,0}, {0,1,0} };
        glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
        unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
        auto mesh = cppgl::Mesh("grapainter_imageframe_mesh");
        mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);
        mesh->add_vertex_buffer(GL_FLOAT, 3, 4, normals);
        mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
        mesh->add_index_buffer(6, indices);

        // setup material
        auto mat = cppgl::Material("grapainter_imageframe_mat");
        mat->add_texture("diffuse", m_texture);
        mesh->material = mat;
        
        // setup drawing shader
        auto shader = cppgl::Shader::findOrCreate("grapainter_imageframe_shader", "shader/pos+norm+tc.vs", "shader/pos+norm+tc.fs");
        m_prototype = cppgl::Drawelement("canvas", shader, mesh);
    }

    void draw(cppgl::Shader &shader) {
        glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(m_scale.x,1,m_scale.y));
        model = glm::translate(glm::mat4(1.f), m_position) * model;
        shader->uniform("model", model);
        shader->uniform("model_normal", transpose(inverse(model)));
        
        cppgl::Mesh &mesh = m_prototype->mesh;
        mesh->bind(shader);
        mesh->draw();
        mesh->unbind();
    }
};