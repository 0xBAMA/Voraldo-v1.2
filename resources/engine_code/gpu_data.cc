#include "gpu_data.h"
#include "includes.h"

void GLContainer::display_block() {

  // ------------------------
  // compute shader raycasts, puts result into texture

  // check for state updates
  static float temp_scale;
  static float temp_clickndragx;
  static float temp_clickndragy;
  static float acp; // alpha correction power
  // color temperature, done this way so it hooks on the first frame
  static int temp_temperature = 0;
  static glm::vec4 temp_clear_color;

  if ((temp_scale != scale) || (temp_clickndragx != clickndragx) ||
      (temp_clickndragy != clickndragy) || (acp != alpha_correction_power) ||
      (clear_color != temp_clear_color))
    redraw_flag = true;

  temp_scale = scale;
  temp_clickndragx = clickndragx;
  temp_clickndragy = clickndragy;
  acp = alpha_correction_power;
  temp_clear_color = clear_color;

  if (redraw_flag) {

    auto t1 = std::chrono::high_resolution_clock::now();

    // regen mipmap if needed
    if (color_mipmap_flag) {
      // regnerate the color mipmap
      color_mipmap_gen();
      color_mipmap_flag = false;
    }

    if (light_mipmap_flag) {
      // generate the light mipmap
      light_mipmap_gen();
      light_mipmap_flag = false;
    }

    // GLuint display_compute_shader = display_compute_image;
    GLuint display_compute_shader = display_compute_sampler;
    // pick a display compute shader based on rendermode
    switch (rendermode) {
    case IMAGE:
      display_compute_shader = display_compute_image;
      break;
    case NEAREST:
    case LINEAR:
    default:
      display_compute_shader = display_compute_sampler;
      break;
    }

    // do the tile based rendering using the raycast compute shader
    glUseProgram(display_compute_shader);

    // display texture
    glUniform1i(glGetUniformLocation(display_compute_shader, "current"), 0);
    glUniform1i(glGetUniformLocation(display_compute_shader, "block"),
                2 + tex_offset);
    glUniform1i(glGetUniformLocation(display_compute_shader, "lighting"), 6);

    // basis vectors
    glUniform3f(glGetUniformLocation(display_compute_shader, "basis_x"),
                basisx.x, basisx.y, basisx.z);
    glUniform3f(glGetUniformLocation(display_compute_shader, "basis_y"),
                basisy.x, basisy.y, basisy.z);
    glUniform3f(glGetUniformLocation(display_compute_shader, "basis_z"),
                basisz.x, basisz.y, basisz.z);

    // zoom parameter
    glUniform1f(glGetUniformLocation(display_compute_shader, "scale"), scale);

    // click and drag
    glUniform1i(glGetUniformLocation(display_compute_shader, "clickndragx"),
                clickndragx);
    glUniform1i(glGetUniformLocation(display_compute_shader, "clickndragy"),
                clickndragy);

    // clear color
    glUniform4fv(glGetUniformLocation(display_compute_shader, "clear_color"), 1,
                 glm::value_ptr(clear_color));

    // alpha power
    glUniform1f(glGetUniformLocation(display_compute_shader, "upow"),
                alpha_correction_power);

    // loop through tiles
    for (int x = 0; x < SSFACTOR * screen_width; x += TILESIZE) {
      // update the x offset
      glUniform1i(glGetUniformLocation(display_compute_shader, "x_offset"), x);
      for (int y = 0; y < SSFACTOR * screen_height; y += TILESIZE) {
        // update the y offset
        glUniform1i(glGetUniformLocation(display_compute_shader, "y_offset"),
                    y);

        // dispatch tiles
        glDispatchCompute(TILESIZE / 32, TILESIZE / 32, 1);
      }
    }

    // make sure everything finishes before blitting
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    auto t2 = std::chrono::high_resolution_clock::now();

    cout << "tiled refresh took "
         << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1)
                .count()
         << " microseconds" << endl;

    redraw_flag =
        false; // we won't need to draw anything again, till something changes
  }

  // clear the screen
  glClearColor(clear_color.x, clear_color.y, clear_color.z,
               clear_color.w);                        // from hsv picker
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the background

  // ------------------------
  // display shader takes texture and blits it to the screen

  glUseProgram(display_shader_program);
  glBindVertexArray(display_vao);
  glBindBuffer(GL_ARRAY_BUFFER, display_vbo);

  // color temperature
  if (temp_temperature != color_temp) {
    temp_temperature = color_temp;
    glm::vec3 col = get_color_for_temp(double(color_temp));
    glUniform3f(glGetUniformLocation(display_shader_program, "temp_adjustment"),
                col.x, col.y, col.z);
  }

  // tonemapping setting
  glUniform1i(glGetUniformLocation(display_shader_program, "ACES_behavior"),
              tonemap_mode);

  // pixel scaling
  glUniform1f(glGetUniformLocation(display_shader_program, "ssfactor"),
              SSFACTOR);

  // one triangle, 3 verticies
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLContainer::display_orientation_widget() {
  glUseProgram(owidget_shader_program);
  glBindVertexArray(owidget_vao);
  glBindBuffer(GL_ARRAY_BUFFER, owidget_vbo);

  ImGuiIO &io = ImGui::GetIO();

  glUniform3f(glGetUniformLocation(owidget_shader_program, "basis_x"), basisx.x,
              basisx.y, basisx.z);
  glUniform3f(glGetUniformLocation(owidget_shader_program, "basis_y"), basisy.x,
              basisy.y, basisy.z);
  glUniform3f(glGetUniformLocation(owidget_shader_program, "basis_z"), basisz.x,
              basisz.y, basisz.z);

  glUniform1f(glGetUniformLocation(owidget_shader_program, "ratio"),
              io.DisplaySize.x / io.DisplaySize.y);

  glUniform3fv(glGetUniformLocation(owidget_shader_program, "offset"), 1,
               glm::value_ptr(orientation_widget_offset));

  // 4 cubes, 6 faces apiece, 2 triangles per face - total is 144 verticies
  glDrawArrays(GL_TRIANGLES, 0, 144);
}

void GLContainer::compile_shaders() {

  // display geometry
  display_shader_program = Shader("resources/engine_code/shaders/blit.vs.glsl",
                                  "resources/engine_code/shaders/blit.fs.glsl")
                               .Program;

  // orientation widget
  owidget_shader_program =
      Shader("resources/engine_code/shaders/widget.vs.glsl",
             "resources/engine_code/shaders/widget.fs.glsl")
          .Program;

  // raycasting
  display_compute_image =
      CShader("resources/engine_code/shaders/raycast.cs.glsl").Program;

  // raycasting, but with compute shaders
  display_compute_sampler =
      CShader("resources/engine_code/shaders/raycast_sampler.cs.glsl").Program;

  // lighting functions
  lighting_clear_compute =
      CShader("resources/engine_code/shaders/light_clear.cs.glsl").Program;
  point_lighting_compute =
      CShader("resources/engine_code/shaders/point_light.cs.glsl").Program;
  directional_lighting_compute =
      CShader("resources/engine_code/shaders/directional_light.cs.glsl")
          .Program;
  ambient_occlusion_compute =
      CShader("resources/engine_code/shaders/ambient_occlusion.cs.glsl")
          .Program;
  mash_compute = CShader("resources/engine_code/shaders/mash.cs.glsl").Program;

  // utility functions
  copy_loadbuff_compute =
      CShader("resources/engine_code/shaders/copy_loadbuff.cs.glsl").Program;
  // clear_all_compute = ;
  // unmask_all_compute;
  // invert_mask_compute;
  // mask_by_color_compute;
  // box_blur_compute;
  // gaussian_blur_compute;
  // shift_compute;

  // shape functions
  aabb_compute = CShader("resources/engine_code/shaders/aabb.cs.glsl").Program;
  cuboid_compute =
      CShader("resources/engine_code/shaders/cuboid.cs.glsl").Program;
  cylinder_compute =
      CShader("resources/engine_code/shaders/cylinder.cs.glsl").Program;
  ellipsoid_compute =
      CShader("resources/engine_code/shaders/ellipsoid.cs.glsl").Program;
  grid_compute = CShader("resources/engine_code/shaders/grid.cs.glsl").Program;
  heightmap_compute =
      CShader("resources/engine_code/shaders/heightmap.cs.glsl").Program;

  // noise_compute;

  sphere_compute =
      CShader("resources/engine_code/shaders/sphere.cs.glsl").Program;
  tube_compute = CShader("resources/engine_code/shaders/tube.cs.glsl").Program;
  triangle_compute =
      CShader("resources/engine_code/shaders/triangle.cs.glsl").Program;
}

void GLContainer::buffer_geometry() {
  clickndragx = -200 * SSFACTOR;

  std::vector<glm::vec3> points;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec3> colors;

  // based on this, one triangle is significantly faster than two
  // https://michaldrobot.com/2014/04/01/gcn-execution-patterns-in-full-screen-passes/
  points.push_back(glm::vec3(-1, -1, 0.5)); // A
  points.push_back(glm::vec3(3, -1, 0.5));  // B
  points.push_back(glm::vec3(-1, 3, 0.5));  // C

  // vao, vbo
  cout << "display geometry init................................";
  glGenVertexArrays(1, &display_vao);
  glBindVertexArray(display_vao);

  glGenBuffers(1, &display_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, display_vbo);

  // buffer the data
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), NULL,
               GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * points.size(),
                  &points[0]);

  // set up attributes
  GLuint points_attrib =
      glGetAttribLocation(display_shader_program, "vPosition");
  glEnableVertexAttribArray(points_attrib);
  glVertexAttribPointer(points_attrib, 3, GL_FLOAT, GL_FALSE, 0,
                        (GLvoid *)(static_cast<const char *>(0) + (0)));

  cout << "done." << endl;

  points.clear();
  normals.clear();
  colors.clear();

  // ------------------------
  // orientation widget, to indicate the orientation of the block

  // this is going to consist of 4 rectangular prisms
  //  1 - small grey cube centered at the origin
  //  2 - red,   extended along the x axis
  //  3 - green, extended along the y axis
  //  4 - blue,  extended along the z axis

  //      e-------g    +y
  //     /|      /|     |
  //    / |     / |     |___+x
  //   a-------c  |    /
  //   |  f----|--h   +z
  //   | /     | /
  //   |/      |/
  //   b-------d

  cout << "orientation widget geometry init.....................";

#define POS 0.125f
#define NEG -0.125f

  glm::vec3 a(NEG, POS, POS);
  glm::vec3 b(NEG, NEG, POS);
  glm::vec3 c(POS, POS, POS);
  glm::vec3 d(POS, NEG, POS);
  glm::vec3 e(NEG, POS, NEG);
  glm::vec3 f(NEG, NEG, NEG);
  glm::vec3 g(POS, POS, NEG);
  glm::vec3 h(POS, NEG, NEG);

  glm::vec3 offset;
  float factor = 0.25f;
  float length = 2.0f;

#define LFF(X) glm::vec3(X.x *length, X.y *factor, X.z *factor)
#define FLF(X) glm::vec3(X.x *factor, X.y *length, X.z *factor)
#define FFL(X) glm::vec3(X.x *factor, X.y *factor, X.z *length)

  offset = glm::vec3(POS, 0, 0);
  glm::vec3 ax = LFF((a + offset));
  glm::vec3 bx = LFF((b + offset));
  glm::vec3 cx = LFF((c + offset));
  glm::vec3 dx = LFF((d + offset));
  glm::vec3 ex = LFF((e + offset));
  glm::vec3 fx = LFF((f + offset));
  glm::vec3 gx = LFF((g + offset));
  glm::vec3 hx = LFF((h + offset));

  offset = glm::vec3(0, POS, 0);
  glm::vec3 ay = FLF((a + offset));
  glm::vec3 by = FLF((b + offset));
  glm::vec3 cy = FLF((c + offset));
  glm::vec3 dy = FLF((d + offset));
  glm::vec3 ey = FLF((e + offset));
  glm::vec3 fy = FLF((f + offset));
  glm::vec3 gy = FLF((g + offset));
  glm::vec3 hy = FLF((h + offset));

  offset = glm::vec3(0, 0, NEG);
  glm::vec3 az = FFL((a + offset));
  glm::vec3 bz = FFL((b + offset));
  glm::vec3 cz = FFL((c + offset));
  glm::vec3 dz = FFL((d + offset));
  glm::vec3 ez = FFL((e + offset));
  glm::vec3 fz = FFL((f + offset));
  glm::vec3 gz = FFL((g + offset));
  glm::vec3 hz = FFL((h + offset));

  cube_geometry(a, b, c, d, e, f, g, h, points, normals, colors,
                glm::vec3(0.618f, 0.618f, 0.618f));
  cube_geometry(ax, bx, cx, dx, ex, fx, gx, hx, points, normals, colors,
                glm::vec3(0.618f, 0.180f, 0.180f));
  cube_geometry(ay, by, cy, dy, ey, fy, gy, hy, points, normals, colors,
                glm::vec3(0.180f, 0.618f, 0.180f));
  cube_geometry(az, bz, cz, dz, ez, fz, gz, hz, points, normals, colors,
                glm::vec3(0.180f, 0.180f, 0.618f));

  // vao, vbo
  glGenVertexArrays(1, &owidget_vao);
  glBindVertexArray(owidget_vao);

  glGenBuffers(1, &owidget_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, owidget_vbo);

  // buffer the data
  int num_bytes_points = sizeof(glm::vec3) * points.size();
  int num_bytes_normals = sizeof(glm::vec3) * normals.size();
  int num_bytes_colors = sizeof(glm::vec3) * colors.size();
  int total_size = num_bytes_points + num_bytes_normals + num_bytes_colors;

  glBufferData(GL_ARRAY_BUFFER, total_size, NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes_points, &points[0]);
  glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points, num_bytes_normals,
                  &normals[0]);
  glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_normals,
                  num_bytes_colors, &colors[0]);

  // set up attributes
  GLuint points_attribute =
      glGetAttribLocation(owidget_shader_program, "vPosition");
  glEnableVertexAttribArray(points_attribute);
  glVertexAttribPointer(points_attribute, 3, GL_FLOAT, GL_FALSE, 0,
                        (GLvoid *)(static_cast<const char *>(0) + (0)));

  GLuint normals_attribute =
      glGetAttribLocation(owidget_shader_program, "vNormal");
  glEnableVertexAttribArray(normals_attribute);
  glVertexAttribPointer(
      normals_attribute, 3, GL_FLOAT, GL_FALSE, 0,
      (GLvoid *)(static_cast<const char *>(0) + (num_bytes_points)));

  GLuint colors_attribute =
      glGetAttribLocation(owidget_shader_program, "vColor");
  glEnableVertexAttribArray(colors_attribute);
  glVertexAttribPointer(colors_attribute, 3, GL_FLOAT, GL_FALSE, 0,
                        (GLvoid *)(static_cast<const char *>(0) +
                                   (num_bytes_points + num_bytes_normals)));
  cout << "done." << endl;
}

// helpers for below
void tri(glm::vec3 a, glm::vec3 b, glm::vec3 c,
         std::vector<glm::vec3> &points) {
  points.push_back(a);
  points.push_back(b);
  points.push_back(c);
}

void face(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
          std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals,
          std::vector<glm::vec3> &colors, glm::vec3 color) {
  glm::vec3 normal = glm::normalize(glm::cross(a - b, a - c));
  tri(a, b, c, points);
  tri(c, b, d, points);
  for (int i = 0; i < 6; i++) {
    normals.push_back(normal);
    colors.push_back(color);
  }
}

void GLContainer::cube_geometry(glm::vec3 a, glm::vec3 b, glm::vec3 c,
                                glm::vec3 d, glm::vec3 e, glm::vec3 f,
                                glm::vec3 g, glm::vec3 h,
                                std::vector<glm::vec3> &points,
                                std::vector<glm::vec3> &normals,
                                std::vector<glm::vec3> &colors,
                                glm::vec3 color) {
  //      e-------g    +y
  //     /|      /|     |
  //    / |     / |     |___+x
  //   a-------c  |    /
  //   |  f----|--h   +z
  //   | /     | /
  //   |/      |/
  //   b-------d

  face(a, b, c, d, points, normals, colors, color); // face ABCD
  face(c, d, g, h, points, normals, colors, color); // face CDGH
  face(e, g, f, h, points, normals, colors, color); // face EGFH
  face(a, e, b, f, points, normals, colors, color); // face AEBF
  face(a, e, c, g, points, normals, colors, color); // face AECG
  face(b, d, f, h, points, normals, colors, color); // face BDFH
}

void GLContainer::load_textures() {

  // data arrays
  std::vector<unsigned char> ucxor, light, zeroes, random;

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned char> distribution(0, 255);

  random.resize(8 * screen_height * screen_width * SSFACTOR * SSFACTOR, 64);
  light.resize(4 * DIM * DIM * DIM, 64); // fill the array with '64'
  zeroes.resize(4 * DIM * DIM * DIM, 0); // fill the array with zeroes

  cout << "generating init xor texture.....";
  PerlinNoise p;

  for (unsigned int x = 0; x < DIM; x++) {
    for (unsigned int y = 0; y < DIM; y++) {
      for (unsigned int z = 0; z < DIM; z++) {
        for (int i = 0; i < 4; i++) // fill r, g, b with the result of the xor
        {
          ucxor.push_back(((unsigned char)(x % 256) ^ (unsigned char)(y % 256) ^
                           (unsigned char)(z % 256)));
          // light.push_back(i - 3 % 4 ? (i % 2 ? 64. : 128.) *
          //                            p.noise(x * 0.01, y * 0.01, z * 0.01)
          //                           : 255);
        }
      }
    }
  }

  cout << "....done." << endl;

  cout << "Creating texture handles...";
  // create all the texture handles
  glGenTextures(13, &textures[0]);
  cout << "...........done." << endl;

  class MyNumPunct : public std::numpunct<char> {
  protected:
    virtual char do_thousands_sep() const { return ','; }
    virtual std::string do_grouping() const { return "\03"; }
  };

  // supposed to add separators to numbers
  std::cout.imbue(std::locale(std::locale::classic(), new MyNumPunct));

  cout << "rendertextures ("
       << int(SSFACTOR * SSFACTOR * screen_width * screen_height * 8)
       << " bytes).....";
  // main render texture - this is going to be a rectangular texture, larger
  // than the screen so we can do some supersampling
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_RECTANGLE, textures[0]);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16, screen_width * SSFACTOR,
               screen_height * SSFACTOR, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               &random[0]);
  glBindImageTexture(
      0, textures[0], 0, GL_TRUE, 0, GL_READ_WRITE,
      GL_RGBA16); // 16 bits, hopefully higher precision is helpful
  // set up filtering for this texture

  // copy/paste buffer render texture - this is going to be a small rectangular
  // texture, will only be shown inside the menus
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, 512, 256, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(1, textures[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16);

  cout << "...........done." << endl;

  cout << "color voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 4 * 2 << " bytes).......";
  // main block front color buffer - initialize with xor
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_3D, textures[2]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &ucxor[0]);
  glBindImageTexture(2, textures[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  // main block back color buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_3D, textures[3]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(3, textures[3], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  cout << "...........done." << endl;

  cout << "mask voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 2 << " bytes).......";

  // main block front mask buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_3D, textures[4]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, DIM, DIM, DIM, 0, GL_RED_INTEGER,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(4, textures[4], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);

  // main block back mask buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_3D, textures[5]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, DIM, DIM, DIM, 0, GL_RED_INTEGER,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(5, textures[5], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);

  cout << "...........done." << endl;

  cout << "light buffer voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 2 << " bytes).......";

  // display lighting buffer - initialize with some base value representing
  // neutral coloration
  glActiveTexture(GL_TEXTURE0 + 6);
  glBindTexture(GL_TEXTURE_3D, textures[6]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &light[0]);
  glBindImageTexture(6, textures[6], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  // lighting cache buffer - this is going to have the same data in it as the
  // regular lighting buffer initially
  glActiveTexture(GL_TEXTURE0 + 7);
  glBindTexture(GL_TEXTURE_3D, textures[7]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &light[0]);
  glBindImageTexture(7, textures[7], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  cout << "...........done." << endl;

  // copy/paste front buffer - initally empty
  glActiveTexture(GL_TEXTURE0 + 8);
  glBindTexture(GL_TEXTURE_3D, textures[8]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(8, textures[8], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // copy/paste back buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 9);
  glBindTexture(GL_TEXTURE_3D, textures[9]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(9, textures[9], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // load buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 10);
  glBindTexture(GL_TEXTURE_3D, textures[10]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(10, textures[10], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // noise - initialize with noise at some default scaling
  glActiveTexture(GL_TEXTURE0 + 11);
  glBindTexture(GL_TEXTURE_3D, textures[11]);

  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  // 3d texture for noise - DIM on a side
  // generate_perlin_noise(0.014, 0.04, 0.014);

  cout << "heightmap............";
  // heightmap - initialize with a generated diamond square heightmap
  glActiveTexture(GL_TEXTURE0 + 12);
  glBindTexture(GL_TEXTURE_2D, textures[12]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  // 2d texture for representation of a heightmap (greyscale - use some channels
  // to hold more data?) - also, DIM on a side
  generate_heightmap_diamond_square();
  cout << "........done." << endl;

  // sets the texture filtering to linear
  main_block_linear_filter();

  // generate a mipmap for the front RGBA buffer
  color_mipmap_gen();
  light_mipmap_gen();
}

void GLContainer::lighting_clear(bool use_cache, glm::vec4 clear_level) {
  glUseProgram(lighting_clear_compute);
  redraw_flag = true;
  light_mipmap_flag = true;

  glUniform1i(glGetUniformLocation(lighting_clear_compute, "lighting"), 6);
  glUniform1i(glGetUniformLocation(lighting_clear_compute, "lighting_cache"),
              7);
  glUniform1i(glGetUniformLocation(lighting_clear_compute, "use_cache"),
              use_cache);
  glUniform4f(glGetUniformLocation(lighting_clear_compute, "intensity"),
              clear_level.x, clear_level.y, clear_level.z, clear_level[3]);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::compute_point_lighting(glm::vec3 light_position,
                                         glm::vec4 color,
                                         float point_decay_power,
                                         float point_distance_power) {
  glUseProgram(point_lighting_compute);
  redraw_flag = true;
  light_mipmap_flag = true;

  glUniform1i(glGetUniformLocation(point_lighting_compute, "lighting"), 6);
  glUniform1i(glGetUniformLocation(point_lighting_compute, "lighting_cache"),
              7);
  glUniform1i(glGetUniformLocation(point_lighting_compute, "block"),
              2 + tex_offset);
  glUniform3f(glGetUniformLocation(point_lighting_compute, "light_position"),
              light_position.x, light_position.y, light_position.z);
  glUniform1f(glGetUniformLocation(point_lighting_compute, "decay_power"),
              point_decay_power);
  glUniform1f(glGetUniformLocation(point_lighting_compute, "distance_power"),
              point_distance_power);
  glUniform4f(glGetUniformLocation(point_lighting_compute, "light_intensity"),
              color.x, color.y, color.z, color[3]);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::compute_cone_lighting(glm::vec3 location, float theta,
                                        float phi, float cone_angle,
                                        float initial_intensity,
                                        float decay_power,
                                        float distance_power) {
  redraw_flag = true;
  light_mipmap_flag = true;
  glUseProgram(cone_lighting_compute);

  glUniform3fv(glGetUniformLocation(cone_lighting_compute, "light_position"), 1,
               glm::value_ptr(location));

  glUniform1f(glGetUniformLocation(cone_lighting_compute, "theta"), theta);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "phi"), phi);

  glUniform1f(glGetUniformLocation(cone_lighting_compute, "cone_angle"),
              cone_angle);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "light_intensity"),
              initial_intensity);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "decay_power"),
              decay_power);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "distance_power"),
              distance_power);

  glUniform1i(glGetUniformLocation(cone_lighting_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(cone_lighting_compute, "lighting"), 6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::compute_new_directional_lighting(
    float theta, float phi, glm::vec4 initial_ray_intensity,
    float decay_power) {

  redraw_flag = true;
  light_mipmap_flag = true;

  glUseProgram(directional_lighting_compute);

  glUniform1f(glGetUniformLocation(directional_lighting_compute, "utheta"),
              theta);
  glUniform1f(glGetUniformLocation(directional_lighting_compute, "uphi"), phi);
  glUniform4f(
      glGetUniformLocation(directional_lighting_compute, "light_intensity"),
      initial_ray_intensity.r, initial_ray_intensity.g, initial_ray_intensity.b,
      initial_ray_intensity.a);
  glUniform1f(glGetUniformLocation(directional_lighting_compute, "decay_power"),
              decay_power);

  glUniform1i(glGetUniformLocation(directional_lighting_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(directional_lighting_compute, "lighting"),
              6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8); // workgroup is 8x8x8
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// fake GI
void GLContainer::compute_fake_GI(float factor, glm::vec4 sky_intensity,
                                  float thresh) {
  redraw_flag = true;
  light_mipmap_flag = true;

  glUseProgram(fakeGI_compute);

  glUniform1i(glGetUniformLocation(fakeGI_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(fakeGI_compute, "lighting"), 6);

  glUniform1f(glGetUniformLocation(fakeGI_compute, "scale_factor"), factor);
  glUniform1f(glGetUniformLocation(fakeGI_compute, "alpha_thresh"), thresh);
  glUniform4f(glGetUniformLocation(fakeGI_compute, "sky_intensity"),
              sky_intensity.r, sky_intensity.g, sky_intensity.b,
              sky_intensity.a);

  // This has a sequential dependence - from the same guy who did the Voxel
  // Automata Terrain, Brent Werness:
  //   "Totally faked the GI!  It just casts out 9 rays in upwards facing the
  //   lattice directions.
  //    If it escapes it gets light from the sky, otherwise it gets some
  //    fraction of the light from whatever cell it hits.  Run from top to
  //    bottom and you are set!"

  // For that reason, I'm doing 2d workgroups, starting from the top, going to
  // the bottom.

  for (int y = DIM - 1; y >= 0; y--) // iterating through y, from top to bottom
  {
    // update y index
    glUniform1i(glGetUniformLocation(fakeGI_compute, "y_index"), y);

    // send the job, for one xz plane
    glDispatchCompute(DIM / 8, 1, DIM / 8);

    // wait for all those shader invocations to finish
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }
}

void GLContainer::compute_ambient_occlusion(int radius) {
  redraw_flag = true;
  light_mipmap_flag = true;

  glUseProgram(ambient_occlusion_compute);

  glUniform1i(glGetUniformLocation(ambient_occlusion_compute, "radius"),
              radius);

  glUniform1i(glGetUniformLocation(ambient_occlusion_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(ambient_occlusion_compute, "lighting"), 6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::mash() {
  glUseProgram(mash_compute);

  glUniform1i(glGetUniformLocation(mash_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(mash_compute, "lighting"), 6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// SHAPES
void GLContainer::draw_aabb(glm::vec3 min, glm::vec3 max, glm::vec4 color,
                            bool aabb_draw, int aabb_mask) {
  // need to redraw after any drawing operation is done
  redraw_flag = true;
  color_mipmap_flag = true;

  swap_blocks();
  glUseProgram(aabb_compute);

  // Uniforms
  glUniform1i(glGetUniformLocation(aabb_compute, "mask"), aabb_mask);
  glUniform1i(glGetUniformLocation(aabb_compute, "draw"), aabb_draw);
  glUniform4fv(glGetUniformLocation(aabb_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform3fv(glGetUniformLocation(aabb_compute, "mins"), 1,
               glm::value_ptr(min));
  glUniform3fv(glGetUniformLocation(aabb_compute, "maxs"), 1,
               glm::value_ptr(max));

  glUniform1i(glGetUniformLocation(aabb_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(aabb_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(aabb_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(aabb_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::draw_cuboid(glm::vec3 a, glm::vec3 b, glm::vec3 c,
                              glm::vec3 d, glm::vec3 e, glm::vec3 f,
                              glm::vec3 g, glm::vec3 h, glm::vec4 color,
                              bool cuboid_draw, int cuboid_mask) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(cuboid_compute);

  glUniform1i(glGetUniformLocation(cuboid_compute, "mask"), cuboid_mask);
  glUniform1i(glGetUniformLocation(cuboid_compute, "draw"), cuboid_draw);
  glUniform4fv(glGetUniformLocation(cuboid_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform3fv(glGetUniformLocation(cuboid_compute, "a"), 1, glm::value_ptr(a));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "b"), 1, glm::value_ptr(b));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "c"), 1, glm::value_ptr(c));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "d"), 1, glm::value_ptr(d));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "e"), 1, glm::value_ptr(e));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "f"), 1, glm::value_ptr(f));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "g"), 1, glm::value_ptr(g));
  glUniform3fv(glGetUniformLocation(cuboid_compute, "h"), 1, glm::value_ptr(h));

  glUniform1i(glGetUniformLocation(cuboid_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(cuboid_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(cuboid_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(cuboid_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::draw_cylinder(glm::vec3 bvec, glm::vec3 tvec, float radius,
                                glm::vec4 color, bool draw, int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(cylinder_compute);
  glUniform1i(glGetUniformLocation(cylinder_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(cylinder_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(cylinder_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform1fv(glGetUniformLocation(cylinder_compute, "radius"), 1, &radius);
  glUniform3fv(glGetUniformLocation(cylinder_compute, "bvec"), 1,
               glm::value_ptr(bvec));
  glUniform3fv(glGetUniformLocation(cylinder_compute, "tvec"), 1,
               glm::value_ptr(tvec));

  glUniform1i(glGetUniformLocation(cylinder_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(cylinder_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(cylinder_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(cylinder_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::draw_ellipsoid(glm::vec3 center, glm::vec3 radii,
                                 glm::vec3 rotation, glm::vec4 color, bool draw,
                                 int mask) {

  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(ellipsoid_compute);

  glUniform1i(glGetUniformLocation(ellipsoid_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(ellipsoid_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(ellipsoid_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform3fv(glGetUniformLocation(ellipsoid_compute, "radii"), 1,
               glm::value_ptr(radii));
  glUniform3fv(glGetUniformLocation(ellipsoid_compute, "rotation"), 1,
               glm::value_ptr(rotation));
  glUniform3fv(glGetUniformLocation(ellipsoid_compute, "center"), 1,
               glm::value_ptr(center));

  glUniform1i(glGetUniformLocation(ellipsoid_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(ellipsoid_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(ellipsoid_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(ellipsoid_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::draw_grid(glm::ivec3 spacing, glm::ivec3 width,
                            glm::ivec3 offsets, glm::vec3 rot, glm::vec4 color,
                            bool draw, int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(grid_compute);

  glUniform1i(glGetUniformLocation(grid_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(grid_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(grid_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform3i(glGetUniformLocation(grid_compute, "spacing"), spacing.x,
              spacing.y, spacing.z);
  glUniform3i(glGetUniformLocation(grid_compute, "offsets"), offsets.x,
              offsets.y, offsets.z);
  glUniform3i(glGetUniformLocation(grid_compute, "width"), width.x, width.y,
              width.z);
  glUniform3f(glGetUniformLocation(grid_compute, "rotation"), rot.x, rot.y,
              rot.z);

  glUniform1i(glGetUniformLocation(grid_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(grid_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(grid_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(grid_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// heightmap
void GLContainer::draw_heightmap(float height_scale, bool height_color,
                                 glm::vec4 color, bool draw, int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(heightmap_compute);

  glUniform1i(glGetUniformLocation(heightmap_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(heightmap_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(heightmap_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform1i(glGetUniformLocation(heightmap_compute, "height_color"),
              height_color);
  glUniform1i(glGetUniformLocation(heightmap_compute, "map"), 12);
  glUniform1f(glGetUniformLocation(heightmap_compute, "vscale"), height_scale);

  glUniform1i(glGetUniformLocation(heightmap_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(heightmap_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(heightmap_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(heightmap_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void GLContainer::draw_regular_icosahedron(
    double x_rot, double y_rot, double z_rot, double scale,
    glm::vec3 center_point, glm::vec4 vertex_material, double verticies_radius,
    glm::vec4 edge_material, double edge_thickness, glm::vec4 face_material,
    float face_thickness, bool draw, bool mask) {

  double phi = (1 + std::sqrt(5.0)) / 2.0;

  // rotation matricies allowing rotation of the polyhedron
  glm::mat3 rotation_x_axis;
  // refernces [column][row] - sin and cos take arguments in radians
  rotation_x_axis[0][0] = 1;
  rotation_x_axis[1][0] = 0;
  rotation_x_axis[2][0] = 0;
  rotation_x_axis[0][1] = 0;
  rotation_x_axis[1][1] = std::cos(x_rot);
  rotation_x_axis[2][1] = -1.0 * std::sin(x_rot);
  rotation_x_axis[0][2] = 0;
  rotation_x_axis[1][2] = std::sin(x_rot);
  rotation_x_axis[2][2] = std::cos(x_rot);

  glm::mat3 rotation_y_axis;
  rotation_y_axis[0][0] = std::cos(y_rot);
  rotation_y_axis[1][0] = 0;
  rotation_y_axis[2][0] = std::sin(y_rot);
  rotation_y_axis[0][1] = 0;
  rotation_y_axis[1][1] = 1;
  rotation_y_axis[2][1] = 0;
  rotation_y_axis[0][2] = -1.0 * std::sin(y_rot);
  rotation_y_axis[1][2] = 0;
  rotation_y_axis[2][2] = std::cos(y_rot);

  glm::mat3 rotation_z_axis;
  rotation_z_axis[0][0] = std::cos(z_rot);
  rotation_z_axis[1][0] = -1.0 * std::sin(z_rot);
  rotation_z_axis[2][0] = 0;
  rotation_z_axis[0][1] = std::sin(z_rot);
  rotation_z_axis[1][1] = std::cos(z_rot);
  rotation_z_axis[2][1] = 0;
  rotation_z_axis[0][2] = 0;
  rotation_z_axis[1][2] = 0;
  rotation_z_axis[2][2] = 1;

  glm::mat3 rotation = rotation_x_axis * rotation_y_axis *
                       rotation_z_axis; // multiply these all together

  glm::vec3 a, b, c, d, e, f, g, h, i, j, k, l;
  // the work for this is in my journal, the entry on 3/1/2019, done for v06 of
  // Voraldo it is based on the idea that the points of a regular icosahedron
  // lie on the points defined by three mutually orthogonal golden rectangles
  // that share a center point at the origin i.e. these rectanges are abcd,efgh
  // and ijkl
  a = rotation * glm::vec3(0, 1 * scale, phi * scale) + center_point;
  e = rotation * glm::vec3(1 * scale, phi * scale, 0) + center_point;
  i = rotation * glm::vec3(phi * scale, 0, 1 * scale) + center_point;
  b = rotation * glm::vec3(0, 1 * scale, -phi * scale) + center_point;
  f = rotation * glm::vec3(-1 * scale, -phi * scale, 0) + center_point;
  j = rotation * glm::vec3(phi * scale, 0, -1 * scale) + center_point;
  c = rotation * glm::vec3(0, -1 * scale, phi * scale) + center_point;
  g = rotation * glm::vec3(-1 * scale, phi * scale, 0) + center_point;
  k = rotation * glm::vec3(-phi * scale, 0, 1 * scale) + center_point;
  d = rotation * glm::vec3(0, -1 * scale, -phi * scale) + center_point;
  h = rotation * glm::vec3(1 * scale, -phi * scale, 0) + center_point;
  l = rotation * glm::vec3(-phi * scale, 0, -1 * scale) + center_point;
  // nonzero components of the coordinates are scaled by the scale input
  // argument. The result of that operation is multiplied by the composed
  // rotation matrix, then added to the shape's center point

  if (face_thickness) { // draw the faces -
    draw_triangle(a, g, e, face_thickness, face_material, draw, mask); // AGE
    draw_triangle(a, i, e, face_thickness, face_material, draw, mask); // AIE
    draw_triangle(a, c, i, face_thickness, face_material, draw, mask); // ACI
    draw_triangle(a, c, k, face_thickness, face_material, draw, mask); // ACK
    draw_triangle(a, g, k, face_thickness, face_material, draw, mask); // AGK
    draw_triangle(l, b, g, face_thickness, face_material, draw, mask); // LBG
    draw_triangle(l, g, k, face_thickness, face_material, draw, mask); // LGK
    draw_triangle(l, f, k, face_thickness, face_material, draw, mask); // LFK
    draw_triangle(l, d, f, face_thickness, face_material, draw, mask); // LDF
    draw_triangle(l, d, b, face_thickness, face_material, draw, mask); // LDB
    draw_triangle(k, f, c, face_thickness, face_material, draw, mask); // KFC
    draw_triangle(f, h, c, face_thickness, face_material, draw, mask); // FHC
    draw_triangle(h, i, c, face_thickness, face_material, draw, mask); // HIC
    draw_triangle(e, j, i, face_thickness, face_material, draw, mask); // EJI
    draw_triangle(b, g, e, face_thickness, face_material, draw, mask); // BGE
    draw_triangle(f, h, d, face_thickness, face_material, draw, mask); // FHD
    draw_triangle(d, h, j, face_thickness, face_material, draw, mask); // DHJ
    draw_triangle(d, b, j, face_thickness, face_material, draw, mask); // DBJ
    draw_triangle(b, j, e, face_thickness, face_material, draw, mask); // BJE
    draw_triangle(h, i, j, face_thickness, face_material, draw, mask); // HIJ
  }

  if (edge_thickness) { // nonzero value passed for edge thickness
    // draw the edges
    // using cylinders of radius edge_thickness
    draw_cylinder(a, c, edge_thickness, edge_material, draw, mask); // AC
    draw_cylinder(a, e, edge_thickness, edge_material, draw, mask); // AE
    draw_cylinder(a, g, edge_thickness, edge_material, draw, mask); // AG
    draw_cylinder(a, i, edge_thickness, edge_material, draw, mask); // AI
    draw_cylinder(a, k, edge_thickness, edge_material, draw, mask); // AK

    draw_cylinder(b, d, edge_thickness, edge_material, draw, mask); // BD
    draw_cylinder(b, e, edge_thickness, edge_material, draw, mask); // BE
    draw_cylinder(b, g, edge_thickness, edge_material, draw, mask); // BG
    draw_cylinder(b, j, edge_thickness, edge_material, draw, mask); // BJ
    draw_cylinder(b, l, edge_thickness, edge_material, draw, mask); // BL

    draw_cylinder(c, f, edge_thickness, edge_material, draw, mask); // CF
    draw_cylinder(c, h, edge_thickness, edge_material, draw, mask); // CH
    draw_cylinder(c, i, edge_thickness, edge_material, draw, mask); // CI
    draw_cylinder(c, k, edge_thickness, edge_material, draw, mask); // CK

    draw_cylinder(d, f, edge_thickness, edge_material, draw, mask); // DF
    draw_cylinder(d, h, edge_thickness, edge_material, draw, mask); // DH
    draw_cylinder(d, j, edge_thickness, edge_material, draw, mask); // DJ
    draw_cylinder(d, l, edge_thickness, edge_material, draw, mask); // DL

    draw_cylinder(e, g, edge_thickness, edge_material, draw, mask); // EG
    draw_cylinder(e, i, edge_thickness, edge_material, draw, mask); // EI
    draw_cylinder(e, j, edge_thickness, edge_material, draw, mask); // EJ

    draw_cylinder(f, h, edge_thickness, edge_material, draw, mask); // FH
    draw_cylinder(f, k, edge_thickness, edge_material, draw, mask); // FK
    draw_cylinder(f, l, edge_thickness, edge_material, draw, mask); // FL

    draw_cylinder(g, k, edge_thickness, edge_material, draw, mask); // GK
    draw_cylinder(g, l, edge_thickness, edge_material, draw, mask); // GL

    draw_cylinder(h, i, edge_thickness, edge_material, draw, mask); // HI
    draw_cylinder(h, j, edge_thickness, edge_material, draw, mask); // HJ

    draw_cylinder(i, j, edge_thickness, edge_material, draw, mask); // IJ

    draw_cylinder(k, l, edge_thickness, edge_material, draw, mask); // KL
  }

  if (verticies_radius) { // nonzero value passed for vertex radius
    // draw the vertexes
    draw_sphere(a, verticies_radius, vertex_material, draw, mask);
    draw_sphere(b, verticies_radius, vertex_material, draw, mask);
    draw_sphere(c, verticies_radius, vertex_material, draw, mask);
    draw_sphere(d, verticies_radius, vertex_material, draw, mask);

    draw_sphere(e, verticies_radius, vertex_material, draw, mask);
    draw_sphere(f, verticies_radius, vertex_material, draw, mask);
    draw_sphere(g, verticies_radius, vertex_material, draw, mask);
    draw_sphere(h, verticies_radius, vertex_material, draw, mask);

    draw_sphere(i, verticies_radius, vertex_material, draw, mask);
    draw_sphere(j, verticies_radius, vertex_material, draw, mask);
    draw_sphere(k, verticies_radius, vertex_material, draw, mask);
    draw_sphere(l, verticies_radius, vertex_material, draw, mask);
  }
}

// sphere
void GLContainer::draw_sphere(glm::vec3 location, float radius, glm::vec4 color,
                              bool draw, int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  swap_blocks();
  glUseProgram(sphere_compute);

  glUniform1i(glGetUniformLocation(sphere_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(sphere_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(sphere_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform1fv(glGetUniformLocation(sphere_compute, "radius"), 1, &radius);
  glUniform3fv(glGetUniformLocation(sphere_compute, "location"), 1,
               glm::value_ptr(location));

  glUniform1i(glGetUniformLocation(sphere_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(sphere_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(sphere_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(sphere_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// tube
void GLContainer::draw_tube(glm::vec3 bvec, glm::vec3 tvec, float inner_radius,
                            float outer_radius, glm::vec4 color, bool draw,
                            int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  swap_blocks();
  glUseProgram(tube_compute);

  glUniform1i(glGetUniformLocation(tube_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(tube_compute, "draw"), draw);
  glUniform1fv(glGetUniformLocation(tube_compute, "iradius"), 1, &inner_radius);
  glUniform1fv(glGetUniformLocation(tube_compute, "oradius"), 1, &outer_radius);
  glUniform3fv(glGetUniformLocation(tube_compute, "bvec"), 1,
               glm::value_ptr(bvec));
  glUniform3fv(glGetUniformLocation(tube_compute, "tvec"), 1,
               glm::value_ptr(tvec));
  glUniform4fv(glGetUniformLocation(tube_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform1i(glGetUniformLocation(tube_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(tube_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(tube_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(tube_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// triangle
void GLContainer::draw_triangle(glm::vec3 point1, glm::vec3 point2,
                                glm::vec3 point3, float thickness,
                                glm::vec4 color, bool draw, int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  swap_blocks();
  glUseProgram(triangle_compute);

  glUniform1i(glGetUniformLocation(triangle_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(triangle_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(triangle_compute, "color"), 1,
               glm::value_ptr(color));

  glUniform1fv(glGetUniformLocation(triangle_compute, "thickness"), 1,
               &thickness);
  glUniform3fv(glGetUniformLocation(triangle_compute, "point1"), 1,
               glm::value_ptr(point1));
  glUniform3fv(glGetUniformLocation(triangle_compute, "point2"), 1,
               glm::value_ptr(point2));
  glUniform3fv(glGetUniformLocation(triangle_compute, "point3"), 1,
               glm::value_ptr(point3));

  glUniform1i(glGetUniformLocation(triangle_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(triangle_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(triangle_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(triangle_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// clear all
void GLContainer::clear_all(bool respect_mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  swap_blocks();
  glUseProgram(clear_all_compute);

  glUniform1i(glGetUniformLocation(clear_all_compute, "respect_mask"),
              respect_mask);

  glUniform1i(glGetUniformLocation(clear_all_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(clear_all_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(clear_all_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(clear_all_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// unmask all
void GLContainer::unmask_all() {
  // don't need to redraw
  swap_blocks();
  glUseProgram(unmask_all_compute);

  glUniform1i(glGetUniformLocation(unmask_all_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(unmask_all_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(unmask_all_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(unmask_all_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// invert mask
void GLContainer::invert_mask() {
  // don't need to redraw
  swap_blocks();
  glUseProgram(invert_mask_compute);

  glUniform1i(glGetUniformLocation(invert_mask_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(invert_mask_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(invert_mask_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(invert_mask_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// mask by color
void GLContainer::mask_by_color(bool r, bool g, bool b, bool a, bool l,
                                glm::vec4 color, float l_val, float r_var,
                                float g_var, float b_var, float a_var,
                                float l_var) {
  // don't need to redraw
  swap_blocks();
  glUseProgram(mask_by_color_compute);

  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_r"), r);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_g"), g);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_b"), b);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_a"), a);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_l"), l);

  glUniform4fv(glGetUniformLocation(mask_by_color_compute, "color"), 1,
               glm::value_ptr(color));
  glUniform1f(glGetUniformLocation(mask_by_color_compute, "l_val"), l_val);

  glUniform1f(glGetUniformLocation(mask_by_color_compute, "r_var"), r_var);
  glUniform1f(glGetUniformLocation(mask_by_color_compute, "g_var"), g_var);
  glUniform1f(glGetUniformLocation(mask_by_color_compute, "b_var"), b_var);
  glUniform1f(glGetUniformLocation(mask_by_color_compute, "a_var"), a_var);
  glUniform1f(glGetUniformLocation(mask_by_color_compute, "l_var"), l_var);

  glUniform1i(glGetUniformLocation(mask_by_color_compute, "lighting"), 6);

  glUniform1i(glGetUniformLocation(mask_by_color_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(mask_by_color_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// box blur
void GLContainer::box_blur(int radius, bool touch_alpha, bool respect_mask) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();
  glUseProgram(box_blur_compute);

  glUniform1i(glGetUniformLocation(box_blur_compute, "radius"), radius);
  glUniform1i(glGetUniformLocation(box_blur_compute, "respect_mask"),
              respect_mask);
  glUniform1i(glGetUniformLocation(box_blur_compute, "touch_alpha"),
              touch_alpha);

  glUniform1i(glGetUniformLocation(box_blur_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(box_blur_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(box_blur_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(box_blur_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// gaussian blur
void GLContainer::gaussian_blur(int radius, bool touch_alpha,
                                bool respect_mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  // I think I'm going to restrict the range of radii, since I'm not sure about
  // what the best way to do different sized kernels is
  swap_blocks();
  glUseProgram(gaussian_blur_compute);

  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "radius"), radius);
  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "respect_mask"),
              respect_mask);
  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "touch_alpha"),
              touch_alpha);

  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(gaussian_blur_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// limiter
void GLContainer::limiter() {
  redraw_flag = true;
  color_mipmap_flag = true;

  // the details of this operation still need to be worked out - there is a
  // couple of different modes
}

// shifting
void GLContainer::shift(glm::ivec3 movement, bool loop, int mode) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(shift_compute);

  glUniform1i(glGetUniformLocation(shift_compute, "loop"), loop);
  glUniform1i(glGetUniformLocation(shift_compute, "mode"), mode);
  glUniform3i(glGetUniformLocation(shift_compute, "movement"), movement.x,
              movement.y, movement.z);

  // glUniform1i(glGetUniformLocation(shift_compute, "lighting"), 6);

  glUniform1i(glGetUniformLocation(shift_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(shift_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(shift_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(shift_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

std::string GLContainer::compile_user_script(std::string text) {

  std::ifstream header_file{
      "resources/engine_code/shaders/user_script_header.h.glsl"};
  std::ifstream body_file{
      "resources/engine_code/shaders/user_script_body.cs.glsl"};

#define ibitr std::istreambuf_iterator

  std::string header{ibitr<char>(header_file), ibitr<char>()};
  std::string body{ibitr<char>(body_file), ibitr<char>()};

  // header has primitives + operators
  // text has user supplied is_inside()
  // body is the assignment logic that deals with masking

  std::string program_string = header + text + body;

  // assign shader handle to the user_compute
  auto shader = UShader(program_string);
  user_compute = shader.Program;

  cout << program_string << endl;

  // report timing, compilation status
  return std::string(shader.report);
}

std::string GLContainer::run_user_script() {
  std::stringstream report;
  auto t1 = std::chrono::high_resolution_clock::now();

  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();

  glUseProgram(user_compute);
  glUniform1i(glGetUniformLocation(user_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(user_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(user_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(user_compute, "previous_mask"),
              5 - tex_offset);

  glUniform1i(glGetUniformLocation(user_compute, "lighting"), 6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  auto t2 = std::chrono::high_resolution_clock::now();
  float time_microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  report << "Shader invocation complete." << std::setw(4) << "  ("
         << time_microseconds / 1000. << " ms)" << endl;

  return report.str();
}

void GLContainer::copy_loadbuffer(bool respect_mask) {
  redraw_flag = true;
  color_mipmap_flag = true;
  swap_blocks();
  glUseProgram(copy_loadbuff_compute);

  glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "respect_mask"),
              respect_mask);

  glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "previous"),
              3 - tex_offset);
  glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "previous_mask"),
              5 - tex_offset);

  glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "loadbuff"), 10);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

// Brent Werness's Voxel Automata Terrain - set redraw_flag to true
std::string GLContainer::vat(float flip, std::string rule, int initmode,
                             glm::vec4 color0, glm::vec4 color1,
                             glm::vec4 color2, float lambda, float beta,
                             float mag, bool respect_mask, glm::bvec3 mins,
                             glm::bvec3 maxs) {
  redraw_flag = true;
  color_mipmap_flag = true;

  int dimension;

  // this is the easiest way to handle the dimension I think
  if (DIM == 32)
    dimension = 5;
  else if (DIM == 64)
    dimension = 6;
  else if (DIM == 128)
    dimension = 7;
  else if (DIM == 256)
    dimension = 8;
  else if (DIM == 512)
    dimension = 9;

  // check for equality with 'r' or 'i' to do random or isingRandom, else
  // interpret as a shortrule I want to add different init modes, to seed
  // multiple faces instead of just the one
  voxel_automata_terrain v(dimension, flip, rule, initmode, lambda, beta, mag,
                           mins, maxs);

  // pull out the texture data
  std::vector<unsigned char> loaded_bytes; // used the same way as load(), below

  // triple for-loop to pull the data out
  for (int x = 0; x < DIM; x++) {
    for (int y = 0; y < DIM; y++) {
      for (int z = 0; z < DIM; z++) {
        // append data with the colors specified as input
        glm::vec4 color;
        switch (v.state[x][y][z]) {
        case 0:
          color = color0;
          break; // use color0
        case 1:
          color = color1;
          break; // use color1
        case 2:
          color = color2;
          break; // use color2

        default:
          color = color0;
          break; // this shouldn't come up, but the compiler was mad
        }

        // put it in the vector as bytes
        loaded_bytes.push_back(static_cast<unsigned char>(color.x * 255));
        loaded_bytes.push_back(static_cast<unsigned char>(color.y * 255));
        loaded_bytes.push_back(static_cast<unsigned char>(color.z * 255));
        loaded_bytes.push_back(static_cast<unsigned char>(color.w * 255));

        // cout << v.state[x][y][z] << " ";
      }
      // cout << endl;
    }
    // cout << endl;
  }

  // send it
  glBindTexture(GL_TEXTURE_3D, textures[10]); // put it in the loadbuffer
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &loaded_bytes[0]);

  copy_loadbuffer(respect_mask);

  // get the rule out of v
  return v.getShortRule();
}

// load - set redraw_flag to true
void GLContainer::load(std::string filename, bool respect_mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  std::vector<unsigned char> image_loaded_bytes;
  unsigned width, height;

  unsigned error =
      lodepng::decode(image_loaded_bytes, width, height, filename.c_str());

  // report any errors
  if (error)
    std::cout << "decode error during load(\" " + filename + " \") " << error
              << ": " << lodepng_error_text(error) << std::endl;

  // put that shit in the front buffer with glTexImage3D()
  glBindTexture(GL_TEXTURE_3D, textures[10]); // put it in the loadbuffer
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &image_loaded_bytes[0]);

  copy_loadbuffer(respect_mask);

  cout << "filename on load is: " << filename << std::endl << std::endl;
}

// save
void GLContainer::save(std::string filename) {
  // don't need to redraw
  std::vector<unsigned char> image_bytes_to_save;
  unsigned width, height;

  width = DIM;
  height = DIM * DIM;

  image_bytes_to_save.resize(4 * DIM * DIM * DIM);
  filename = std::string("saves/") + filename;

  glBindTexture(GL_TEXTURE_3D, textures[2 + tex_offset]);
  glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                /* 4 * DIM * DIM * DIM, */ &image_bytes_to_save[0]);

  unsigned error =
      lodepng::encode(filename.c_str(), image_bytes_to_save, width, height);
  if (error)
    std::cout << "encode error during save(\" " + filename + " \") " << error
              << ": " << lodepng_error_text(error) << std::endl;

  cout << "filename on save is: " << filename << std::endl << std::endl;
}

// functions to generate new heightmaps
void GLContainer::generate_heightmap_diamond_square() {
  long unsigned int seed =
      std::chrono::system_clock::now().time_since_epoch().count();

  std::default_random_engine engine{seed};
  std::uniform_real_distribution<float> distribution{0, 1};

  constexpr auto size = DIM + 1;
  constexpr auto edge = size - 1;

  uint8_t map[size][size] = {{0}};
  map[0][0] = map[edge][0] = map[0][edge] = map[edge][edge] = 128;

  heightfield::diamond_square_no_wrap(
      size,
      // random
      [&engine, &distribution](float range) {
        return distribution(engine) * range;
      },
      // variance
      [](int level) -> float { return 64.0f * std::pow(0.5f, level); },
      // at
      [&map](int x, int y) -> uint8_t & { return map[y][x]; });

  std::vector<unsigned char> data;

  for (int x = 0; x < DIM; x++) {
    for (int y = 0; y < DIM; y++) {
      data.push_back(map[x][y]);
      data.push_back(map[x][y]);
      data.push_back(map[x][y]);
      data.push_back(255);
    }
  }

  // send it to the GPU
  glBindTexture(GL_TEXTURE_2D, textures[12]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &data[0]);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void GLContainer::generate_heightmap_perlin() {
  std::vector<unsigned char> data;

  PerlinNoise p;

  float xscale = 0.014f;
  float yscale = 0.04f;

  // might add more parameters at some point

  static float offset = 0;

  for (int x = 0; x < DIM; x++) {
    for (int y = 0; y < DIM; y++) {
      data.push_back(
          (unsigned char)(p.noise(x * xscale, y * yscale, offset) * 255));
      data.push_back(
          (unsigned char)(p.noise(x * xscale, y * yscale, offset) * 255));
      data.push_back(
          (unsigned char)(p.noise(x * xscale, y * yscale, offset) * 255));
      data.push_back(255);
    }
  }

  offset += 0.5;

  glBindTexture(GL_TEXTURE_2D, textures[12]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &data[0]);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void GLContainer::generate_heightmap_XOR() {
  // create the byte array
  std::vector<unsigned char> data;

  for (int x = 0; x < DIM; x++) {
    for (int y = 0; y < DIM; y++) {
      // cout << " "<< ((unsigned char)(x%256) ^ (unsigned char)(y%256));
      data.push_back((unsigned char)(x % 256) ^ (unsigned char)(y % 256));
      data.push_back((unsigned char)(x % 256) ^ (unsigned char)(y % 256));
      data.push_back((unsigned char)(x % 256) ^ (unsigned char)(y % 256));
      data.push_back(255);
    }
  }

  // send the data to the gpu
  glBindTexture(GL_TEXTURE_2D, textures[12]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &data[0]);
  glGenerateMipmap(GL_TEXTURE_2D);
}

// function to generate new block of 3d perlin noise
void GLContainer::generate_perlin_noise(float xscale = 0.014,
                                        float yscale = 0.04,
                                        float zscale = 0.014) {
  PerlinNoise p;
  std::vector<unsigned char> data;

  for (int x = 0; x < DIM; x++)
    for (int y = 0; y < DIM; y++)
      for (int z = 0; z < DIM; z++) {
        data.push_back(
            (unsigned char)(p.noise(x * xscale, y * yscale, z * zscale) * 255));
        data.push_back(
            (unsigned char)(p.noise(x * xscale, y * yscale, z * zscale) * 255));
        data.push_back(
            (unsigned char)(p.noise(x * xscale, y * yscale, z * zscale) * 255));
        data.push_back(255);
      }

  glBindTexture(GL_TEXTURE_3D, textures[11]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &data[0]);
  glGenerateMipmap(GL_TEXTURE_3D);
}

void GLContainer::draw_noise(float low_thresh, float high_thresh, bool smooth,
                             glm::vec4 color, bool draw, int mask) {
  redraw_flag = true;
  color_mipmap_flag = true;

  swap_blocks();
  glUseProgram(noise_compute);

  glUniform1i(glGetUniformLocation(noise_compute, "usmooth"), smooth);

  glUniform1i(glGetUniformLocation(noise_compute, "mask"), mask);
  glUniform1i(glGetUniformLocation(noise_compute, "draw"), draw);
  glUniform4fv(glGetUniformLocation(noise_compute, "ucolor"), 1,
               glm::value_ptr(color));

  glUniform1i(glGetUniformLocation(noise_compute, "tex"), 11);

  glUniform1f(glGetUniformLocation(noise_compute, "low_thresh"), low_thresh);
  glUniform1f(glGetUniformLocation(noise_compute, "high_thresh"), high_thresh);

  glUniform1i(glGetUniformLocation(noise_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(noise_compute, "current_mask"),
              4 + tex_offset);

  glUniform1i(glGetUniformLocation(noise_compute, "previous"), 3 - tex_offset);
  glUniform1i(glGetUniformLocation(noise_compute, "previous_mask"),
              5 - tex_offset);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
