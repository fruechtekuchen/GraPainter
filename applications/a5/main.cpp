#include <iostream>

#include <cppgl.h>
#include <fstream>
#include "cmdline.h"
#include "particles.h"
#include "rendering.h"
#include "static-objects.h"
#include "dynamic-objects.h"
#include "clientside-networking.h"
#undef far
#undef near

using namespace std;
using namespace cppgl;

// globals
std::shared_ptr<Player> the_player;

Framebuffer gbuffer;


// ---------------------------------------
// callbacks

void keyboard_callback(int key, int scancode, int action, int mods) {
    if (!reader || !reader->prologue_over()) return;
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS) make_camera_current(Camera::find("default"));
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) make_camera_current(Camera::find("playercam"));
    if (key == GLFW_KEY_F4 && action == GLFW_PRESS) gbuffer_debug = !gbuffer_debug;
    if (key == GLFW_KEY_F5 && action == GLFW_PRESS) {
        static bool wireframe = false;
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    if (current_camera()->name != "playercam") return;
}

void resize_callback(int w, int h) {
    if (gbuffer) gbuffer->resize(w, h);
}

void draw_fog_imgui(bool & draw_fog) {
    if (Context::instance().show_gui) {
        if (ImGui::BeginMainMenuBar()) {
            ImGui::Checkbox("Show Fog", &draw_fog);
            ImGui::Separator();
            ImGui::EndMainMenuBar();
        }
    }
}

void draw_comic_imgui(bool & draw_comic) {
    if (Context::instance().show_gui) {
        if (ImGui::BeginMainMenuBar()) {
            ImGui::Checkbox("Comic Style", &draw_comic);
            ImGui::Separator();
            ImGui::EndMainMenuBar();
        }
    }
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
    if (!parse_cmdline(argc, argv)) return 0;

    // init context and set parameters
    ContextParameters params;
    params.title = "bbm";
    params.font_ttf_filename = EXECUTABLE_DIR + std::string("/render-data/fonts/DroidSansMono.ttf");
    params.font_size_pixels = 15;
    params.width = cmdline.res_x;
    params.height = cmdline.res_y;
    Context::init(params);
    Context::set_keyboard_callback(keyboard_callback);
    Context::set_resize_callback(resize_callback);

    // EXECUTABLE_DIR set via cmake, paths now relative to source/executable directory
    std::filesystem::current_path(EXECUTABLE_DIR);


    auto playercam = Camera("playercam");
    playercam->far = 250;
    make_camera_current(playercam);

    const glm::ivec2 res = Context::resolution();
    gbuffer = Framebuffer("gbuffer", res.x, res.y);
    gbuffer->attach_depthbuffer(Texture2D("gbuf_depth", res.x, res.y, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));
    gbuffer->attach_colorbuffer(Texture2D("gbuf_diff", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->attach_colorbuffer(Texture2D("gbuf_pos", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->attach_colorbuffer(Texture2D("gbuf_norm", res.x, res.y, GL_RGB32F, GL_RGB, GL_FLOAT));
    gbuffer->check();

    //Tests if render-data was correctly found in applications/bbm-a1/render-data
    std::ifstream test_folders("render-data/images/gameover.png");
    if (!test_folders.is_open()) {
        std::cerr << "IMAGE IN RENDER DATA NOT FOUND. Please check if the bbm folder contains the entire render-data folder and download it otherwise." << std::endl;
        throw std::runtime_error("IMAGE IN RENDER DATA NOT FOUND. \
            Please check if the bbm folder contains the entire render-data folder and download it otherwise.; \
            Failed to open image file: render-data/images/gameover.png; ");
        return 1;
    }
    else {
        test_folders.close();
    }


    auto game_over_tex = Texture2D("game-over", "render-data/images/gameover.png");

    init_static_prototypes();
    init_dynamic_prototypes();
    particles = std::make_shared<Particles>(2000, render_settings::particle_size);
    particles_small = std::make_shared<Particles>(3000, render_settings::particle_size * 0.1);

    networking_prologue();

    TimerQuery input_timer("input");
    TimerQuery update_timer("update");
    TimerQueryGL render_timer("render");
    TimerQueryGL postprocess_timer("postprocess");

    // delete later
    bool draw_fog = true;
    the_fog = std::make_shared<Fog>(the_board->getTilesx(), the_board->getTilesy());


    while (Context::running() && game_is_running) {
        // input handling
        input_timer.begin();
        if (current_camera()->name != "playercam")
            CameraImpl::default_input_handler(Context::frame_time());

        reader->read_and_handle();
        current_camera()->update();
        static uint32_t counter = 0;
        if (counter++ % 100 == 0) reload_modified_shaders();
        input_timer.end();

        // update
        update_timer.begin();
        for (auto& player : players)
            player->update();
        the_board->update();
        particles->update();
        particles_small->update();
        update_timer.end();

        // render
        render_timer.begin();
        gbuffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto& player : players)
            player->draw();
        the_board->draw();
        the_floor->draw();
        gbuffer->unbind();
        render_timer.end();

        // postprocess
        postprocess_timer.begin();
        if (gbuffer_debug)
            deferred_debug_pass(gbuffer);
        else {
            deferred_shading_pass(gbuffer);
            the_skysphere->draw();
            particles->draw();
            particles_small->draw();
        }

        if (draw_fog) {
            the_fog->draw();
        }

        draw_imgui_demo();
        draw_gui();
        draw_fog_imgui(draw_fog);
        draw_comic_imgui(draw_comic_style);
        postprocess_timer.end();

        // finish frame
        Context::swap_buffers();
    }

    Timer game_over_timer;
    while (Context::running() && game_over_timer.look() < 1337) {
        blit(game_over_tex);
        Context::swap_buffers();
    }

    return 0;
}
