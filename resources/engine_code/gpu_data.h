#ifndef GPU_DATA
#define GPU_DATA

#include "includes.h"

class GLContainer {
public:
  GLContainer() {}
  ~GLContainer() {}

  // GPU side initialization
  void init() {
    // compile_shaders();
    // buffer_geometry();
    // load_textures();
  }

  // display functions
  bool show_widget = true;
  void display() {
    display_block();
    if (show_widget)
      display_orientation_widget();
  }

  // OpenGL clear color
  glm::vec4 clear_color;

  // screen dimensions
  unsigned int screen_width, screen_height;

private:
  // display helper functions
  void display_block();
  void display_orientation_widget();

  // // OpenGL Data
  //  // blitting via fullscreen geometry
  //  GLuint display_vao, display_vbo,
  //      display_shader_program =
  //          Shader("resources/engine_code/shaders/blit.vs.glsl",
  //                  "resources/engine_code/shaders/blit.fs.glsl")
  //              .Program;

  //    // orientation widget with phong shading
  //    GLuint owidget_vao, owidget_vbo,
  //        owidget_shader_program =
  //            Shader("resources/engine_code/shaders/widget.vs.glsl",
  //                   "resources/engine_code/shaders/widget.fs.glsl")
  //                .Program;
};

#endif
