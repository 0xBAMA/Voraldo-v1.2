#ifndef ENGINE
#define ENGINE

#include "includes.h"

class engine {
public:
  engine();
  ~engine();

private:
  SDL_Window *window;
  SDL_GLContext GLcontext;

  // screen dimensions
  int total_screen_width;
  int total_screen_height;

  // wrapper for the grisly details
  GLContainer GPU_Data;

  // cleaner call from constructor
  void init() {
    SDL2_setup();           // all SDL setup, window hidden
    gl_setup();             // gl3w init, glEnables, blendfunc
    GPU_Data.init();        // wrapper for GPU-side setup
    imgui_setup();          // colors, other config
    SDL_ShowWindow(window); // show the window when done

    GPU_Data.xor_block(true, 0);
  }

  // init helper functions
  void SDL2_setup();
  void gl_setup();
  void imgui_setup();

  // the program's main loop
  void main_loop() {
    update_fps_history();      // get new fps value
    GPU_Data.display();        // GPU-side action
    draw_windows();            // ImGUI windows
    SDL_GL_SwapWindow(window); // swap double buffers
    handle_events();           // SDL input handling
  }

  // main loop helper functions
  void update_fps_history();
  void draw_windows();
  void handle_events();

  // due to how much is going on here, this is broken out
  void draw_user_editor_tab_contents();

  // shows general control window
  bool show_voraldo_window = true;
  void show_voraldo_menu(bool *open);

  // shows the fps history
  bool show_fps_overlay = true;
  std::deque<float> fps_history;
  void fps_overlay(bool *p_open);

  // ImGuizmo
  void orientation_widget_imgui();

  // everything associated with quitting
  bool quitconfirm = false;   // show quit prompt
  void quit_conf(bool *open); // draws quit prompt window
  bool pquit = false;         // loop control on main loop
  void quit();                // close window, delete GPU data

public:
  // OBJ data (per mesh) - used in OBJ loading
  void load_OBJ(std::string filename);
  // this may vary in length, as they are indexed into independently
  std::vector<glm::vec4> vertices;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> texcoords;
  // these should all be the same length, the number of triangles
  std::vector<glm::ivec3> triangle_indices;
  std::vector<glm::ivec3> normal_indices;
  std::vector<glm::ivec3> texcoord_indices;
};

#endif
