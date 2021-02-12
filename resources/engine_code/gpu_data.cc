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
    // cout << "redrawing" << endl;
    auto t1 = std::chrono::high_resolution_clock::now();

    // regen mipmap if needed

    static GLuint display_compute_shader;
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
  display_shader_program = Shader("resources/engine_code/shaders/blit.vs.glsl",
                                  "resources/engine_code/shaders/blit.fs.glsl")
                               .Program;
  owidget_shader_program =
      Shader("resources/engine_code/shaders/widget.vs.glsl",
             "resources/engine_code/shaders/widget.fs.glsl")
          .Program;
}

void GLContainer::buffer_geometry() {
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

  offset = glm::vec3(POS, 0, 0);
  glm::vec3 ax = a + offset;
  ax.x *= length;
  ax.y *= factor;
  ax.z *= factor;
  glm::vec3 bx = b + offset;
  bx.x *= length;
  bx.y *= factor;
  bx.z *= factor;
  glm::vec3 cx = c + offset;
  cx.x *= length;
  cx.y *= factor;
  cx.z *= factor;
  glm::vec3 dx = d + offset;
  dx.x *= length;
  dx.y *= factor;
  dx.z *= factor;
  glm::vec3 ex = e + offset;
  ex.x *= length;
  ex.y *= factor;
  ex.z *= factor;
  glm::vec3 fx = f + offset;
  fx.x *= length;
  fx.y *= factor;
  fx.z *= factor;
  glm::vec3 gx = g + offset;
  gx.x *= length;
  gx.y *= factor;
  gx.z *= factor;
  glm::vec3 hx = h + offset;
  hx.x *= length;
  hx.y *= factor;
  hx.z *= factor;

  offset = glm::vec3(0, POS, 0);
  glm::vec3 ay = a + offset;
  ay.x *= factor;
  ay.y *= length;
  ay.z *= factor;
  glm::vec3 by = b + offset;
  by.x *= factor;
  by.y *= length;
  by.z *= factor;
  glm::vec3 cy = c + offset;
  cy.x *= factor;
  cy.y *= length;
  cy.z *= factor;
  glm::vec3 dy = d + offset;
  dy.x *= factor;
  dy.y *= length;
  dy.z *= factor;
  glm::vec3 ey = e + offset;
  ey.x *= factor;
  ey.y *= length;
  ey.z *= factor;
  glm::vec3 fy = f + offset;
  fy.x *= factor;
  fy.y *= length;
  fy.z *= factor;
  glm::vec3 gy = g + offset;
  gy.x *= factor;
  gy.y *= length;
  gy.z *= factor;
  glm::vec3 hy = h + offset;
  hy.x *= factor;
  hy.y *= length;
  hy.z *= factor;

  offset = glm::vec3(0, 0, NEG);
  glm::vec3 az = a + offset;
  az.x *= factor;
  az.y *= factor;
  az.z *= length;
  glm::vec3 bz = b + offset;
  bz.x *= factor;
  bz.y *= factor;
  bz.z *= length;
  glm::vec3 cz = c + offset;
  cz.x *= factor;
  cz.y *= factor;
  cz.z *= length;
  glm::vec3 dz = d + offset;
  dz.x *= factor;
  dz.y *= factor;
  dz.z *= length;
  glm::vec3 ez = e + offset;
  ez.x *= factor;
  ez.y *= factor;
  ez.z *= length;
  glm::vec3 fz = f + offset;
  fz.x *= factor;
  fz.y *= factor;
  fz.z *= length;
  glm::vec3 gz = g + offset;
  gz.x *= factor;
  gz.y *= factor;
  gz.z *= length;
  glm::vec3 hz = h + offset;
  hz.x *= factor;
  hz.y *= factor;
  hz.z *= length;

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

  glm::vec3 normal;

  face(a, b, c, d, points, normals, colors, color); // face ABCD
  face(c, d, g, h, points, normals, colors, color); // face CDGH
  face(e, g, f, h, points, normals, colors, color); // face EGFH
  face(a, e, b, f, points, normals, colors, color); // face AEBF
  face(a, e, c, g, points, normals, colors, color); // face AECG
  face(b, d, f, h, points, normals, colors, color); // face BDFH
}

void GLContainer::load_textures() {

  // ------------------------
  // for v1.1, I am planning out the locations of all textures at the
  //  beginning of the project - I hope to keep a more consistent environment
  //  across all the shaders, to make it easier to understand and extend

  // see gpu_data.h for the numbered listing

  // data arrays
  std::vector<unsigned char> ucxor, light, zeroes, random;

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned char> distribution(0, 255);

  cout << "generating init xor texture.....";

  for (unsigned int x = 0; x < DIM; x++) {
    for (unsigned int y = 0; y < DIM; y++) {
      for (unsigned int z = 0; z < DIM; z++) {
        for (int i = 0; i < 4; i++) // fill r, g, b with the result of the xor
        {
          ucxor.push_back(((unsigned char)(x % 256) ^ (unsigned char)(y % 256) ^
                           (unsigned char)(z % 256)));
        }
      }
    }
  }

  cout << "....done." << endl;

  random.resize(8 * screen_height * screen_width * SSFACTOR * SSFACTOR, 64);
  light.resize(3 * DIM * DIM * DIM, 64); // fill the array with '64'
  zeroes.resize(3 * DIM * DIM * DIM, 0); // fill the array with zeroes

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
  glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // copy/paste buffer render texture - this is going to be a small rectangular
  // texture, will only be shown inside the menus
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, 512, 256, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(1, textures[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  cout << "...........done." << endl;

  cout << "color voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 4 * 2 << " bytes).......";
  // main block front color buffer - initialize with xor
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_3D, textures[2]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &ucxor[0]);
  glBindImageTexture(2, textures[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // main block back color buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_3D, textures[3]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(3, textures[3], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // sets the texture filtering to linear
  main_block_linear_filter();

  // generate a mipmap for the front RGBA buffer
  main_block_mipmap_gen();

  cout << "...........done." << endl;

  cout << "mask voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 2 << " bytes).......";

  // main block front mask buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_3D, textures[4]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED,
               GL_UNSIGNED_BYTE, NULL);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(4, textures[4], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

  // main block back mask buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_3D, textures[5]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED,
               GL_UNSIGNED_BYTE, NULL);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(5, textures[5], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

  cout << "...........done." << endl;

  cout << "light buffer voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 2 << " bytes).......";

  // display lighting buffer - initialize with some base value representing
  // neutral coloration
  glActiveTexture(GL_TEXTURE0 + 6);
  glBindTexture(GL_TEXTURE_3D, textures[6]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED,
               GL_UNSIGNED_BYTE, &light[0]);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(6, textures[6], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

  // lighting cache buffer - this is going to have the same data in it as the
  // regular lighting buffer initially
  glActiveTexture(GL_TEXTURE0 + 7);
  glBindTexture(GL_TEXTURE_3D, textures[7]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED,
               GL_UNSIGNED_BYTE, &light[0]);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(7, textures[7], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

  cout << "...........done." << endl;

  // copy/paste front buffer - initally empty
  glActiveTexture(GL_TEXTURE0 + 8);
  glBindTexture(GL_TEXTURE_3D, textures[8]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(8, textures[8], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // copy/paste back buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 9);
  glBindTexture(GL_TEXTURE_3D, textures[9]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(9, textures[9], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // load buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 10);
  glBindTexture(GL_TEXTURE_3D, textures[10]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindImageTexture(10, textures[10], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  cout << "perlin texture generation....." << std::flush;

  // perlin noise - initialize with noise at some default scaling
  glActiveTexture(GL_TEXTURE0 + 11);
  glBindTexture(GL_TEXTURE_3D, textures[11]);

  glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
  // 3d texture for perlin noise - DIM on a side
  // generate_perlin_noise(0.014, 0.04, 0.014);

  cout << ".............done." << endl;
  cout << "heightmap............";
  // heightmap - initialize with a generated diamond square heightmap
  glActiveTexture(GL_TEXTURE0 + 12);
  glBindTexture(GL_TEXTURE_2D, textures[12]);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  // 2d texture for representation of a heightmap (greyscale - use some channels
  // to hold more data?) - also, DIM on a side
  // generate_heightmap_diamond_square();
  cout << "........done." << endl;
}
