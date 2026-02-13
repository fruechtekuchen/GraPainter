#include <iostream>

#include <cppgl.h>
#include <fstream>
#include "particles.h"
#include "rendering.h"
#include "grapainter/game.h"
#undef far
#undef near

using namespace std;
using namespace cppgl;

// globals
bool game_is_running = true;
std::unique_ptr<Grapainter> the_game;

cppgl::Shader copy_tex_shader;

Framebuffer gbuffer;


// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) make_camera_current(Camera::find("default"));
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) make_camera_current(Camera::find("playercam"));
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        static bool wireframe = false;
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    the_game->keyboard_callback(key, scancode, action, mods);
    
}

void resize_callback(int w, int h) {
    if (gbuffer) gbuffer->resize(w, h);
}

void draw_imgui_demo(){
    //HINT: This is a demo on how to use ImGUI to set and print variables
    //HINT: You can call ImGui from anywhere in the code

    //Variables for gui state
    static bool gui_show_Demo = false;
    static bool gui_show_ImGuiDemo = false;
    //Check for the F1 status
    if(Context::instance().show_gui){
        //Add checkbox to Menubar
        if (ImGui::BeginMainMenuBar()) {
            //Demo Menu
            ImGui::Checkbox("Demo", &gui_show_Demo);
            ImGui::Separator();
            ImGui::EndMainMenuBar();
        }
    }

    //Your variables for debugging
    static bool your_bool = false;
    static float your_float = 1.0f;
    static glm::vec3 your_color(1,0,0);
    static glm::vec3 your_vec3(1,0,0);

    //Check if Demo should be visible
    if (gui_show_Demo) {
        //Fix the Window size for the next Window
        ImGui::SetNextWindowSize(ImVec2(570,250));
        //Begin a Window
        if (ImGui::Begin("Demo", &gui_show_Demo)) {

            ImGui::Checkbox("This is a checkbox, to set a bool variable", &your_bool);
            ImGui::Text(std::string("This is a text element, to print variables: YourBool: " + std::to_string(your_bool)).c_str());
            if(ImGui::Button("This is a button, to execute some code")){/* code or function call */}
            ImGui::SliderFloat("This is a float slider", &your_float, 0.0f, 100.f, "%.3f");
            ImGui::ColorEdit3("This is a color picker", &your_color[0]);
            ImGui::DragFloat3("This is a vec3 element", &your_vec3[0], 0.001f);
            ImGui::Separator(); // This is a separating line between elements

            ImGui::Text("For more information have a look at the full ImGui Demo:");
            ImGui::Text("(bbm-a1/common/src/imgui/imgui-demo.cpp ->Widgets)");
            ImGui::Checkbox("Show Full ImGui Demo", &gui_show_ImGuiDemo);
            if(gui_show_ImGuiDemo){
                ImGui::ShowDemoWindow();
            }
        }
        // End a Window
        ImGui::End();
    }
}

// ---------------------------------------
// main

int main(int argc, char** argv) {
    // init context and set parameters
    ContextParameters params;
    params.title = "grapainter";
    params.font_ttf_filename = EXECUTABLE_DIR + std::string("/render-data/fonts/DroidSansMono.ttf");
    params.font_size_pixels = 15;
    Context::init(params);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_resize_callback(resize_callback);

    // EXECUTABLE_DIR set via cmake, paths now relative to source/executable directory
    std::filesystem::current_path(EXECUTABLE_DIR);


    const glm::ivec2 res = Context::resolution();
    gbuffer = Framebuffer("gbuffer", res.x, res.y);
    gbuffer->attach_depthbuffer(Texture2D("gbuf_depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
    gbuffer->attach_colorbuffer(Texture2D("gbuf_diff", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->attach_colorbuffer(Texture2D("gbuf_pos", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->attach_colorbuffer(Texture2D("gbuf_norm", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->check();

    copy_tex_shader = cppgl::Shader("copy_tex_shader", "shader/pos+norm+tc.vs", "shader/pos+norm+tc.fs");

    the_game = std::make_unique<Grapainter>();

    while (Context::running() && game_is_running) {
        // input handling
        if (current_camera()->name == "default")
            CameraImpl::default_input_handler(Context::frame_time());

        current_camera()->update();
        static uint32_t counter = 0;
        if (counter++ % 100 == 0) reload_modified_shaders();

        // update
        the_game->update();

        // render
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        copy_tex_shader->bind();
        setup_geometry_shader(copy_tex_shader);
        the_game->draw(copy_tex_shader);
        copy_tex_shader->unbind();
        gbuffer->unbind();

        deferred_shading_pass(gbuffer);
        
        draw_imgui_demo();

        // finish frame
        Context::swap_buffers();
    }


    return 0;
}
