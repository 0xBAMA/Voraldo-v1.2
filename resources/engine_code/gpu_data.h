#ifndef GPU_DATA
#define GPU_DATA

#include "includes.h"

class GLContainer {
public:
  GLContainer() {}
  ~GLContainer() {}

  // GPU side initialization
  void init() {
    compile_shaders();
    buffer_geometry();
    load_textures();
    // main_block_image();
    main_block_linear_filter();
  }

  // display functions
  bool show_widget = true;
  void display() {
    display_block();
    if (show_widget)
      display_orientation_widget();
  }

  // swapping blocks
  void swap_blocks() { tex_offset = tex_offset == 1 ? 0 : 1; }

  // OpenGL clear color
  glm::vec4 clear_color;

  // screen dimensions
  unsigned int screen_width, screen_height;

  // basis vectors
  glm::vec3 basisx = glm::vec3(-1., 0., 0.), basisy = glm::vec3(0., -1., 0.),
            basisz = glm::vec3(0., 0., 1.);

  // initialize the basis vectors
  void init_basis() {
    redraw_flag = true;
    basisx = glm::vec3(-1., 0., 0.);
    basisy = glm::vec3(0., -1., 0.);
    basisz = glm::vec3(0., 0., 1.);
  }

  // apply the rotation to the basis vectors about the x and y axes
  void rotate_vertical(float amnt) {
    basisx = glm::rotate(basisx, amnt, glm::vec3(1, 0, 0));
    basisy = glm::rotate(basisy, amnt, glm::vec3(1, 0, 0));
    basisz = glm::rotate(basisz, amnt, glm::vec3(1, 0, 0));
    redraw_flag = true;
  }

  void rotate_horizontal(float amnt) {
    basisx = glm::rotate(basisx, amnt, glm::vec3(0, 1, 0));
    basisy = glm::rotate(basisy, amnt, glm::vec3(0, 1, 0));
    basisz = glm::rotate(basisz, amnt, glm::vec3(0, 1, 0));
    redraw_flag = true;
  }

  void rolltate(float amnt) {
    basisx = glm::rotate(basisx, amnt, glm::vec3(0, 0, 1));
    basisy = glm::rotate(basisy, amnt, glm::vec3(0, 0, 1));
    basisz = glm::rotate(basisz, amnt, glm::vec3(0, 0, 1));
    redraw_flag = true;
  }

  void view_front();
  void view_back();
  void view_right();
  void view_left();
  void view_up();
  void view_down();

  // use an image object instead of samplers
  void main_block_image() {
    rendermode = IMAGE;
    redraw_flag = true;
  }

  // set linear filtering
  void main_block_linear_filter() {
    rendermode = LINEAR;
    glBindTexture(GL_TEXTURE_3D, textures[2]); // front color
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_3D, textures[3]); // back color
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_3D, textures[6]); // lighting
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    redraw_flag = true;
  }
  // set nearest filtering
  void main_block_nearest_filter() {
    rendermode = NEAREST;
    glBindTexture(GL_TEXTURE_3D, textures[2]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, textures[3]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, textures[6]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    redraw_flag = true;
  }

  // settings variables
  glm::vec3 orientation_widget_offset;
  float alpha_correction_power = 2.0;
  int color_temp = 6500;
  int tonemap_mode = 2;

  float scale = 7.;
  int clickndragx = 0;
  int clickndragy = 0;

  // Functions as invoked from the menu

  // --
  // lighting

  void lighting_clear(bool use_cache, glm::vec4 clear_level);

  void compute_point_lighting(glm::vec3 light_position, glm::vec4 color,
                              float point_decay_power,
                              float point_distance_power);

  void compute_cone_lighting(glm::vec3 location, float theta, float phi,
                             float cone_angle, float initial_intensity,
                             float decay_power, float distance_power);

  void compute_new_directional_lighting(float theta, float phi,
                                        glm::vec4 initial_ray_intensity,
                                        float decay_power);

  void compute_fake_GI(float factor, glm::vec4 sky_intensity, float thresh);

  void compute_ambient_occlusion(int radius);

  void mash();

  // --
  // shapes

  void draw_aabb(glm::vec3 min, glm::vec3 max, glm::vec4 color, bool aabb_draw,
                 int aabb_mask);

  void draw_cuboid(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                   glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h,
                   glm::vec4 color, bool cuboid_draw, int cuboid_mask);

  void draw_cylinder(glm::vec3 cylinder_bvec, glm::vec3 cylinder_tvec,
                     float cylinder_radius, glm::vec4 color, bool cylinder_draw,
                     int cylinder_mask);

  void draw_ellipsoid(glm::vec3 center, glm::vec3 radius, glm::vec3 rotation,
                      glm::vec4 color, bool ellipsoid_draw, int ellipsoid_mask);

  void draw_grid(glm::ivec3 spacing, glm::ivec3 wid, glm::ivec3 off,
                 glm::vec3 rot, glm::vec4 color, bool draw, int mask);

  // heightmap algorithms
  void generate_heightmap_XOR();
  void generate_heightmap_perlin();
  void generate_heightmap_diamond_square();
  void draw_heightmap(float height_scale, bool height_color, glm::vec4 color,
                      bool draw, int mask);

  // noise algorithms
  //   this is where the FastNoise2 generation goes
  void generate_perlin_noise(float xscale, float yscale, float zscale);
  void draw_noise(float low_thresh, float high_thresh, bool smooth,
                  glm::vec4 color, bool draw, int mask);

  void draw_regular_icosahedron(double x_rot, double y_rot, double z_rot,
                                double scale, glm::vec3 center_point,
                                glm::vec4 vertex_material,
                                double verticies_radius,
                                glm::vec4 edge_material, double edge_thickness,
                                glm::vec4 face_material, float face_thickness,
                                bool draw, bool mask);

  void draw_sphere(glm::vec3 location, float radius, glm::vec4 color, bool draw,
                   int mask);

  void draw_tube(glm::vec3 bvec, glm::vec3 tvec, float inner_radius,
                 float outer_radius, glm::vec4 color, bool draw, int mask);

  void draw_triangle(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3,
                     float thickness, glm::vec4 color, bool draw, int mask);

  // --
  // utilities

  void clear_all(bool respect_mask);

  void unmask_all();
  void invert_mask();
  void mask_by_color(bool r, bool g, bool b, bool a, bool l, glm::vec4 color,
                     float l_val, float r_var, float g_var, float b_var,
                     float a_var, float l_var, int mask);

  void box_blur(int radius, bool touch_alpha, bool respect_mask);
  void gaussian_blur(int radius, bool touch_alpha, bool respect_mask);

  void limiter();

  void shift(glm::ivec3 movement, bool loop, int mode);

  // --

  std::string compile_user_script(std::string text);
  std::string run_user_script();

  // --

  void copy_loadbuffer(bool respect_mask);

  void load(std::string filename, bool respect_mask);

  void save(std::string filename);

  std::string vat(float flip, std::string rule, int initmode, glm::vec4 color0,
                  glm::vec4 color1, glm::vec4 color2, float lambda, float beta,
                  float mag, bool respect_mask, glm::bvec3 mins,
                  glm::bvec3 maxs);

  // --

private:
  enum rendermode_t { IMAGE, NEAREST, LINEAR } rendermode = LINEAR;

  bool redraw_flag = true;       // need to update render texture
  bool color_mipmap_flag = true; // need to recompute mipmap before render
  bool light_mipmap_flag = true; // need to recompute mipmap before render

  // this is better than rebinding textures, it is either 0 or 1
  int tex_offset = 0; // the method by which front/back are toggled

  // when mipmap_flag is true, this is used in the display function
  void color_mipmap_gen() {
    auto t1 = std::chrono::high_resolution_clock::now();
    // bind front buffer texture to GL_TEXTURE_3D
    glBindTexture(GL_TEXTURE_3D, textures[2 + tex_offset]);
    // compute the mipmap
    glGenerateMipmap(GL_TEXTURE_3D);
    cout << "main color block mipmap generated in "
         << std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t1)
                .count()
         << " microseconds" << endl;
  }

  void light_mipmap_gen() {
    // same for the lighting buffer
    auto t1 = std::chrono::high_resolution_clock::now();
    glBindTexture(GL_TEXTURE_3D, textures[6]);
    glGenerateMipmap(GL_TEXTURE_3D);
    cout << "light block mipmap generated in "
         << std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - t1)
                .count()
         << " microseconds" << endl;
  }
  // init helper functions
  void compile_shaders();
  void buffer_geometry();
  void load_textures();
  void cube_geometry(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
                     glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h,
                     std::vector<glm::vec3> &points,
                     std::vector<glm::vec3> &normals,
                     std::vector<glm::vec3> &colors, glm::vec3 color);

  // display helper functions
  void display_block();
  void display_orientation_widget();

  // log of all operations
  std::vector<std::string> operations;

  // OpenGL Data
  // the two versions of the raycast shader
  GLuint display_compute_image, display_compute_sampler;

  // blitting via fullscreen geometry
  GLuint display_vao, display_vbo, display_shader_program;

  // orientation widget with phong shading
  GLuint owidget_vao, owidget_vbo, owidget_shader_program;

  // texture handles
  GLuint textures[13];

  // Compute Shader Handles and associated functions
  // Shapes
  GLuint aabb_compute;
  GLuint cuboid_compute;
  GLuint cylinder_compute;
  GLuint ellipsoid_compute;
  GLuint grid_compute;
  GLuint heightmap_compute;
  GLuint noise_compute;
  GLuint sphere_compute;
  GLuint tube_compute;
  GLuint triangle_compute;

  // GPU-side utilities
  GLuint clear_all_compute;
  GLuint unmask_all_compute;
  GLuint invert_mask_compute;
  GLuint mask_by_color_compute;
  GLuint box_blur_compute;
  GLuint gaussian_blur_compute;
  GLuint shift_compute;
  GLuint copy_loadbuff_compute; // done - for VAT and load
  GLuint user_compute;          // from the user script editor

  // Lighting
  GLuint lighting_clear_compute;       // done
  GLuint directional_lighting_compute; // done
  GLuint point_lighting_compute;       // done
  GLuint cone_lighting_compute;
  GLuint ambient_occlusion_compute; // done
  GLuint fakeGI_compute;
  GLuint mash_compute; // done
};

#endif
