#include "gpu_data.h"
#include "includes.h"

void GLContainer::log(std::string text) {
  cout << " issued \"" << text << "\" at " << current_time_and_date() << endl;
  operations.push_back(text);
}

void GLContainer::clear_log() { operations.clear(); }

void GLContainer::save_log(std::string filename) {
  json j;

  j["num_operations"] = operations.size();
  for (unsigned int i = 0; i < operations.size(); i++) {
    j[std::to_string(i)] = operations[i];
  }
  // open the file and dump it
  std::ofstream file("logs/" + filename);
  file << j.dump(1);
}

// void GLContainer::load_log(std::string filename) { // load and execute log
// load the file as a string
// make json out of string
// pass to the animation worker class
// }

// void GLContainer::run_list(json j) {
// j contains a list of operations to execute
// }

float GLContainer::parse_and_execute_JSON_op(json j) {
  auto t1 = std::chrono::high_resolution_clock::now();

  if (j["type"] == std::string("rotate_vertical")) {
    rotate_vertical(j["amount"]);
  } else if (j["type"] == std::string("rotate_horizontal")) {
    rotate_horizontal(j["amount"]);
  } else if (j["type"] == std::string("rollate")) {
    rolltate(j["amount"]);
  } else if (j["type"] == std::string("init_basis")) {
    init_basis();
  } else if (j["type"] == std::string("view_front")) {
    view_front();
  } else if (j["type"] == std::string("view_back")) {
    view_back();
  } else if (j["type"] == std::string("view_right")) {
    view_right();
  } else if (j["type"] == std::string("view_left")) {
    view_left();
  } else if (j["type"] == std::string("view_up")) {
    view_up();
  } else if (j["type"] == std::string("view_down")) {
    view_down();
  } else if (j["type"] == std::string("clickndrag_adjust")) {
    clickndragx += float(j["x"]);
    clickndragy += float(j["y"]);
  } else if (j["type"] == std::string("scale_adjust")) {
    scale += float(j["amount"]);
  } else if (j["type"] == std::string("nearest_filter")) {
    main_block_nearest_filter();
  } else if (j["type"] == std::string("linear_filter")) {
    main_block_linear_filter();
  } else if (j["type"] == std::string("lighting_clear")) {
    lighting_clear(j["use_cache"],
                   glm::vec4(j["clear_level"]["r"], j["clear_level"]["g"],
                             j["clear_level"]["b"], j["clear_level"]["a"]));
  } else if (j["type"] == std::string("point_lighting")) {
    compute_point_lighting(glm::vec3(j["light_position"]["x"],
                                     j["light_position"]["y"],
                                     j["light_position"]["z"]),
                           glm::vec4(j["color"]["r"], j["color"]["g"],
                                     j["color"]["b"], j["color"]["a"]),
                           j["decay_power"], j["distance_power"]);
  } else if (j["type"] == std::string("cone_lighting")) {
    compute_cone_lighting(
        glm::vec3(j["location"]["x"], j["location"]["y"], j["location"]["z"]),
        j["theta"], j["phi"], j["cone_angle"],
        glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                  j["color"]["a"]),
        j["decay_power"], j["distance_power"]);
  } else if (j["type"] == std::string("directional_lighting")) {
    compute_new_directional_lighting(j["theta"], j["phi"],
                                     glm::vec4(j["color"]["r"], j["color"]["g"],
                                               j["color"]["b"],
                                               j["color"]["a"]),
                                     j["decay_power"]);
  } else if (j["type"] == std::string("fake_GI")) {
    compute_fake_GI(j["factor"],
                    glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                              j["color"]["a"]),
                    j["threshold"]);
  } else if (j["type"] == std::string("ambient_occlusion")) {
    compute_ambient_occlusion(j["radius"]);
  } else if (j["type"] == std::string("mash")) {
    mash();
  } else if (j["type"] == std::string("draw_aabb")) {
    draw_aabb(glm::vec3(j["min"]["x"], j["min"]["y"], j["min"]["z"]),
              glm::vec3(j["max"]["x"], j["max"]["y"], j["max"]["z"]),
              glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                        j["color"]["a"]),
              j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_cuboid")) {
    draw_cuboid(glm::vec3(j["a"]["x"], j["a"]["y"], j["a"]["z"]),
                glm::vec3(j["b"]["x"], j["b"]["y"], j["b"]["z"]),
                glm::vec3(j["c"]["x"], j["c"]["y"], j["c"]["z"]),
                glm::vec3(j["d"]["x"], j["d"]["y"], j["d"]["z"]),
                glm::vec3(j["e"]["x"], j["e"]["y"], j["e"]["z"]),
                glm::vec3(j["f"]["x"], j["f"]["y"], j["f"]["z"]),
                glm::vec3(j["g"]["x"], j["g"]["y"], j["g"]["z"]),
                glm::vec3(j["h"]["x"], j["h"]["y"], j["h"]["z"]),
                glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                          j["color"]["a"]),
                j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_cylinder")) {
    draw_cylinder(glm::vec3(j["tvec"]["x"], j["tvec"]["y"], j["tvec"]["z"]),
                  glm::vec3(j["bvec"]["x"], j["bvec"]["y"], j["bvec"]["z"]),
                  j["radius"],
                  glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                            j["color"]["a"]),
                  j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_ellipsoid")) {
    draw_ellipsoid(
        glm::vec3(j["center"]["x"], j["center"]["y"], j["center"]["z"]),
        glm::vec3(j["radii"]["x"], j["radii"]["y"], j["radii"]["z"]),
        glm::vec3(j["rotation"]["x"], j["rotation"]["y"], j["rotation"]["z"]),
        glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                  j["color"]["a"]),
        j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_grid")) {
    draw_grid(
        glm::ivec3(j["spacing"]["x"], j["spacing"]["y"], j["spacing"]["z"]),
        glm::ivec3(j["width"]["x"], j["width"]["y"], j["width"]["z"]),
        glm::ivec3(j["offsets"]["x"], j["offsets"]["y"], j["offsets"]["z"]),
        glm::vec3(j["rotation"]["x"], j["rotation"]["y"], j["rotation"]["z"]),
        glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                  j["color"]["a"]),
        j["draw"], j["mask"]);
  } else if (j["type"] == std::string("generate_heightmap_xor")) {
    generate_heightmap_XOR();
  } else if (j["type"] == std::string("generate_heightmap_perlin")) {
    generate_heightmap_perlin();
  } else if (j["type"] == std::string("generate_heightmap_diamond_square")) {
    generate_heightmap_diamond_square();
  } else if (j["type"] == std::string("draw_heightmap")) {
    draw_heightmap(j["height_scale"], j["height_color"],
                   glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                             j["color"]["a"]),
                   j["draw"], j["mask"]);
  } else if (j["type"] == std::string("generate_perlin_noise")) {
    generate_perlin_noise(j["xscale"], j["yscale"], j["zscale"], j["seed"]);
  } else if (j["type"] == std::string("gen_noise")) {
    gen_noise(j["preset"], j["seed"]);
  } else if (j["type"] == std::string("draw_noise")) {
    draw_noise(j["low_thresh"], j["high_thresh"], j["smooth"],
               glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                         j["color"]["a"]),
               j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_regular_icosahedron")) {
    draw_regular_icosahedron(
        j["x_rot"], j["y_rot"], j["z_rot"], j["scale"],
        glm::vec3(j["center_point"]["x"], j["center_point"]["y"],
                  j["center_point"]["z"]),
        glm::vec4(j["vertex_color"]["r"], j["vertex_color"]["g"],
                  j["vertex_color"]["b"], j["vertex_color"]["a"]),
        j["vertex_radius"],
        glm::vec4(j["edge_color"]["r"], j["edge_color"]["g"],
                  j["edge_color"]["b"], j["edge_color"]["a"]),
        j["edge_thickness"],
        glm::vec4(j["face_color"]["r"], j["face_color"]["g"],
                  j["face_color"]["b"], j["face_color"]["a"]),
        j["face_thickness"], j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_sphere")) {
    draw_sphere(
        glm::vec3(j["location"]["x"], j["location"]["y"], j["location"]["z"]),
        j["radius"],
        glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                  j["color"]["a"]),
        j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_tube")) {
    draw_tube(glm::vec3(j["bvec"]["x"], j["bvec"]["y"], j["bvec"]["z"]),
              glm::vec3(j["tvec"]["x"], j["tvec"]["y"], j["tvec"]["z"]),
              j["inner_radius"], j["outer_radius"],
              glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                        j["color"]["a"]),
              j["draw"], j["mask"]);
  } else if (j["type"] == std::string("draw_triangle")) {
    draw_triangle(
        glm::vec3(j["point1"]["x"], j["point1"]["y"], j["point1"]["z"]),
        glm::vec3(j["point2"]["x"], j["point2"]["y"], j["point2"]["z"]),
        glm::vec3(j["point3"]["x"], j["point3"]["y"], j["point3"]["z"]),
        j["thickness"],
        glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                  j["color"]["a"]),
        j["draw"], j["mask"]);
  } else if (j["type"] == std::string("invert_mask")) {
    invert_mask();
  } else if (j["type"] == std::string("unmask_all")) {
    unmask_all();
  } else if (j["type"] == std::string("mask_by_color")) {
    mask_by_color(j["r"], j["g"], j["b"], j["a"], j["l"],
                  glm::vec4(j["color"]["r"], j["color"]["g"], j["color"]["b"],
                            j["color"]["a"]),
                  j["l_val"], j["r_var"], j["g_var"], j["b_var"], j["a_var"],
                  j["l_var"], j["amount"]);
  } else if (j["type"] == std::string("box_blur")) {
    box_blur(j["radius"], j["touch_alpha"], j["respect_mask"]);
  } else if (j["type"] == std::string("gaussian_blur")) {
    gaussian_blur(j["radius"], j["touch_alpha"], j["respect_mask"]);
  } else if (j["type"] == std::string("limiter")) {
    // float limiter();
  } else if (j["type"] == std::string("shift")) {
    shift(
        glm::ivec3(j["movement"]["x"], j["movement"]["y"], j["movement"]["z"]),
        j["loop"], j["mode"]);
  } else if (j["type"] == std::string("compile_user_script")) {
    compile_user_script(j["text"]);
  } else if (j["type"] == std::string("run_user_script")) {
    run_user_script();
  } else if (j["type"] == std::string("load")) {
    load(j["filename"], j["respect_mask"]);
  } else if (j["type"] == std::string("save")) {
    save(j["filename"]);
  } else if (j["type"] == std::string("vat")) {
    vat(j["flip"], j["rule"], j["init_mode"],
        glm::vec4(j["color0"]["r"], j["color0"]["g"], j["color0"]["b"],
                  j["color0"]["a"]),
        glm::vec4(j["color1"]["r"], j["color1"]["g"], j["color1"]["b"],
                  j["color1"]["a"]),
        glm::vec4(j["color2"]["r"], j["color2"]["g"], j["color2"]["b"],
                  j["color2"]["a"]),
        j["lambda"], j["beta"], j["mag"], j["respect_mask"],
        glm::bvec3(j["mins"]["x"], j["mins"]["y"], j["mins"]["z"]),
        glm::bvec3(j["maxs"]["x"], j["maxs"]["y"], j["maxs"]["z"]));
  }

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

void GLContainer::dither_bayer()
{
	// from https://www.anisopteragames.com/how-to-fix-color-banding-with-dithering/
	static const char pattern[] = {
  0, 32,  8, 40,  2, 34, 10, 42,   /* 8x8 Bayer ordered dithering  */
  48, 16, 56, 24, 50, 18, 58, 26,  /* pattern.  Each input pixel   */
  12, 44,  4, 36, 14, 46,  6, 38,  /* is scaled to the 0..63 range */
  60, 28, 52, 20, 62, 30, 54, 22,  /* before looking in this table */
  3, 35, 11, 43,  1, 33,  9, 41,   /* to determine the action.     */
  51, 19, 59, 27, 49, 17, 57, 25,
  15, 47,  7, 39, 13, 45,  5, 37,
  63, 31, 55, 23, 61, 29, 53, 21 };

  ditherdim = 8;

  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, textures[1]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ditherdim, ditherdim, 0, GL_RED,
               GL_UNSIGNED_BYTE, pattern);
}

void GLContainer::dither_blue()
{
	// generate 64x64 blue noise texture from the header

	ditherdim = 64;

	// send it GPU-wards
}

void GLContainer::screenshot(std::string filename) {
  // already accounts for TRIPLE_MONITOR
  unsigned width = screen_width;

  // height is the same either way
  unsigned height = screen_height;

  // declare buffer and allocate
  std::vector<unsigned char> image_bytes_to_save, temp;
  image_bytes_to_save.resize(width * height * 3);

  // glReadnPixels(...) - this is supposed to be slow, note that this needs to
  // consider TRIPLE_MONITOR define to output a wide image
  // using the version with the n so you can use a user-provided buffer and
  // saving that directly to keep from having to copy glReadnPixels( 0, 0,
  // width, height, GL_RGB, GL_UNSIGNED_BYTE, width * height * 3,
  // &image_bytes_to_save[0]);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
               &image_bytes_to_save[0]);

  temp.assign(image_bytes_to_save.begin(), image_bytes_to_save.end());

  // reorder flipped image content
  for (int x = 0; x < (int)width; x++)
    for (int y = 0; y < (int)height; y++) {
      int input_base = 3 * ((height - y - 1) * width + x);
      int output_base = 3 * (y * width + x);
      image_bytes_to_save[output_base + 0] = temp[input_base + 0];
      image_bytes_to_save[output_base + 1] = temp[input_base + 1];
      image_bytes_to_save[output_base + 2] = temp[input_base + 2];
    }

  // save the resulting image - using the same buffer makes it so you don't have
  // to copy it
  unsigned error = lodepng::encode(filename.c_str(), image_bytes_to_save, width,
                                   height, LCT_RGB, 8);

  if (error) {
    std::cout << "encode error during save(\" " + filename + " \") " << error
              << ": " << lodepng_error_text(error) << std::endl;
  }
}

void GLContainer::single_screenshot() {
  // formatted date and time
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t),
                      "Voraldo_1_2_Screenshot-%Y-%m-%d %X")
     << ".png";
  screenshot(ss.str());
}

void GLContainer::animation(std::string filename) {
  // load the json
  std::ifstream fin(filename);
  json j;
  fin >> j;

  // get number of frames, number of setup operations
  int num_frames = j["num_frames"];
  int num_setup_ops = j["setup"]["num_ops"];

  // run all the setup operations
  for (int i = 0; i < num_setup_ops; i++) {
    parse_and_execute_JSON_op(json(j["setup"]["op" + std::to_string(i)]));
  }

  // for all frames n
  for (int n = 0; n < num_frames; n++) {
    //  run the frame n operations
    for (int i = 0; i < j["frame" + std::to_string(n)]["num_ops"]; i++) {
      parse_and_execute_JSON_op(
          json(j["frame" + std::to_string(n)]["op" + std::to_string(i)]));
    }

    //  display the block
    display_block();

    //  save the screenshot for frame n
    std::stringstream ss;
    ss << "frames/step" << std::setfill('0') << std::setw(5) << n << ".png";
    screenshot(ss.str());
  }
}

float GLContainer::init_basis() {
  auto t1 = std::chrono::high_resolution_clock::now();
  redraw_flag = true;
  basisx = glm::vec3(-1., 0., 0.);
  basisy = glm::vec3(0., -1., 0.);
  basisz = glm::vec3(0., 0., 1.);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// apply the rotation to the basis vectors about the x and y axes
float GLContainer::rotate_vertical(float amnt) {
  auto t1 = std::chrono::high_resolution_clock::now();
  basisx = glm::rotate(basisx, amnt, glm::vec3(1, 0, 0));
  basisy = glm::rotate(basisy, amnt, glm::vec3(1, 0, 0));
  basisz = glm::rotate(basisz, amnt, glm::vec3(1, 0, 0));
  redraw_flag = true;
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::rotate_horizontal(float amnt) {
  auto t1 = std::chrono::high_resolution_clock::now();
  basisx = glm::rotate(basisx, amnt, glm::vec3(0, 1, 0));
  basisy = glm::rotate(basisy, amnt, glm::vec3(0, 1, 0));
  basisz = glm::rotate(basisz, amnt, glm::vec3(0, 1, 0));
  redraw_flag = true;
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::rolltate(float amnt) {
  auto t1 = std::chrono::high_resolution_clock::now();
  basisx = glm::rotate(basisx, amnt, glm::vec3(0, 0, 1));
  basisy = glm::rotate(basisy, amnt, glm::vec3(0, 0, 1));
  basisz = glm::rotate(basisz, amnt, glm::vec3(0, 0, 1));
  redraw_flag = true;
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::main_block_image() {
  auto t1 = std::chrono::high_resolution_clock::now();
  redraw_flag = true;

  display_compute_shader = display_compute_image;

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::main_block_depth() {
  auto t1 = std::chrono::high_resolution_clock::now();
  main_block_linear_filter(); // filtering
  redraw_flag = true;

  display_compute_shader = display_compute_depth;

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::main_block_position() {
  auto t1 = std::chrono::high_resolution_clock::now();
  main_block_linear_filter(); // filtering
  redraw_flag = true;

  display_compute_shader = display_compute_position;

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::main_block_linear_filter() {
  auto t1 = std::chrono::high_resolution_clock::now();
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

  display_compute_shader = display_compute_sampler;

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// set nearest filtering
float GLContainer::main_block_nearest_filter() {
  auto t1 = std::chrono::high_resolution_clock::now();
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

  display_compute_shader = display_compute_sampler;

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

void GLContainer::color_mipmap_gen() {

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

void GLContainer::light_mipmap_gen() {
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
  static int temp_tonemapping_mode = 0;
  static glm::vec4 temp_clear_color;

  if ((temp_scale != scale) || (temp_clickndragx != clickndragx) ||
      (temp_clickndragy != clickndragy) || (acp != alpha_correction_power) ||
      (clear_color != temp_clear_color) || (tonemap_mode != temp_tonemapping_mode))
    redraw_flag = true;

  temp_scale = scale;
  temp_clickndragx = clickndragx;
  temp_clickndragy = clickndragy;
  acp = alpha_correction_power;
  temp_clear_color = clear_color;
  temp_tonemapping_mode = tonemap_mode;

  if (redraw_flag) {

    auto t1 = std::chrono::high_resolution_clock::now();

    // regen mipmaps if needed
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

    // do the tile based rendering using the raycast compute shader
    glUseProgram(display_compute_shader);

    // display texture
    glUniform1i(glGetUniformLocation(display_compute_shader, "current"), 0);
    glUniform1i(glGetUniformLocation(display_compute_shader, "block"), 2 + tex_offset);

    // lighting
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
    glUniform1f(glGetUniformLocation(display_compute_shader, "clickndragx"),
                clickndragx);
    glUniform1f(glGetUniformLocation(display_compute_shader, "clickndragy"),
                clickndragy);

    // clear color
    glUniform4fv(glGetUniformLocation(display_compute_shader, "clear_color"), 1,
                 glm::value_ptr(clear_color));

    // alpha power
    glUniform1f(glGetUniformLocation(display_compute_shader, "upow"),
                alpha_correction_power);

    // tonemapping setting
    glUniform1i(glGetUniformLocation(display_compute_shader, "ACES_behavior"),
                tonemap_mode);

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
         << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
         << " microseconds" << endl;

    redraw_flag = false; // no need to draw anything again, till something changes
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

  // gamma correction
  glUniform1f(glGetUniformLocation(display_shader_program, "gamma"),
              gamma_correction);

  // dithering
  glUniform1i(glGetUniformLocation(display_shader_program, "dither"), 1);
  glUniform1i(glGetUniformLocation(display_shader_program, "ditherdim"), ditherdim);

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

  // raycasting, but with samplers
  display_compute_sampler =
      CShader("resources/engine_code/shaders/raycast_sampler.cs.glsl").Program;

  // raycasting with depth visualization
  display_compute_depth =
      CShader("resources/engine_code/shaders/raycast_sampler_depth.cs.glsl")
          .Program;
  // raycasting with position visualization
  display_compute_position =
      CShader("resources/engine_code/shaders/raycast_sampler_position.cs.glsl")
          .Program;

  // lighting functions
  lighting_clear_compute =
      CShader("resources/engine_code/shaders/light_clear.cs.glsl").Program;
  point_lighting_compute =
      CShader("resources/engine_code/shaders/point_light.cs.glsl").Program;
  directional_lighting_compute =
      CShader("resources/engine_code/shaders/directional_light.cs.glsl")
          .Program;
  fakeGI_compute =
      CShader("resources/engine_code/shaders/fakeGI.cs.glsl").Program;
  ambient_occlusion_compute =
      CShader("resources/engine_code/shaders/ambient_occlusion.cs.glsl")
          .Program;
  mash_compute = CShader("resources/engine_code/shaders/mash.cs.glsl").Program;

  // utility functions
  copy_loadbuff_compute =
      CShader("resources/engine_code/shaders/copy_loadbuff.cs.glsl").Program;
  clear_all_compute =
      CShader("resources/engine_code/shaders/clear_all.cs.glsl").Program;
  unmask_all_compute =
      CShader("resources/engine_code/shaders/unmask_all.cs.glsl").Program;
  invert_mask_compute =
      CShader("resources/engine_code/shaders/invert_mask.cs.glsl").Program;
  mask_by_color_compute =
      CShader("resources/engine_code/shaders/mask_by_color.cs.glsl").Program;
  box_blur_compute =
      CShader("resources/engine_code/shaders/box_blur.cs.glsl").Program;
  gaussian_blur_compute =
      CShader("resources/engine_code/shaders/gauss_blur.cs.glsl").Program;
  shift_compute =
      CShader("resources/engine_code/shaders/shift.cs.glsl").Program;

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
  noise_compute =
      CShader("resources/engine_code/shaders/noise.cs.glsl").Program;
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
  std::vector<unsigned char> ucxor, zeroes, random;
  std::vector<GLfloat> light;

  std::default_random_engine generator;
  std::uniform_int_distribution<unsigned char> distribution(0, 255);

  light.resize(4 * DIM * DIM * DIM, 1.0); // fill the array with 1.0
  zeroes.resize(4 * DIM * DIM * DIM, 0);  // fill the array with zeroes

  cout << "generating init xor texture.....";
  PerlinNoise p;

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
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16F, screen_width * SSFACTOR,
               screen_height * SSFACTOR, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               &zeroes[0]);
  glBindImageTexture(
      0, textures[0], 0, GL_TRUE, 0, GL_READ_WRITE,
      GL_RGBA16F); // 16 bits, hopefully higher precision is helpful

  // this is now the dither pattern shader
  // glActiveTexture(GL_TEXTURE0 + 1);
  // glBindTexture(GL_TEXTURE_2D, textures[1]);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 256, 0, GL_RGBA,
               // GL_UNSIGNED_BYTE, &zeroes[0]);
  // glBindImageTexture(1, textures[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  dither_bayer();

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
               GL_UNSIGNED_BYTE, &zeroes[0]);
  glBindImageTexture(3, textures[3], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  // options are blue noise and the bayer matrix, initialize with bayer or blue noise
  dither_bayer();
  // dither_blue();


  cout << "...........done." << endl;

  cout << "mask voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 2 << " bytes).......";

  // main block front mask buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_3D, textures[4]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, DIM, DIM, DIM, 0, GL_RED_INTEGER,
               GL_UNSIGNED_BYTE, &zeroes[0]);
  glBindImageTexture(4, textures[4], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);

  // main block back mask buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_3D, textures[5]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, DIM, DIM, DIM, 0, GL_RED_INTEGER,
               GL_UNSIGNED_BYTE, NULL);
  glBindImageTexture(5, textures[5], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);

  cout << "...........done." << endl;

  cout << "light buffer voxel blocks at " << DIM << " resolution ("
       << DIM * DIM * DIM * 3 * 2 << " bytes).......";

  // display lighting buffer - initialize with some base value representing
  // neutral coloration
  glActiveTexture(GL_TEXTURE0 + 6);
  glBindTexture(GL_TEXTURE_3D, textures[6]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, DIM, DIM, DIM, 0, GL_RGBA,
               GL_FLOAT, &light[0]);
  glBindImageTexture(6, textures[6], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  // lighting cache buffer - this is going to have the same data in it as the
  // regular lighting buffer initially
  glActiveTexture(GL_TEXTURE0 + 7);
  glBindTexture(GL_TEXTURE_3D, textures[7]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, DIM, DIM, DIM, 0, GL_RGBA,
               GL_FLOAT, &light[0]);
  glBindImageTexture(7, textures[7], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

  cout << "...........done." << endl;

  // copy/paste front buffer - initally empty
  glActiveTexture(GL_TEXTURE0 + 8);
  glBindTexture(GL_TEXTURE_3D, textures[8]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &zeroes[0]);
  glBindImageTexture(8, textures[8], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // copy/paste back buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 9);
  glBindTexture(GL_TEXTURE_3D, textures[9]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &zeroes[0]);
  glBindImageTexture(9, textures[9], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

  // load buffer - initially empty
  glActiveTexture(GL_TEXTURE0 + 10);
  glBindTexture(GL_TEXTURE_3D, textures[10]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &zeroes[0]);
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
  cout << generate_perlin_noise(0.014, 0.04, 0.014, 0) << " us" << endl << endl;
  cout << gen_noise(0,0) << " us" << endl << endl;

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

// view rotation functions
float GLContainer::view_front() {
  auto t1 = std::chrono::high_resolution_clock::now();
  init_basis();
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::view_back() {
  auto t1 = std::chrono::high_resolution_clock::now();
  init_basis();
  rotate_horizontal(pi);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::view_right() {
  auto t1 = std::chrono::high_resolution_clock::now();
  init_basis();
  rotate_horizontal(pi / 2.);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::view_left() {
  auto t1 = std::chrono::high_resolution_clock::now();
  init_basis();
  rotate_horizontal(-pi / 2.);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::view_up() {
  auto t1 = std::chrono::high_resolution_clock::now();
  init_basis();
  rotate_vertical(-pi / 2.);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::view_down() {
  auto t1 = std::chrono::high_resolution_clock::now();
  init_basis();
  rotate_vertical(pi / 2.);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// lighting functions
float GLContainer::lighting_clear(bool use_cache, glm::vec4 clear_level) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "lighting_clear";
  j["clear_level"]["r"] = clear_level.r;
  j["clear_level"]["g"] = clear_level.g;
  j["clear_level"]["b"] = clear_level.b;
  j["clear_level"]["a"] = clear_level.a;
  j["use_cache"] = use_cache;
  log(j.dump());

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

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::compute_point_lighting(glm::vec3 light_position,
                                          glm::vec4 color, float decay_power,
                                          float distance_power) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "point_lighting";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["light_position"]["x"] = light_position.x;
  j["light_position"]["y"] = light_position.y;
  j["light_position"]["z"] = light_position.z;
  j["decay_power"] = decay_power;
  j["distance_power"] = distance_power;
  log(j.dump());

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
              decay_power);
  glUniform1f(glGetUniformLocation(point_lighting_compute, "distance_power"),
              distance_power);
  glUniform4f(glGetUniformLocation(point_lighting_compute, "light_intensity"),
              color.x, color.y, color.z, color[3]);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::compute_cone_lighting(glm::vec3 location, float theta,
                                         float phi, float cone_angle,
                                         glm::vec4 color, float decay_power,
                                         float distance_power) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "cone_lighting";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["theta"] = theta;
  j["phi"] = phi;
  j["cone_angle"] = cone_angle;
  j["location"]["x"] = location.x;
  j["location"]["y"] = location.y;
  j["location"]["z"] = location.z;
  j["decay_power"] = decay_power;
  j["distance_power"] = distance_power;
  log(j.dump());

  redraw_flag = true;
  light_mipmap_flag = true;
  glUseProgram(cone_lighting_compute);

  glUniform3fv(glGetUniformLocation(cone_lighting_compute, "light_position"), 1,
               glm::value_ptr(location));

  glUniform1f(glGetUniformLocation(cone_lighting_compute, "theta"), theta);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "phi"), phi);

  glUniform1f(glGetUniformLocation(cone_lighting_compute, "cone_angle"),
              cone_angle);
  glUniform4f(glGetUniformLocation(cone_lighting_compute, "light_intensity"),
              color.r, color.g, color.b, color.a);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "decay_power"),
              decay_power);
  glUniform1f(glGetUniformLocation(cone_lighting_compute, "distance_power"),
              distance_power);

  glUniform1i(glGetUniformLocation(cone_lighting_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(cone_lighting_compute, "lighting"), 6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::compute_new_directional_lighting(float theta, float phi,
                                                    glm::vec4 color,
                                                    float decay_power) {

  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "directional_lighting";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["decay_power"] = decay_power;
  j["theta"] = theta;
  j["phi"] = phi;
  log(j.dump());

  redraw_flag = true;
  light_mipmap_flag = true;

  glUseProgram(directional_lighting_compute);

  glUniform1f(glGetUniformLocation(directional_lighting_compute, "utheta"),
              theta);
  glUniform1f(glGetUniformLocation(directional_lighting_compute, "uphi"), phi);
  glUniform4f(
      glGetUniformLocation(directional_lighting_compute, "light_intensity"),
      color.r, color.g, color.b, color.a);
  glUniform1f(glGetUniformLocation(directional_lighting_compute, "decay_power"),
              decay_power);

  glUniform1i(glGetUniformLocation(directional_lighting_compute, "current"),
              2 + tex_offset);
  glUniform1i(glGetUniformLocation(directional_lighting_compute, "lighting"),
              6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8); // workgroup is 8x8x8
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// fake GI
float GLContainer::compute_fake_GI(float factor, glm::vec4 color,
                                   float thresh) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "fake_GI";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["threshold"] = thresh;
  j["factor"] = factor;
  log(j.dump());

  redraw_flag = true;
  light_mipmap_flag = true;

  glUseProgram(fakeGI_compute);

  glUniform1i(glGetUniformLocation(fakeGI_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(fakeGI_compute, "lighting"), 6);

  glUniform1f(glGetUniformLocation(fakeGI_compute, "scale_factor"), factor);
  glUniform1f(glGetUniformLocation(fakeGI_compute, "alpha_thresh"), thresh);
  glUniform4f(glGetUniformLocation(fakeGI_compute, "sky_intensity"), color.r,
              color.g, color.b, color.a);

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::compute_ambient_occlusion(int radius) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "ambient_occlusion";
  j["radius"] = radius;
  log(j.dump());

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

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::mash() {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "mash";
  log(j.dump());

  glUseProgram(mash_compute);

  glUniform1i(glGetUniformLocation(mash_compute, "current"), 2 + tex_offset);
  glUniform1i(glGetUniformLocation(mash_compute, "lighting"), 6);

  glDispatchCompute(DIM / 8, DIM / 8, DIM / 8);

  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// SHAPES
float GLContainer::draw_aabb(glm::vec3 min, glm::vec3 max, glm::vec4 color,
                             bool aabb_draw, int aabb_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_aabb";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["min"]["x"] = min.x;
  j["min"]["y"] = min.y;
  j["min"]["z"] = min.z;
  j["max"]["x"] = max.x;
  j["max"]["y"] = max.y;
  j["max"]["z"] = max.z;
  j["draw"] = aabb_draw;
  j["mask"] = aabb_mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::draw_cuboid(glm::vec3 a, glm::vec3 b, glm::vec3 c,
                               glm::vec3 d, glm::vec3 e, glm::vec3 f,
                               glm::vec3 g, glm::vec3 h, glm::vec4 color,
                               bool cuboid_draw, int cuboid_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_cuboid";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["a"]["x"] = a.x;
  j["a"]["y"] = a.y;
  j["a"]["z"] = a.z;
  j["b"]["x"] = b.x;
  j["b"]["y"] = b.y;
  j["b"]["z"] = b.z;
  j["c"]["x"] = c.x;
  j["c"]["y"] = c.y;
  j["c"]["z"] = c.z;
  j["d"]["x"] = d.x;
  j["d"]["y"] = d.y;
  j["d"]["z"] = d.z;
  j["e"]["x"] = e.x;
  j["e"]["y"] = e.y;
  j["e"]["z"] = e.z;
  j["f"]["x"] = f.x;
  j["f"]["y"] = f.y;
  j["f"]["z"] = f.z;
  j["g"]["x"] = g.x;
  j["g"]["y"] = g.y;
  j["g"]["z"] = g.z;
  j["h"]["x"] = h.x;
  j["h"]["y"] = h.y;
  j["h"]["z"] = h.z;
  j["draw"] = cuboid_draw;
  j["mask"] = cuboid_mask;
  log(j.dump());

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

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::draw_cylinder(glm::vec3 bvec, glm::vec3 tvec, float radius,
                                 glm::vec4 color, bool draw, int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_cylinder";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["tvec"]["x"] = tvec.x;
  j["tvec"]["y"] = tvec.y;
  j["tvec"]["z"] = tvec.z;
  j["bvec"]["x"] = bvec.x;
  j["bvec"]["y"] = bvec.y;
  j["bvec"]["z"] = bvec.z;
  j["radius"] = radius;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::draw_ellipsoid(glm::vec3 center, glm::vec3 radii,
                                  glm::vec3 rotation, glm::vec4 color,
                                  bool draw, int mask) {

  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_ellipsoid";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["center"]["x"] = center.x;
  j["center"]["y"] = center.y;
  j["center"]["z"] = center.z;
  j["radii"]["x"] = radii.x;
  j["radii"]["y"] = radii.y;
  j["radii"]["z"] = radii.z;
  j["rotation"]["x"] = radii.x;
  j["rotation"]["y"] = radii.y;
  j["rotation"]["z"] = radii.z;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::draw_grid(glm::ivec3 spacing, glm::ivec3 width,
                             glm::ivec3 offsets, glm::vec3 rot, glm::vec4 color,
                             bool draw, int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_grid";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["spacing"]["x"] = spacing.x;
  j["spacing"]["y"] = spacing.y;
  j["spacing"]["z"] = spacing.z;
  j["width"]["x"] = width.x;
  j["width"]["y"] = width.y;
  j["width"]["z"] = width.z;
  j["rotation"]["x"] = rot.x;
  j["rotation"]["y"] = rot.y;
  j["rotation"]["z"] = rot.z;
  j["offsets"]["x"] = offsets.x;
  j["offsets"]["y"] = offsets.y;
  j["offsets"]["z"] = offsets.z;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// heightmap
float GLContainer::draw_heightmap(float height_scale, bool height_color,
                                  glm::vec4 color, bool draw, int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_heightmap";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["height_scale"] = height_scale;
  j["height_color"] = height_color;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::draw_regular_icosahedron(
    double x_rot, double y_rot, double z_rot, double scale,
    glm::vec3 center_point, glm::vec4 vertex_material, double verticies_radius,
    glm::vec4 edge_material, double edge_thickness, glm::vec4 face_material,
    float face_thickness, bool draw, bool mask) {
  auto t1 = std::chrono::high_resolution_clock::now();

  { // scoped to prevent issues with below
    json j;
    j["type"] = "draw_regular_icosahedron";
    j["scale"] = scale;
    j["vertex_color"]["r"] = vertex_material.r;
    j["vertex_color"]["g"] = vertex_material.g;
    j["vertex_color"]["b"] = vertex_material.b;
    j["vertex_color"]["a"] = vertex_material.a;
    j["vertex_radius"] = verticies_radius;
    j["edge_color"]["r"] = edge_material.r;
    j["edge_color"]["g"] = edge_material.g;
    j["edge_color"]["b"] = edge_material.b;
    j["edge_color"]["a"] = edge_material.a;
    j["edge_thickness"] = edge_thickness;
    j["face_color"]["r"] = face_material.r;
    j["face_color"]["g"] = face_material.g;
    j["face_color"]["b"] = face_material.b;
    j["face_color"]["a"] = face_material.a;
    j["face_thickness"] = face_thickness;
    j["center_point"]["x"] = center_point.x;
    j["center_point"]["y"] = center_point.y;
    j["center_point"]["z"] = center_point.z;
    j["x_rot"]["x"] = x_rot;
    j["y_rot"]["y"] = y_rot;
    j["z_rot"]["z"] = z_rot;
    j["draw"] = draw;
    j["mask"] = mask;
    log(j.dump());
  }

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// sphere
float GLContainer::draw_sphere(glm::vec3 location, float radius,
                               glm::vec4 color, bool draw, int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_sphere";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["location"]["x"] = location.x;
  j["location"]["y"] = location.y;
  j["location"]["z"] = location.z;
  j["radius"] = radius;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// tube
float GLContainer::draw_tube(glm::vec3 bvec, glm::vec3 tvec, float inner_radius,
                             float outer_radius, glm::vec4 color, bool draw,
                             int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_tube";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["bvec"]["x"] = bvec.x;
  j["bvec"]["y"] = bvec.y;
  j["bvec"]["z"] = bvec.z;
  j["tvec"]["x"] = tvec.x;
  j["tvec"]["y"] = tvec.y;
  j["tvec"]["z"] = tvec.z;
  j["inner_radius"] = inner_radius;
  j["outer_radius"] = outer_radius;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// triangle
float GLContainer::draw_triangle(glm::vec3 point1, glm::vec3 point2,
                                 glm::vec3 point3, float thickness,
                                 glm::vec4 color, bool draw, int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_triangle";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["point1"]["x"] = point1.x;
  j["point1"]["y"] = point1.y;
  j["point1"]["z"] = point1.z;
  j["point2"]["x"] = point2.x;
  j["point2"]["y"] = point2.y;
  j["point2"]["z"] = point2.z;
  j["point3"]["x"] = point3.x;
  j["point3"]["y"] = point3.y;
  j["point3"]["z"] = point3.z;
  j["thickness"] = thickness;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// clear all
float GLContainer::clear_all(bool respect_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "clear_all";
  j["respect_mask"] = respect_mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// unmask all
float GLContainer::unmask_all() {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "unmask_all";
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// invert mask
float GLContainer::invert_mask() {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "invert_mask";
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// mask by color
float GLContainer::mask_by_color(bool r, bool g, bool b, bool a, bool l,
                                 glm::vec4 color, float l_val, float r_var,
                                 float g_var, float b_var, float a_var,
                                 float l_var, int amt) {

  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "mask_by_color";
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["r"] = r;
  j["g"] = g;
  j["b"] = b;
  j["a"] = a;
  j["l"] = l;
  j["l_val"] = l_val;
  j["l_var"] = l_var;
  j["r_var"] = r_var;
  j["g_var"] = g_var;
  j["b_var"] = b_var;
  j["a_var"] = a_var;
  j["amount"] = amt;
  log(j.dump());

  // don't need to redraw
  swap_blocks();
  glUseProgram(mask_by_color_compute);

  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_r"), r);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_g"), g);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_b"), b);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_a"), a);
  glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_l"), l);

  glUniform1i(glGetUniformLocation(mask_by_color_compute, "mask"), amt);

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// box blur
float GLContainer::box_blur(int radius, bool touch_alpha, bool respect_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "box_blur";
  j["radius"] = radius;
  j["touch_alpha"] = touch_alpha;
  j["respect_mask"] = respect_mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// gaussian blur
float GLContainer::gaussian_blur(int radius, bool touch_alpha,
                                 bool respect_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "gaussian_blur";
  j["radius"] = radius;
  j["touch_alpha"] = touch_alpha;
  j["respect_mask"] = respect_mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// limiter
float GLContainer::limiter() {
  auto t1 = std::chrono::high_resolution_clock::now();
  redraw_flag = true;
  color_mipmap_flag = true;

  // the details of this operation still need to be worked out - there is a
  // couple of different modes
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// shifting
float GLContainer::shift(glm::ivec3 movement, bool loop, int mode) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "shift";
  j["loop"] = loop;
  j["mode"] = mode;
  j["movement"]["x"] = movement.x;
  j["movement"]["y"] = movement.y;
  j["movement"]["z"] = movement.z;
  log(j.dump());

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

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

std::string GLContainer::compile_user_script(std::string text) {
  json j;
  j["type"] = "compile_user_script";
  j["text"] = text;
  log(j.dump());

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
  json j;
  j["type"] = "run_user_script";
  log(j.dump());

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

float GLContainer::copy_loadbuffer(bool respect_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();

  // doesn't make sense to make this available to the JSON thing because it is
  // only available through the load and VAT

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// Brent Werness's Voxel Automata Terrain - set redraw_flag to true
std::string GLContainer::vat(float flip, std::string rule, int initmode,
                             glm::vec4 color0, glm::vec4 color1,
                             glm::vec4 color2, float lambda, float beta,
                             float mag, bool respect_mask, glm::bvec3 mins,
                             glm::bvec3 maxs) {
  json j;
  j["type"] = "vat";
  j["color0"]["r"] = color0.r;
  j["color0"]["g"] = color0.g;
  j["color0"]["b"] = color0.b;
  j["color0"]["a"] = color0.a;
  j["color1"]["r"] = color1.r;
  j["color1"]["g"] = color1.g;
  j["color1"]["b"] = color1.b;
  j["color1"]["a"] = color1.a;
  j["color2"]["r"] = color2.r;
  j["color2"]["g"] = color2.g;
  j["color2"]["b"] = color2.b;
  j["color2"]["a"] = color2.a;
  j["lambda"] = lambda;
  j["beta"] = beta;
  j["flip"] = flip;
  j["mag"] = mag;
  j["rule"] = rule;
  j["init_mode"] = initmode;
  j["respect_mask"] = respect_mask;
  j["mins"]["x"] = mins.x;
  j["mins"]["y"] = mins.y;
  j["mins"]["z"] = mins.z;
  j["maxs"]["x"] = maxs.x;
  j["maxs"]["y"] = maxs.y;
  j["maxs"]["z"] = maxs.z;
  log(j.dump());

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
float GLContainer::load(std::string filename, bool respect_mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "load";
  j["filename"] = filename;
  j["respect_mask"] = respect_mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// save
float GLContainer::save(std::string filename) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "save";
  j["filename"] = filename;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// functions to generate new heightmaps
float GLContainer::generate_heightmap_diamond_square() {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "generate_heightmap_diamond_square";
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::generate_heightmap_perlin() {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "generate_heightmap_perlin";
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::generate_heightmap_XOR() {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "generate_heightmap_xor";
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// function to generate new block of 3d perlin noise
float GLContainer::generate_perlin_noise(float xscale = 0.014,
                                         float yscale = 0.04,
                                         float zscale = 0.014, int seed = 0) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "generate_perlin_noise";
  j["xscale"] = xscale;
  j["yscale"] = yscale;
  j["zscale"] = zscale;
  j["seed"] = seed;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

// float GLContainer::generate_glm_perlin()
// {
//   auto t1 = std::chrono::high_resolution_clock::now();
//   std::vector<unsigned char> data;

//   for (int x = 0; x < DIM; x++)
//     for (int y = 0; y < DIM; y++)
//       for (int z = 0; z < DIM; z++) {
//         float sample = glm::perlin(glm::vec3(x*0.014, y*0.04, z*0.014));
//         data.push_back((unsigned char)(sample * 255));
//         data.push_back((unsigned char)(sample * 255));
//         data.push_back((unsigned char)(sample * 255));
//         data.push_back(255);
//       }

//   auto t2 = std::chrono::high_resolution_clock::now();
//   return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
// }

float GLContainer::gen_noise(int preset, int seed) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["preset"] = preset;
  j["seed"] = seed;
  log(j.dump());

  auto fnPerlin = FastNoise::New<FastNoise::Perlin>();
  // Create an array of floats to store the noise output in
  std::vector<float> noiseOutput(DIM * DIM * DIM);

  // Generate a 16 x 16 x 16 area of noise
  fnPerlin->GenUniformGrid3D(noiseOutput.data(), 0, 0, 0, DIM, DIM, DIM,
                                0.02f, 1337);

  cout << endl << fnPerlin->GetSIMDLevel() << endl << endl;

  // FastNoise::SmartNode<> fnGenerator =
  // FastNoise::NewFromEncodedNodeTree("DQAFAAAAAAAAQAgAAAAAAD8AAAAAAA==");
  // int index = 0;
  // for (int z = 0; z < DIM; z++) {
  // for (int y = 0; y < DIM; y++) {
  // for (int x = 0; x < DIM; x++) {
  // ProcessVoxelData(x, y, z, noiseOutput[index++]);
  // cout << noiseOutput[index++] << " ";
  // }
  // cout << endl;
  // }
  // cout << endl;
  // }

  // send noiseOutput to the GPU
  glBindTexture(GL_TEXTURE_3D, textures[11]);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &noiseOutput[0]);
  glGenerateMipmap(GL_TEXTURE_3D);

  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}

float GLContainer::draw_noise(float low_thresh, float high_thresh, bool smooth,
                              glm::vec4 color, bool draw, int mask) {
  auto t1 = std::chrono::high_resolution_clock::now();
  json j;
  j["type"] = "draw_noise";
  j["low_thresh"] = low_thresh;
  j["high_thresh"] = high_thresh;
  j["smooth"] = smooth;
  j["color"]["r"] = color.r;
  j["color"]["g"] = color.g;
  j["color"]["b"] = color.b;
  j["color"]["a"] = color.a;
  j["draw"] = draw;
  j["mask"] = mask;
  log(j.dump());

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
  auto t2 = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
}
