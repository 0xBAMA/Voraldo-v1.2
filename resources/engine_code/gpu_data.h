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
    main_block_linear_filter();
    init_basis();
    animation_worker.set_parent(this);
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
  void set_redraw_flag() { redraw_flag = true; }

  // OpenGL clear color
  glm::vec4 clear_color;

  // screen dimensions
  unsigned int screen_width, screen_height;

  // basis vectors
  glm::vec3 basisx, basisy, basisz;

  // initialize the basis vectors
  void init_basis();

  // apply the rotation to the basis vectors about the x and y axes
  void rotate_vertical(float amnt);   // up and down arrows
  void rotate_horizontal(float amnt); // left and right arrows
  void rolltate(float amnt); // page up / page down to roll about the z axis

  // cardinal directions
  void view_front();
  void view_back();
  void view_right();
  void view_left();
  void view_up();
  void view_down();

  // to set type of renderer
  void main_block_image();
  void main_block_linear_filter();
  void main_block_nearest_filter();

  // settings variables
  glm::vec3 orientation_widget_offset;
  float alpha_correction_power = 2.0;
  int color_temp = 6500;
  int tonemap_mode = 1;

  float scale = 5.;
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
                             float cone_angle, glm::vec4 color,
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
  void generate_perlin_noise(float xscale, float yscale, float zscale,
                             int seed);
  void gen_noise(int preset, int seed);
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
  // the worker class closely integrates here
  class worker {
  public:
    GLContainer *parent;
    void set_parent(GLContainer *my_parent) { parent = my_parent; }

    bool pop(); // do an operation, report if oplist is empty
    std::vector<json> oplist;
  } animation_worker;

  void log(std::string text);          // for operation logging
  void clear_log();                    // clear the log
  void save_log(std::string filename); // save the current log
  void
  load_log(std::string filename); // load log and add to animation worker's list
  // void run_list(json j);               // list of operations in json
  std::vector<std::string> operations; // log of all operations

private:
  enum rendermode_t { IMAGE, NEAREST, LINEAR } rendermode = LINEAR;

  bool redraw_flag = true;       // need to update render texture
  bool color_mipmap_flag = true; // need to recompute mipmap before render
  bool light_mipmap_flag = true; // need to recompute mipmap before render

  // this is better than rebinding textures, it is either 0 or 1
  int tex_offset = 0; // the method by which front/back are toggled

  // when mipmap_flag is true, this is used in the display function
  void color_mipmap_gen();
  void light_mipmap_gen();

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
  GLuint copy_loadbuff_compute; // for VAT and load
  GLuint user_compute;          // user script editor

  // Lighting
  GLuint lighting_clear_compute;
  GLuint directional_lighting_compute;
  GLuint point_lighting_compute;
  GLuint cone_lighting_compute;
  GLuint ambient_occlusion_compute;
  GLuint fakeGI_compute;
  GLuint mash_compute;
};

#endif
