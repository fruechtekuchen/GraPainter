#include <iostream>

#include <cppgl.h>
#include <fstream>
#include "particles.h"
#include "rendering.h"
#include "player.h"
#include "playercam.h"
#include "canvas.h"
#undef far
#undef near

using namespace std;
using namespace cppgl;

// globals
bool game_is_running = true;
bool is_drawing = false;
glm::vec3 drawing_color{0,1,1};
std::shared_ptr<Player> the_player;
std::shared_ptr<Playercam> the_camera;
std::shared_ptr<Canvas> the_canvas;

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
    if (key == GLFW_KEY_W && action == GLFW_PRESS) the_player->start_moving_in_dir(0);
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) the_player->stop_moving_in_dir(0);
    if (key == GLFW_KEY_D && action == GLFW_PRESS) the_player->start_moving_in_dir(1);
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) the_player->stop_moving_in_dir(1);
    if (key == GLFW_KEY_S && action == GLFW_PRESS) the_player->start_moving_in_dir(2);
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) the_player->stop_moving_in_dir(2);
    if (key == GLFW_KEY_A && action == GLFW_PRESS) the_player->start_moving_in_dir(3);
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) the_player->stop_moving_in_dir(3);

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) is_drawing = true;
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) is_drawing = false;

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) drawing_color.x = drawing_color.x == 0 ? 1 : 0;
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) drawing_color.y = drawing_color.y == 0 ? 1 : 0;
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) drawing_color.z = drawing_color.z == 0 ? 1 : 0;
    
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

    the_player = std::make_shared<Player>(glm::vec3(0));
    the_camera = std::make_shared<Playercam>(10.f, glm::vec3(0,-0.5,-0.70), the_player);
    the_canvas = std::make_shared<Canvas>(glm::vec3(-5,0,-5));

    while (Context::running() && game_is_running) {
        // input handling
        if (current_camera()->name == "default")
            CameraImpl::default_input_handler(Context::frame_time());

        current_camera()->update();
        static uint32_t counter = 0;
        if (counter++ % 100 == 0) reload_modified_shaders();

        // update
        double dt = cppgl::Context::instance().frame_time()/1000;
        the_player->update(dt);
        the_camera->update(dt);
        if(is_drawing) {
            the_canvas->try_paint(the_player->position, drawing_color);
        }

        // render
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        copy_tex_shader->bind();
        setup_geometry_shader(copy_tex_shader);
        the_player->draw(copy_tex_shader);
        the_canvas->draw(copy_tex_shader);
        copy_tex_shader->unbind();
        gbuffer->unbind();

        deferred_shading_pass(gbuffer);
        
        draw_imgui_demo();

        // finish frame
        Context::swap_buffers();
    }


    return 0;
}
