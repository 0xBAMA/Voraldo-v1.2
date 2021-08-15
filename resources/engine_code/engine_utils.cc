#include "engine.h"
// This contains the lower level code

// TinyOBJLoader - This has to be included in a .cc file, so it's here for right
// now
#define TINYOBJLOADER_IMPLEMENTATION
// #define TINYOBJLOADER_USE_DOUBLE
#include "../TinyOBJLoader/tiny_obj_loader.h"

// tinyobj callbacks
//  user_data is passed in as void, then cast as 'engine' class to push
//  vertices, normals, texcoords, index, material info
//   the vectors for the data are made public so that everything is accessible
void vertex_cb(void *user_data, float x, float y, float z, float w) {
  engine *t = reinterpret_cast<engine *>(user_data);

  t->vertices.push_back(glm::vec4(x, y, z, w));
}

void normal_cb(void *user_data, float x, float y, float z) {
  engine *t = reinterpret_cast<engine *>(user_data);

  t->normals.push_back(glm::vec3(x, y, z));
}

void texcoord_cb(void *user_data, float x, float y, float z) {
  engine *t = reinterpret_cast<engine *>(user_data);

  t->texcoords.push_back(glm::vec3(x, y, z));
}

void index_cb(void *user_data, tinyobj::index_t *indices, int num_indices) {
  engine *t = reinterpret_cast<engine *>(user_data);

  if (num_indices == 3) // this is a triangle
  {
    // OBJ uses 1-indexing, convert to 0-indexing
    t->triangle_indices.push_back(glm::ivec3(indices[0].vertex_index - 1,
                                             indices[1].vertex_index - 1,
                                             indices[2].vertex_index - 1));
    t->normal_indices.push_back(glm::ivec3(indices[0].normal_index - 1,
                                           indices[1].normal_index - 1,
                                           indices[2].normal_index - 1));
    t->texcoord_indices.push_back(glm::ivec3(indices[0].texcoord_index - 1,
                                             indices[1].texcoord_index - 1,
                                             indices[2].texcoord_index - 1));
  }

  // lines, points have a different number of indicies
  //  might want to handle these
}

void usemtl_cb(void *user_data, const char *name, int material_idx) {
  engine *t = reinterpret_cast<engine *>(user_data);
  (void)t;
}

void mtllib_cb(void *user_data, const tinyobj::material_t *materials,
               int num_materials) {
  engine *t = reinterpret_cast<engine *>(user_data);
  (void)t;
}

void group_cb(void *user_data, const char **names, int num_names) {
  engine *t = reinterpret_cast<engine *>(user_data);
  (void)t;
}

void object_cb(void *user_data, const char *name) {
  engine *t = reinterpret_cast<engine *>(user_data);
  (void)t;
}

// this is where the callbacks are used
void engine::load_OBJ(std::string filename) {
  tinyobj::callback_t cb;
  cb.vertex_cb = vertex_cb;
  cb.normal_cb = normal_cb;
  cb.texcoord_cb = texcoord_cb;
  cb.index_cb = index_cb;
  cb.usemtl_cb = usemtl_cb;
  cb.mtllib_cb = mtllib_cb;
  cb.group_cb = group_cb;
  cb.object_cb = object_cb;

  std::string warn;
  std::string err;

  std::ifstream ifs(filename.c_str());
  tinyobj::MaterialFileReader mtlReader(".");

  bool ret =
      tinyobj::LoadObjWithCallback(ifs, cb, this, &mtlReader, &warn, &err);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << err << std::endl;
  }

  if (!ret) {
    std::cerr << "Failed to parse .obj" << std::endl;
  }

  cout << "vertex list length: " << vertices.size() << endl;
  cout << "normal list length: " << normals.size() << endl;
  cout << "texcoord list length: " << texcoords.size() << endl;

  cout << "vertex index list length: " << triangle_indices.size() << endl;
  cout << "normal index length: " << normal_indices.size() << endl;
  cout << "texcoord index length: " << texcoord_indices.size() << endl;
}

// used in load/save operation to check extension
bool hasEnding(std::string fullString, std::string ending) {
  if (fullString.length() >= ending.length()) {
    return (0 == fullString.compare(fullString.length() - ending.length(),
                                    ending.length(), ending));
  } else {
    return false;
  }
}

bool hasPNG(std::string filename) {
  return hasEnding(filename, std::string(".png"));
}

// got this from
// http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html the
// intention is to read all the files in the current directory and put them in
// the array

#define LISTBOX_SIZE 256
std::vector<std::string> directory_strings;

struct path_leaf_string {
  std::string operator()(const std::filesystem::directory_entry &entry) const {
    return entry.path().string();
  }
};

void update_listbox_items() {
  directory_strings.clear();

  std::filesystem::path p("saves");
  std::filesystem::directory_iterator start(p);
  std::filesystem::directory_iterator end;

  std::transform(start, end, std::back_inserter(directory_strings),
                 path_leaf_string());

  // sort these alphabetically
  std::sort(directory_strings.begin(), directory_strings.end());
}

void engine::SDL2_setup() {
  cout << "creating window.............";

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("Error: %s\n", SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

  // this is how you query the screen resolution
  SDL_DisplayMode dm;
  SDL_GetDesktopDisplayMode(0, &dm);

  // pulling these out because I'm going to try to span the whole screen with
  // the window, in a way that's flexible on different resolution screens
  total_screen_width = dm.w;
  total_screen_height = dm.h;

  window = SDL_CreateWindow(
      "Voraldo v1.2", 0, 0, total_screen_width, total_screen_height,
      SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS);
  SDL_ShowWindow(window);

  cout << "...done." << endl;

  cout << "setting up OpenGL context...";
  // OpenGL 4.3 + GLSL version 430
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  GLcontext = SDL_GL_CreateContext(window);

  SDL_GL_MakeCurrent(window, GLcontext);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  // SDL_GL_SetSwapInterval(0); // explicitly disable vsync

  cout << "...done." << endl << endl;
}

void engine::gl_setup() {
  if (gl3wInit() != 0) {
    fprintf(stderr, "\n\nFailed to initialize OpenGL loader!\n\n");
    abort(); // nothing's going to work
  }

  // we're now good to start using functions with the gl- prefix
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);

  glPointSize(3.0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // some info on your current platform
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n\n", version);

  GPU_Data.screen_width = total_screen_width;
  GPU_Data.screen_height = total_screen_height;

  GPU_Data.orientation_widget_offset = glm::vec3(0.94, -0.905, 0.0);
  // setup completed, show the window and start rendering
}

void engine::imgui_setup() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io; // void cast prevents unused variable warning

  // enable docking - disables floating windows
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // adds a font, as default - otherwise it uses the default ImGUI font
  // io.Fonts->AddFontFromFileTTF("resources/fonts/star_trek/titles/Jefferies.ttf",
  // 15);

  // Setup Platform/Renderer bindings
  const char *glsl_version = "#version 430";
  ImGui_ImplSDL2_InitForOpenGL(window, GLcontext);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // set OpenGL clear color
  GPU_Data.clear_color =
      glm::vec4(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f,
                1.0f); // initial value for clear color
  glClearColor(GPU_Data.clear_color.x, GPU_Data.clear_color.y,
               GPU_Data.clear_color.z, GPU_Data.clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(window);

#define FPS_HISTORY_SIZE 300
  fps_history.resize(FPS_HISTORY_SIZE); // initialize the array of fps values

  ImVec4 *colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(0.67f, 0.50f, 0.16f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.27f, 0.16f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.05f, 0.00f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.23f, 0.17f, 0.02f, 0.05f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.30f, 0.12f, 0.06f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.25f, 0.18f, 0.09f, 0.33f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.561f, 0.082f, 0.04f, 0.17f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.09f, 0.02f, 0.17f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.25f, 0.12f, 0.01f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.25f, 0.12f, 0.01f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.07f, 0.02f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.10f, 0.08f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.69f, 0.45f, 0.11f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.18f, 0.06f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.36f, 0.22f, 0.06f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_Header] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.18f, 0.06f, 0.37f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.42f, 0.18f, 0.06f, 0.17f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.12f, 0.01f, 0.78f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.34f, 0.14f, 0.01f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.33f, 0.15f, 0.02f, 1.00f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.18f, 0.06f, 1.00f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.06f, 0.03f, 0.01f, 0.78f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.64f, 0.42f, 0.09f, 0.90f);
  colors[ImGuiCol_NavHighlight] = ImVec4(0.64f, 0.42f, 0.09f, 0.90f);
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

  ImGuiStyle &style = ImGui::GetStyle();

  style.TabRounding = 2;
  style.FrameRounding = 2;
  style.WindowPadding.x = 0;
  style.WindowPadding.y = 0;
  style.FramePadding.x = 1;
  style.FramePadding.y = 0;
  style.IndentSpacing = 8;
  style.WindowRounding = 3;
  style.ScrollbarSize = 10;

  update_listbox_items(); // populate the list of saves initially
}

void OrangeText(const char *string) {
  ImGui::Separator();
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75, 0.35, 0.1, 1.));
  ImGui::Text(string);
  ImGui::PopStyleColor();
}

void WrappedText(const char *string) {
  // ImGui::PushTextWrapPos(ImGui::GetFontSize() * wrap);
  ImGui::PushTextWrapPos(ImGui::GetWindowSize().x);
  ImGui::TextUnformatted(string);
  ImGui::PopTextWrapPos();
}

static void HelpMarker(const char *desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

void engine::show_voraldo_menu(bool *show) {
  ImGui::Begin("Voraldo Menu", show,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoScrollbar);

  ImGuiTabBarFlags tab_bar_flags_wdropdown =
      ImGuiTabBarFlags_TabListPopupButton |
      ImGuiTabBarFlags_FittingPolicyScroll;
  ImGuiTabBarFlags tab_bar_flags_wodropdown =
      ImGuiTabBarFlags_FittingPolicyScroll;

  if (ImGui::BeginTabBar("top_level", tab_bar_flags_wodropdown)) {
    if (ImGui::BeginTabItem(" Shapes ")) {
      ImGui::BeginTabBar("shapes", tab_bar_flags_wdropdown);
      if (ImGui::BeginTabItem(" AABB ")) {
        static glm::vec3 max, min;
        static ImVec4 aabb_draw_color;
        static bool aabb_draw = true;
        static int aabb_mask = 0;

        WrappedText(" Axis-Aligned Bounding Box (AABB) ");
        ImGui::SameLine();
        HelpMarker("This is defined by some minimum and maximum extent on each "
                   "of the three axes, x, y, and z. The affected area is "
                   "between the minimum and maximum extents. Make sure each "
                   "maximum is greater than the corresponding minimum.");

        OrangeText("EXTENTS");
        ImGui::SliderFloat(" x max", &max.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" x min", &min.x, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat(" y max", &max.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" y min", &min.y, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat(" z max", &max.z, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" z min", &min.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        OrangeText("OPTIONS");

        ImGui::Checkbox("  Draw ", &aabb_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &aabb_mask);

        // bounds check
        aabb_mask = std::clamp(aabb_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&aabb_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          // draw the sphere with the selected values
          GPU_Data.draw_aabb(min, max,
                             glm::vec4(aabb_draw_color.x, aabb_draw_color.y,
                                       aabb_draw_color.z, aabb_draw_color.w),
                             aabb_draw, aabb_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Cuboid ")) {
        // kind of start with some defaults because this is not super easy to
        // use
        static glm::vec3 a = glm::vec3(20, 230, 230);  // a : -x, +y, +z
        static glm::vec3 b = glm::vec3(20, 20, 230);   // b : -x, -y, +z
        static glm::vec3 c = glm::vec3(230, 230, 230); // c : +x, +y, +z
        static glm::vec3 d = glm::vec3(230, 20, 230);  // d : +x, -y, +z
        static glm::vec3 e = glm::vec3(20, 230, 20);   // e : -x, +y, -z
        static glm::vec3 f = glm::vec3(20, 20, 20);    // f : -x, -y, -z
        static glm::vec3 g = glm::vec3(230, 230, 20);  // g : +x, +y, -z
        static glm::vec3 h = glm::vec3(230, 20, 20);   // h : +x, -y, -z

        static bool cuboid_draw = true;
        static int cuboid_mask = 0;
        static ImVec4 cuboid_draw_color;

        WrappedText(" Cuboid ");
        ImGui::SameLine();
        HelpMarker(
            "This is hard to use.\n"
            "     e-------g    +y     \n"
            "    /|      /|     |     \n"
            "   / |     / |     |___+x\n"
            "  a-------c  |    /      \n"
            "  |  f----|--h   +z      \n"
            "  | /     | /            \n"
            "  |/      |/             \n"
            "  b-------d              \n"
            "Also called a quadrillateral hexahedron, this is defined by 8 "
            "points at the corners. The simplest example is a cube - all faces "
            "squares, as with the default parameters. The options available to "
            "you are scaling and skewing, with the limitation that the four "
            "points making up a face should lie in a plane, or there will be "
            "artifacts. Due to the way the shape is processed the "
            "quadrilateral faces have sort of a 'winding order' in the way "
            "that they are interpreted, because it uses planes defined by the "
            "verticies of each face.");

        OrangeText("8 POINTS");
        ImGui::Separator();

        ImGui::SliderFloat("a x", &a.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("a y", &a.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("a z", &a.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("b x", &b.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("b y", &b.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("b z", &b.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("c x", &c.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("c y", &c.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("c z", &c.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("d x", &d.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("d y", &d.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("d z", &d.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("e x", &e.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("e y", &e.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("e z", &e.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("f x", &f.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("f y", &f.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("f z", &f.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("g x", &g.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("g y", &g.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("g z", &g.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        ImGui::SliderFloat("h x", &h.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("h y", &h.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("h z", &h.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();
        OrangeText("OPTIONS");

        ImGui::Checkbox("  Draw ", &cuboid_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &cuboid_mask);

        // bounds check
        cuboid_mask = std::clamp(cuboid_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&cuboid_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_cuboid(
              a, b, c, d, e, f, g, h,
              glm::vec4(cuboid_draw_color.x, cuboid_draw_color.y,
                        cuboid_draw_color.z, cuboid_draw_color.w),
              cuboid_draw, cuboid_mask);
        }

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Cylinder ")) {
        static glm::vec3 cylinder_bvec, cylinder_tvec;
        static bool cylinder_draw = true;
        static int cylinder_mask = 0;
        static ImVec4 cylinder_draw_color;
        static float cylinder_radius;

        WrappedText(" Cylinder ");
        ImGui::SameLine();
        HelpMarker("Cylinder is defined by two points. tvec is in the center "
                   "of the top and bvec is in the center of the bottom. "
                   "Thickness determines the radius of the cylinder.");

        OrangeText("RADIUS");
        ImGui::SliderFloat("radius", &cylinder_radius, 0.0f, float(DIM),
                           "%.3f");

        ImGui::Separator();

        OrangeText("BVEC");
        ImGui::SliderFloat("bvec x", &cylinder_bvec.x, 0.0f, float(DIM),
                           "%.3f");
        ImGui::SliderFloat("bvec y", &cylinder_bvec.y, 0.0f, float(DIM),
                           "%.3f");
        ImGui::SliderFloat("bvec z", &cylinder_bvec.z, 0.0f, float(DIM),
                           "%.3f");

        ImGui::Separator();

        OrangeText("TVEC");
        ImGui::SliderFloat("tvec x", &cylinder_tvec.x, 0.0f, float(DIM),
                           "%.3f");
        ImGui::SliderFloat("tvec y", &cylinder_tvec.y, 0.0f, float(DIM),
                           "%.3f");
        ImGui::SliderFloat("tvec z", &cylinder_tvec.z, 0.0f, float(DIM),
                           "%.3f");

        ImGui::Separator();
        OrangeText("OPTIONS");

        ImGui::Checkbox("  Draw ", &cylinder_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &cylinder_mask);

        // bounds check
        cylinder_mask = std::clamp(cylinder_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&cylinder_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_cylinder(
              cylinder_bvec, cylinder_tvec, cylinder_radius,
              glm::vec4(cylinder_draw_color.x, cylinder_draw_color.y,
                        cylinder_draw_color.z, cylinder_draw_color.w),
              cylinder_draw, cylinder_mask);
        }

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Ellipsoid ")) {
        static glm::vec3 radius, center, rotation;
        static bool ellipsoid_draw = true;
        static int ellipsoid_mask = 0;
        static ImVec4 ellipsoid_draw_color;

        WrappedText(" Ellipsoid ");
        ImGui::SameLine();
        HelpMarker(
            "The ellipsoid is similar to the sphere but has three different "
            "radii, one for each axis. In addition to this, three rotations "
            "can be combined to give the desired orientation.");

        OrangeText("LOCATION");
        ImGui::SliderFloat(" x", &center.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" y", &center.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" z", &center.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        OrangeText("RADII");
        ImGui::SliderFloat("x radius", &radius.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("y radius", &radius.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("z radius", &radius.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        OrangeText("ROTATION");
        ImGui::SliderFloat("x rotation", &rotation.x, 0.0f, 6.28f, "%.3f");
        ImGui::SliderFloat("y rotation", &rotation.y, 0.0f, 6.28f, "%.3f");
        ImGui::SliderFloat("z rotation", &rotation.z, 0.0f, 6.28f, "%.3f");

        ImGui::Separator();
        OrangeText("OPTIONS");

        ImGui::Checkbox("  Draw ", &ellipsoid_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &ellipsoid_mask);

        // bounds check
        ellipsoid_mask = std::clamp(ellipsoid_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&ellipsoid_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_ellipsoid(
              center, radius, rotation,
              glm::vec4(ellipsoid_draw_color.x, ellipsoid_draw_color.y,
                        ellipsoid_draw_color.z, ellipsoid_draw_color.w),
              ellipsoid_draw, ellipsoid_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Grid ")) {
        static int xoff, yoff, zoff;
        static int xspacing, yspacing, zspacing;
        static int xwid, ywid, zwid;
        static float xrot, yrot, zrot;
        static ImVec4 grid_draw_color;
        static bool grid_draw = true;
        static int grid_mask = 0;

        WrappedText(" Grid ");
        ImGui::SameLine();
        HelpMarker("Grid is a regularly spaced orthogonal lattice. Use the "
                   "spacing control to set the distance between grid lines on "
                   "each axis. Width sets the width of these grid lines, along "
                   "each axis, and offset allows the whole grid to be moved.");

        OrangeText("SPACING");
        ImGui::SliderInt(" xs", &xspacing, 0, 100);
        ImGui::SliderInt(" ys", &yspacing, 0, 100);
        ImGui::SliderInt(" zs", &zspacing, 0, 100);

        OrangeText("WIDTH");
        ImGui::SliderInt(" xw", &xwid, 0, 25);
        ImGui::SliderInt(" yw", &ywid, 0, 25);
        ImGui::SliderInt(" zw", &zwid, 0, 25);

        OrangeText("OFFSET");
        ImGui::SliderInt(" xo", &xoff, 0, 25);
        ImGui::SliderInt(" yo", &yoff, 0, 25);
        ImGui::SliderInt(" zo", &zoff, 0, 25);

        OrangeText("ROTATION");
        ImGui::SliderFloat(" xr", &xrot, -pi, pi);
        ImGui::SliderFloat(" yr", &yrot, -pi, pi);
        ImGui::SliderFloat(" zr", &zrot, -pi, pi);

        OrangeText("OPTIONS");
        ImGui::Checkbox("  Draw ", &grid_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &grid_mask);

        // bounds check
        grid_mask = std::clamp(grid_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&grid_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");

        ImGui::SetCursorPosX(16);
        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_grid(glm::ivec3(xspacing, yspacing, zspacing),
                             glm::ivec3(xwid, ywid, zwid),
                             glm::ivec3(xoff, yoff, zoff),
                             glm::vec3(xrot, yrot, zrot),
                             glm::vec4(grid_draw_color.x, grid_draw_color.y,
                                       grid_draw_color.z, grid_draw_color.w),
                             grid_draw, grid_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Heightmap ")) {
        static float heightmap_vertical_scale = 1.0;
        static bool heightmap_draw = true;
        static int heightmap_mask = 0;
        static bool height_color = true;
        static ImVec4 heightmap_draw_color;

        WrappedText(" Heightmap ");
        ImGui::SameLine();
        HelpMarker("This is a tool to draw heightmaps up the y-axis. Use the "
                   "vertical scale to set the height scaling. There are three "
                   "algorithms for heightmap generation, as labeled below.");

        OrangeText("PREVIEW");
        // show off the currently held texture - for some reason it is the
        // number of the texture unit + 1
        ImGui::Image((void *)(intptr_t)13 /*not a good way to do this, but it
                                             works right now*/
                     ,
                     ImVec2(240, 256));
        OrangeText("GENERATION ALGORITHMS");

        if (ImGui::SmallButton(" Perlin ")) {
          GPU_Data.generate_heightmap_perlin();
        }

        ImGui::SameLine();

        if (ImGui::SmallButton(" Diamond-Square ")) {
          GPU_Data.generate_heightmap_diamond_square();
        }

        ImGui::SameLine();

        if (ImGui::SmallButton(" XOR ")) {
          GPU_Data.generate_heightmap_XOR();
        }

        OrangeText("VERTICAL SCALE");

        ImGui::Separator();
        ImGui::SliderFloat(" Scale", &heightmap_vertical_scale, 0.0f, 5.0f,
                           "%.3f");
        ImGui::Separator();

        OrangeText("OPTIONS");
        ImGui::Checkbox("  Height Color ", &height_color);
        ImGui::Checkbox("  Draw ", &heightmap_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &heightmap_mask);

        // bounds check
        heightmap_mask = std::clamp(heightmap_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&heightmap_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          // draw the heightmap with the selected values
          GPU_Data.draw_heightmap(
              heightmap_vertical_scale, height_color,
              glm::vec4(heightmap_draw_color.x, heightmap_draw_color.y,
                        heightmap_draw_color.z, heightmap_draw_color.w),
              heightmap_draw, heightmap_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Icosahedron ")) {
        static ImVec4 color0 =
            ImVec4(200.0 / 255.0, 49.0 / 255.0, 11.0 / 255.0, 10.0 / 255.0);
        static ImVec4 color1 =
            ImVec4(207.0 / 255.0, 179.0 / 255.0, 7.0 / 255.0, 125.0 / 255.0);
        static ImVec4 color2 =
            ImVec4(190.0 / 255.0, 95.0 / 255.0, 0.0 / 255.0, 155.0 / 255.0);

        static ImVec4 vertex_color = color0;
        static float vertex_radius = 0.;

        static ImVec4 edge_color = color1;
        static float edge_radius = 0.;

        static ImVec4 face_color = color2;
        static float face_thickness = 0.;

        static glm::vec3 center_point = glm::vec3(0);
        static glm::vec3 rotations = glm::vec3(0);
        static bool draw = true;
        static int mask = 0;
        static float scale = 20.0;

        WrappedText(" Icosahedron ");
        ImGui::SameLine();
        HelpMarker(
            "This is a simple way to draw a regular icosahedron. You may be "
            "familiar with the shape from a 20-sided die. It is defined by the "
            "12 points at the corners of '3 mutually orthogonal golden "
            "rectangles' that share a center point. The shape itself can be "
            "scaled and rotated, and parameters can be set for each component "
            "of the drawn result, colors, radii, etc.");

        OrangeText("SCALE");
        ImGui::SliderFloat("  scale", &scale, 0.0f, DIM, "%.3f");

        OrangeText("POSITION");
        ImGui::SliderFloat("  xpos", &center_point.x, 0.0f, DIM, "%.3f");
        ImGui::SliderFloat("  ypos", &center_point.y, 0.0f, DIM, "%.3f");
        ImGui::SliderFloat("  zpos", &center_point.z, 0.0f, DIM, "%.3f");
        OrangeText("ROTATION");
        ImGui::SliderFloat("  xrot", &rotations.x, -2. * pi, 2 * pi, "%.3f");
        ImGui::SliderFloat("  yrot", &rotations.y, -2. * pi, 2 * pi, "%.3f");
        ImGui::SliderFloat("  zrot", &rotations.z, -2. * pi, 2 * pi, "%.3f");
        OrangeText("SUBSHAPE PARAMETERS");
        ImGui::SliderFloat("  vertex radius", &vertex_radius, 0., 20., "%.3f");
        ImGui::SliderFloat("  edge radius", &edge_radius, 0., 20., "%.3f");
        ImGui::SliderFloat("  face thickness", &face_thickness, 0., 20.,
                           "%.3f");
        OrangeText("COLORS");

        ImGui::ColorEdit4("  Vertex", (float *)&vertex_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::ColorEdit4("  Edge", (float *)&edge_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::ColorEdit4("  Face", (float *)&face_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);
        OrangeText("OPTIONS");

        ImGui::Checkbox("  Draw ", &draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &mask);

        // bounds check
        mask = std::clamp(mask, 0, 255);
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_regular_icosahedron(
              rotations.x, rotations.y, rotations.z, scale, center_point,
              glm::vec4(vertex_color.x, vertex_color.y, vertex_color.z,
                        vertex_color.w),
              vertex_radius,
              glm::vec4(edge_color.x, edge_color.y, edge_color.z, edge_color.w),
              edge_radius,
              glm::vec4(face_color.x, face_color.y, face_color.z, face_color.w),
              face_thickness, draw, mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Noise ")) {
        static float noise_scale_x = 0.014;
        static float noise_scale_y = 0.014;
        static float noise_scale_z = 0.014;

        static float noise_threshold_lo = 0.0f;
        static float noise_threshold_hi = 0.0f;

        static ImVec4 noise_draw_color;
        static bool noise_draw = true;
        static int noise_mask = 0;
        static bool noise_smooth = false;

        WrappedText(" WIP FastNoise2 integration ");
        ImGui::SameLine();
        HelpMarker(
            "This is to create a more general and flexible noise tool with "
            "different algorithms and a node graph. TBD how multiple channels "
            "will be used, but that seems to have potential. \n\nThe way the "
            "noise has been used in previous verions, there is a low and high "
            "threshold set when drawing - noise values are in the range 0-1, "
            "so the drawing operation will be applied to cells which read a "
            "noise sample in the defined range between lothresh and hithresh.");

        ImGui::SliderFloat("  xscale", &noise_scale_x, 0.01f, 0.5f, "%.3f");
        ImGui::SliderFloat("  yscale", &noise_scale_y, 0.01f, 0.5f, "%.3f");
        ImGui::SliderFloat("  zscale", &noise_scale_z, 0.01f, 0.5f, "%.3f");
        ImGui::Text(" ");

        if (ImGui::SmallButton(" generate ")) {
          GPU_Data.generate_perlin_noise(noise_scale_x, noise_scale_y,
                                         noise_scale_z, 0);
        }

        ImGui::Separator();

        WrappedText(
            "Perlin noise ranges from 0 to 1. Use hithresh and lowthresh to "
            "tell how much of this perlin texture to color in.");

        OrangeText("THRESHOLDING");
        ImGui::SliderFloat(" hithresh", &noise_threshold_hi, 0.0f, 1.0f,
                           "%.3f");
        ImGui::SliderFloat(" lothresh", &noise_threshold_lo, 0.0f, 1.0f,
                           "%.3f");

        ImGui::Checkbox(" Smooth Color ", &noise_smooth);

        ImGui::Separator();

        OrangeText("OPTIONS");
        ImGui::Checkbox("  Draw ", &noise_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &noise_mask);

        // bounds check
        noise_mask = std::clamp(noise_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&noise_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");

        ImGui::SetCursorPosX(16);
        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_noise(noise_threshold_lo, noise_threshold_hi,
                              noise_smooth,
                              glm::vec4(noise_draw_color.x, noise_draw_color.y,
                                        noise_draw_color.z, noise_draw_color.w),
                              noise_draw, noise_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Sphere ")) {
        static bool sphere_draw = true;
        static int sphere_mask = 0;
        static float sphere_radius = 0.0;
        static ImVec4 sphere_draw_color;
        static glm::vec3 sphere_location;

        WrappedText(" Sphere ");
        ImGui::SameLine();
        HelpMarker(
            "Spheres are all the points that are within a certain "
            "distance of a center point. Specify radius and center point.");

        OrangeText("RADIUS");
        ImGui::SliderFloat("  radius", &sphere_radius, 0.0f, 500.0f, "%.3f");

        ImGui::Separator();

        OrangeText("POSITION");
        ImGui::SliderFloat(" x", &sphere_location.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" y", &sphere_location.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat(" z", &sphere_location.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        OrangeText("OPTIONS");
        ImGui::Checkbox("  Draw ", &sphere_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &sphere_mask);

        // bounds check
        sphere_mask = std::clamp(sphere_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&sphere_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_sphere(
              sphere_location, sphere_radius,
              glm::vec4(sphere_draw_color.x, sphere_draw_color.y,
                        sphere_draw_color.z, sphere_draw_color.w),
              sphere_draw, sphere_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Tube ")) {
        static glm::vec3 tube_bvec, tube_tvec;
        static bool tube_draw = true;
        static int tube_mask = 0;
        static ImVec4 tube_draw_color;
        static float tube_inner_radius, tube_outer_radius;

        WrappedText(" Tube ");
        ImGui::SameLine();
        HelpMarker(
            "Tube is a cylinder with a cylinder cut out from the "
            "center. Outer is the outer radius, and inner is the radius of the "
            "cutout. Bvec and tvec are the center points of the bottom and top "
            "faces, respectively.");

        OrangeText("RADII");
        ImGui::SliderFloat("inner radius", &tube_inner_radius, 0.0f, 300.0f,
                           "%.3f");
        ImGui::SliderFloat("outer radius", &tube_outer_radius, 0.0f, 300.0f,
                           "%.3f");

        ImGui::Separator();
        OrangeText("BVEC");

        ImGui::SliderFloat("bvec x", &tube_bvec.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("bvec y", &tube_bvec.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("bvec z", &tube_bvec.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        OrangeText("TVEC");
        ImGui::SliderFloat("tvec x", &tube_tvec.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("tvec y", &tube_tvec.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("tvec z", &tube_tvec.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();

        OrangeText("OPTIONS");
        ImGui::Checkbox("  Draw ", &tube_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &tube_mask);

        // bounds check
        tube_mask = std::clamp(tube_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&tube_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_tube(tube_bvec, tube_tvec, tube_inner_radius,
                             tube_outer_radius,
                             glm::vec4(tube_draw_color.x, tube_draw_color.y,
                                       tube_draw_color.z, tube_draw_color.w),
                             tube_draw, tube_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Triangle ")) {
        static float thickness;
        static glm::vec3 point1, point2, point3;
        static ImVec4 triangle_draw_color;
        static bool triangle_draw = true;
        static int triangle_mask = 0;

        WrappedText(" Triangle ");
        ImGui::SameLine();
        HelpMarker(
            "Triangles consist of three points, use the sliders below "
            "to set each x, y and z value. Thickness will set the "
            "thickness of the triangle. The basic algorithm considers a plane "
            "in which these three points lie, and constructs a convex "
            "triangular prism from them - there are better (faster) methods, "
            "but I like the way it considers the volume in a flexible way. ");

        OrangeText("THICKNESS");
        ImGui::SliderFloat(" thickness", &thickness, 0.0f, 300.0f, "%.3f");

        ImGui::Separator();
        OrangeText("POINT 1");

        ImGui::SliderFloat("  x1 ", &point1.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("  y1 ", &point1.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("  z1 ", &point1.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();
        OrangeText("POINT 2");

        ImGui::SliderFloat("  x2 ", &point2.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("  y2 ", &point2.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("  z2 ", &point2.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();
        OrangeText("POINT 3");

        ImGui::SliderFloat("  x3 ", &point3.x, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("  y3 ", &point3.y, 0.0f, float(DIM), "%.3f");
        ImGui::SliderFloat("  z3 ", &point3.z, 0.0f, float(DIM), "%.3f");

        ImGui::Separator();
        OrangeText("OPTIONS");

        ImGui::Checkbox("  Draw ", &triangle_draw);
        ImGui::SameLine();
        ImGui::InputInt(" Mask ", &triangle_mask);

        // bounds check
        triangle_mask = std::clamp(triangle_mask, 0, 255);

        ImGui::ColorEdit4("  Color", (float *)&triangle_draw_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        ImGui::Text(" ");
        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Draw ")) {
          GPU_Data.draw_triangle(
              point1, point2, point3, thickness,
              glm::vec4(triangle_draw_color.x, triangle_draw_color.y,
                        triangle_draw_color.z, triangle_draw_color.w),
              triangle_draw, triangle_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" User ")) {
        static bool in_editor;

        ImGui::BeginTabBar("user", tab_bar_flags_wodropdown);
        if (ImGui::BeginTabItem(" Editor ")) {
          draw_user_editor_tab_contents();
          ImGui::EndTabItem();
          in_editor = true;
        }
        if (ImGui::BeginTabItem(" Documentation ")) {
          // documentation provides the very basic description and
          // usage of the SDF functions and operators represented here:

          // SDFS
          // https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
          // https://iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
          // https://iquilezles.org/www/articles/ellipsoids/ellipsoids.htm
          // https://iquilezles.org/www/articles/interiordistance/interiordistance.htm

          // OPERATORS
          // https://twitter.com/gaziya5/status/1354945792851668999
          // https://iquilezles.org/www/articles/smin/smin.htm
          // https://iquilezles.org/www/articles/menger/menger.htm

          // OTHER FUNCTIONS
          // https://iquilezles.org/www/articles/functions/functions.htm - HOT
          WrappedText("You bring an is_inside() function, and a shader will be "
                      "created. It is then invoked per-voxel with the same "
                      "logic as the rest of the drawing functions.");
          WrappedText(
              "A header is provided with some basic SDF primitives and "
              "operators, as well as some interesting easing functions. "
              "Individual descriptions can also be found in the console, "
              "contained in the manual entries.");
          ImGui::Separator();
          WrappedText("Fill out an irec it will be applied with the"
                      " selected mask blending mode.\n");
          ImGui::Text("");

          // this goes in the documentation tab
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75, 0.35, 0.1, 1.));
          ImGui::Text("HOW TO USE");
          ImGui::PopStyleColor();

          WrappedText("Abstractions are provided as follows:");
          ImGui::Separator();
          WrappedText(
              "- myloc is a vec3 with the centerpoint's location in the "
              "unit cube (-1, 1). This is to simplify the use of "
              "glGlobalInvocationID.xyz, which is still available if you "
              "prefer working with integers over this 'normalized' mapping.\n");

          WrappedText("- An output struct definition exists like this:\n"
                      "  struct irec{\n"
                      "    bool draw = false;\n"
                      "    vec4 color = vec4(0);\n"
                      "    int mask = 0;\n"
                      "  };\n");

          ImGui::EndTabItem();
          in_editor = false;
        }

        ImGui::EndTabBar();

        if (in_editor) {
          ImGui::SameLine();
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75, 0.35, 0.1, 1.));
          ImGui::Text("Voraldo v1.2 User Shader Editor");
          ImGui::PopStyleColor();
        }

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" VAT ")) {
        // https://softologyblog.wordpress.com/2017/05/27/voxel-automata-terrain/
        // https://bitbucket.org/BWerness/voxel-automata-terrain/src/master/

        // static ImVec4 color0 =
        //     ImVec4(165.0 / 255.0, 118.0 / 255.0, 64.0 / 255.0,
        //            10.0 / 255.0); // neutral volume color
        // static ImVec4 color1 =
        //     ImVec4(210.0 / 255.0, 180.0 / 255.0, 140.0 / 255.0,
        //            105.0 / 255.0); // Wikipedia Tan
        // static ImVec4 color2 =
        //     ImVec4(143.0 / 255.0, 151.0 / 255.0, 121.0 / 255.0,
        //            95.0 / 255.0); // Wikipedia Artichoke Green

        static ImVec4 color0 =
            ImVec4(200.0 / 255.0, 49.0 / 255.0, 11.0 / 255.0, 10.0 / 255.0);
        static ImVec4 color1 =
            ImVec4(207.0 / 255.0, 179.0 / 255.0, 7.0 / 255.0, 125.0 / 255.0);
        static ImVec4 color2 =
            ImVec4(190.0 / 255.0, 95.0 / 255.0, 0.0 / 255.0, 155.0 / 255.0);

        static float lambda = 0.35;
        static float beta = 0.5;
        static float mag = 0.0;

        static bool respect_mask = true;

        static int initmode = 3;
        static float flip;

        static char str0[256] = "";

        static bool plusx = false, plusy = false, plusz = false;
        static bool minusx = false, minusy = true, minusz = false;

        WrappedText(" Voxel Automata Terrain ");
        ImGui::SameLine();
        HelpMarker(
            "This is a very interesting noise algorithm from Brent Werness - "
            "if you know the code for a rule, you can enter it in the box "
            "below. If you do not, you can use the random parameters and then "
            "tell it to generate a random rule. The random rule is then stored "
            "in the same text box.\n\nThe remaining parameters influence "
            "seeding, which faces will be seeded with values, and what those "
            "values are. Symmetrical structures come from filling faces with "
            "the same value, while more chaotic structures come from random "
            "seeding. The flip parameter is used to randomly flip state while "
            "recursively evaluating the cube's voxels.");

        // string entry, letting the user input a rule

        OrangeText("SEEDING ");
        // mode slider (int)
        switch (initmode) {
        case 0:
          ImGui::Text(" Seed with state '0' ");
          break;
        case 1:
          ImGui::Text(" Seed with state '1' ");
          break;
        case 2:
          ImGui::Text(" Seed with state '2' ");
          break;
        case 3:
          ImGui::Text(" Random seeding ");
          break;
        default:
          break;
        }
        ImGui::SliderInt(" mode", &initmode, 0, 3);

        ImGui::Checkbox(" fill +x", &plusx);
        ImGui::SameLine();
        ImGui::Checkbox(" fill +y", &plusy);
        ImGui::SameLine();
        ImGui::Checkbox(" fill +z", &plusz);

        ImGui::Checkbox(" fill -x", &minusx);
        ImGui::SameLine();
        ImGui::Checkbox(" fill -y", &minusy);
        ImGui::SameLine();
        ImGui::Checkbox(" fill -z", &minusz);

        ImGui::Separator();
        OrangeText("EVALUATION ");
        ImGui::Checkbox(" respect mask ", &respect_mask);
        ImGui::SliderFloat(" flip chance", &flip, 0.0f, 1.0f, "%.3f");
        ImGui::SameLine();
        HelpMarker(
            "Make nonzero for stochastic result. Random flips occur with this "
            "probability, during the evaluation of the block.");

        // three colors
        ImGui::ColorEdit4(" State 0 Color", (float *)&color0,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::ColorEdit4(" State 1 Color", (float *)&color1,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::ColorEdit4(" State 2 Color", (float *)&color2,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        OrangeText("RULE ");

        ImGui::SameLine();
        HelpMarker(
            "Enter base62 encoded rule and hit Compute String, or mess with "
            "the random parameters (lambda for Random, beta and mag for "
            "IRandom) and hit Compute Random or Compute IRandom. The resulting "
            "base62 string is reported back to the text box if you find one "
            "you like and want to save it for later. See VAT_cool_rules.txt in "
            "the root directory of this repo for some that I've found.");

        ImGui::InputText(" rule", str0, IM_ARRAYSIZE(str0));
        ImGui::SetItemDefaultFocus();

        ImGui::Separator();
        ImGui::Separator();
        ImGui::SliderFloat(" lambda", &lambda, 0.0f, 1.0f, "%.3f");
        ImGui::Separator();
        ImGui::SliderFloat(" beta", &beta, 0.0f, 1.0f, "%.3f");
        ImGui::SliderFloat(" mag", &mag, 0.0f, 1.0f, "%.3f");

        if (ImGui::SmallButton(" Compute String ")) {
          // invoke the constructor, etc - return a string from the
          // OpenGL_container::vat(...), and put it in str0
          glm::vec4 col0, col1, col2;
          col0 = glm::vec4(color0.x, color0.y, color0.z, color0.w);
          col1 = glm::vec4(color1.x, color1.y, color1.z, color1.w);
          col2 = glm::vec4(color2.x, color2.y, color2.z, color2.w);

          std::string temp = GPU_Data.vat(
              flip, std::string(str0), initmode, col0, col1, col2, lambda, beta,
              mag, respect_mask, glm::bvec3(minusx, minusy, minusz),
              glm::bvec3(plusx, plusy, plusz)); // assign with the function call

          strcpy(str0,
                 temp.c_str()); // you get to see how the random rule you
                                // generated, or retain the rule you entered
        }

        ImGui::SameLine();

        if (ImGui::SmallButton(" Compute Random ")) {
          glm::vec4 col0, col1, col2;
          col0 = glm::vec4(color0.x, color0.y, color0.z, color0.w);
          col1 = glm::vec4(color1.x, color1.y, color1.z, color1.w);
          col2 = glm::vec4(color2.x, color2.y, color2.z, color2.w);

          std::string temp = GPU_Data.vat(
              flip, std::string("r"), initmode, col0, col1, col2, lambda, beta,
              mag, respect_mask, glm::bvec3(minusx, minusy, minusz),
              glm::bvec3(plusx, plusy, plusz));

          strcpy(str0, temp.c_str());
        }

        ImGui::SameLine();

        if (ImGui::SmallButton(" Compute IRandom ")) {
          glm::vec4 col0, col1, col2;
          col0 = glm::vec4(color0.x, color0.y, color0.z, color0.w);
          col1 = glm::vec4(color1.x, color1.y, color1.z, color1.w);
          col2 = glm::vec4(color2.x, color2.y, color2.z, color2.w);

          std::string temp = GPU_Data.vat(
              flip, std::string("i"), initmode, col0, col1, col2, lambda, beta,
              mag, respect_mask, glm::bvec3(minusx, minusy, minusz),
              glm::bvec3(plusx, plusy, plusz));

          strcpy(str0, temp.c_str());
        }

        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem(" Letters ")) {
			static int count = 0;
			static bool draw = true;
			static int mask = 0;
			static ImVec4 color1 = ImVec4(210.0 / 255.0, 180.0 / 255.0, 140.0 / 255.0, 105.0 / 255.0); // Wikipedia Tan

			ImGui::Text(" ");
			ImGui::SliderInt("Letter Count", &count, 0, 10000);
			ImGui::Text(" ");

			ImGui::Separator();

			OrangeText("OPTIONS");
			ImGui::Checkbox("  Draw ", &draw);
			ImGui::SameLine();
			ImGui::InputInt(" Mask ", &mask);

			// bounds check
			mask = std::clamp(mask, 0, 255);

			ImGui::ColorEdit4("  Color", (float *)&color1, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

			ImGui::Text(" ");
			ImGui::SetCursorPosX(16);

			if (ImGui::SmallButton(" Draw ")) {
				GPU_Data.letters(count, glm::vec4(color1.x, color1.y, color1.z, color1.w), draw, mask);
			}
			ImGui::EndTabItem();
	  	}

      ImGui::EndTabBar();
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem(" Utilities ")) {
      ImGui::BeginTabBar("utils", tab_bar_flags_wdropdown);
      if (ImGui::BeginTabItem(" Clear ")) {
        static bool respect_mask = false;

        WrappedText(" Clear ");
        ImGui::SameLine();
        HelpMarker("This will clear the block, with an option to respect the "
                   "mask. If you check that, masked cells won't be cleared.");

        OrangeText("SETTINGS");

        ImGui::Checkbox("  Respect mask ", &respect_mask);

        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Clear ")) {
          // do the clear all operation - note that this respects the mask
          // values
          GPU_Data.clear_all(respect_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Masking ")) {
        // WrappedText("This will clear the mask value for all cells.
        // Equivalently, set mask to false for all voxels. ", windowsize.x);

        WrappedText(" Masking ");
        ImGui::SameLine();
        HelpMarker(
            "Masking is a function which allows all or part of the "
            "block to be protected from subsequent draw calls. The "
            "value of the mask for each voxel tells how subsequent "
            "operations will affect its contents. A mask value of 0 indicates "
            "that subsequent draw operations will replace completely, while "
            "255 indicates that the raster data will be completely retained. "
            "\n\nUnmask all sets all cells' values to zero, invert sets them "
            "to 255 minus their current value. Mask by color works the same "
            "way but now with the addition of an 'amount' slider. Set a base "
            "value for each color channel or the lighting and a variance from "
            "that value. Selected channels will be used in the logic in the "
            "shader to determine cells to mask.");

        OrangeText("BASIC OPERATIONS");
        if (ImGui::SmallButton(" Unmask All ")) {
          // unmask all cells
          GPU_Data.unmask_all();
        }

        ImGui::SameLine();

        if (ImGui::SmallButton(" Invert Mask ")) {
          // do the toggle operation
          GPU_Data.invert_mask();
        }

        static bool use_r;
        static bool use_g;
        static bool use_b;
        static bool use_a;
        static bool use_l;

        static ImVec4 select_color;
        static float light_val = 0.0001;

        static float r_variance = 0.0;
        static float g_variance = 0.0;
        static float b_variance = 0.0;
        static float a_variance = 0.0;
        static float l_variance = 0.0;

        static int amt = 0;

        OrangeText("MASK BY COLOR");
        ImGui::SameLine();
        HelpMarker(
            "Use the HSV picker or the RGB fields to enter a color. Once you "
            "do that, use the check boxes and sliders to express how you want "
            "to use each channel.\n\nFor example, if I pick 255 in the red "
            "channel, check the red check box, and set the slider to a non "
            "zero value, you will be masking the parts of the image that have "
            "a high value in the red channel.\n\nThe slider sets how broadly "
            "this operation will be applied. Each channel's spread value tells "
            "how far above and below the specified value will be considered in "
            "this masking operation. \n\nThis can be applied to the RGBA color "
            "channels as well as the value in the lighting buffer, to mask "
            "only light or dark areas. ");

        ImGui::ColorEdit4("  Color", (float *)&select_color,
                          ImGuiColorEditFlags_AlphaBar |
                              ImGuiColorEditFlags_AlphaPreviewHalf);

        // sliders

        ImGui::Checkbox("use r", &use_r);
        ImGui::SameLine();
        ImGui::SliderFloat("r spread", &r_variance, 0.0f, 1.0f, "%.3f");

        ImGui::Checkbox("use g", &use_g);
        ImGui::SameLine();
        ImGui::SliderFloat("g spread", &g_variance, 0.0f, 1.0f, "%.3f");

        ImGui::Checkbox("use b", &use_b);
        ImGui::SameLine();
        ImGui::SliderFloat("b spread", &b_variance, 0.0f, 1.0f, "%.3f");

        ImGui::Checkbox("use a", &use_a);
        ImGui::SameLine();
        ImGui::SliderFloat("a spread", &a_variance, 0.0f, 1.0f, "%.3f");

        ImGui::Separator();

        ImGui::Checkbox("use l", &use_l);
        ImGui::SameLine();
        ImGui::SliderFloat("l value", &light_val, 0.0f, 1.0f, "%.3f");
        ImGui::SliderFloat("l spread", &l_variance, 0.0f, 1.0f, "%.3f");

        ImGui::InputInt(" Amount ", &amt);

        // bounds check
        amt = std::clamp(amt, 0, 255);

        if (ImGui::SmallButton(" Mask by Color")) {
          GPU_Data.mask_by_color(use_r, use_g, use_b, use_a, use_l,
                                 glm::vec4(select_color.x, select_color.y,
                                           select_color.z, select_color.w),
                                 light_val, r_variance, g_variance, b_variance,
                                 a_variance, l_variance, amt);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Blur ")) {
        static int blur_radius = 0;
        static bool touch_alpha = true;
        static bool respect_mask = false;

        WrappedText(" Blur ");
        ImGui::SameLine();
        HelpMarker(
            "This to invoke either a simple box blur or a fancier Gaussian "
            "blur. Either algorithm will consider the size neighborhood you "
            "select, and average the colors to give smoother transitions "
            "beteen neighboring cells.\n\nIf you use 'respect mask' on masked "
            "cells with high alpha, the colors will begin to diffuse outwards "
            "like an aura, around the sharper masked shapes.\n\nAlso note that "
            "as the neighborhood of the blur increases, more texture reads are "
            "required in order to compute the result of the kernel. Please "
            "keep this in mind as this number of memory accesses becomes "
            "large, it can become prohibitively slow on some hardware without "
            "dedicated graphics memory.");

        OrangeText("SETTINGS");
        ImGui::Checkbox("  Touch alpha ", &touch_alpha);
        ImGui::SameLine();
        ImGui::Checkbox("  Respect mask ", &respect_mask);
        ImGui::SliderInt(" Radius", &blur_radius, 0, 5);

        // do the blur operation with the selected values
        if (ImGui::SmallButton(" Box Blur ")) {
          GPU_Data.box_blur(blur_radius, touch_alpha, respect_mask);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(" Gaussian Blur ")) {
          GPU_Data.gaussian_blur(blur_radius, touch_alpha, respect_mask);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Limiter ")) {
        if (ImGui::SmallButton(" Limit ")) {
          // GPU_Data.limiter();
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Shift ")) {
        static int xmove;
        static int ymove;
        static int zmove;
        static bool loop = false;
        static int shift_mode = 1;

        WrappedText(" Shift ");
        ImGui::SameLine();
        HelpMarker(
            "This allows you to shift the voxel data by some amount "
            "along the x, y, and z axes. This can be a very interesting way to "
            "look at the inside of a volume model. Modes of operation are "
            "described below.\n\nIf you turn on looping, data that goes off "
            "one side will appear on the opposite side, torus-style.");

        OrangeText("MOVEMENT AMOUNT");
        ImGui::SliderInt(" x", &xmove, -DIM, DIM);
        ImGui::SliderInt(" y", &ymove, -DIM, DIM);
        ImGui::SliderInt(" z", &zmove, -DIM, DIM);

        OrangeText("SETTINGS");
        ImGui::Checkbox(" Loop Edges", &loop);
        ImGui::SliderInt(" Mode", &shift_mode, 1, 3);

        switch (shift_mode) {
        case 1:
          WrappedText("Mode 1: Ignore mask buffer, move color data only");
          break;
        case 2:
          WrappedText("Mode 2: Respect mask buffer, masked cells retain color");
          break;
        case 3:
          WrappedText("Mode 3: Carry mask buffer, both mask and color move");
          break;
        default:
          ImGui::Text("Pick a valid mode");
          break;
        }

        if (ImGui::SmallButton(" Shift ")) {
          GPU_Data.shift(glm::ivec3(xmove, ymove, zmove), loop, shift_mode);
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Log ")) {

        // show the log

        // button to clear the log
        if (ImGui::SmallButton(" Clear Log ")) {
          GPU_Data.clear_log();
        }

        // button to save log to file
        // > add text entry for filename
        if (ImGui::SmallButton(" Save Log ")) {
          GPU_Data.save_log("log.txt");
        }

        // button to load a log from file
        // > add text entry for filename
        // if (ImGui::SmallButton()) {
        // GPU_Data.load()
        // }

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Copy/Paste ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Load/Save ")) {
        static char str0[256] = "";

        const char *listbox_items[LISTBOX_SIZE];

        // count up the number of non-null c-strings
        unsigned int i;
        for (i = 0; i < LISTBOX_SIZE && i < directory_strings.size(); ++i) {
          listbox_items[i] = directory_strings[i].c_str();
        }

        static bool respect_mask_on_load = false;

        WrappedText(" Load/Save ");
        ImGui::SameLine();
        HelpMarker(
            "This function lets you load or save of blocks using the PNG image "
            "format. Slices of the block are enumerated out one after the "
            "other to create a very tall image which contains a lossless copy "
            "of all the volume data.\n\nSelect one from the list to load, or "
            "save one you have created using the provided text entry "
            "field. Respecting mask on load means that the load operation will "
            "be blended with existing data based on the current blend "
            "mode.\n\nPlease note that if you have lighting applied to your "
            "model, you need to use the 'mash' function under the lighting tab "
            "in order to destructively combine the RGBA and Lighting values. "
            "Once it has been applied and the lighting buffer is cleared back "
            "to neutral levels, what you see is what you get in terms of what "
            "will be saved to disk.");

        OrangeText("FILES IN SAVES FOLDER");
        static int listbox_select_index = 1;
        ImGui::ListBox(" ", &listbox_select_index, listbox_items, i, 18);

        OrangeText("ENTER FILENAME TO SAVE");
        ImGui::InputTextWithHint(".png", "", str0, IM_ARRAYSIZE(str0));
        ImGui::SameLine();
        // HelpMarker("(?)", "USER:\nHold SHIFT or use mouse to select text.\n"
        //                   "CTRL+Left/Right to word jump.\n"
        //                   "CTRL+A or double-click to select all.\n"
        //                   "CTRL+X,CTRL+C,CTRL+V clipboard.\n"
        //                   "CTRL+Z,CTRL+Y undo/redo.\n"
        //                   "ESCAPE to revert.");

        ImGui::Text(" ");
        ImGui::Checkbox("  Respect mask on load", &respect_mask_on_load);

        ImGui::SetCursorPosX(16);

        if (ImGui::SmallButton(" Load ")) {
          // load that image
          GPU_Data.load(directory_strings[listbox_select_index],
                        respect_mask_on_load);
        }

        ImGui::SameLine();

        if (ImGui::SmallButton(" Save ")) {
          if (hasPNG(std::string(str0))) {
            GPU_Data.save(std::string(str0));
          } else {
            GPU_Data.save(std::string(str0) + std::string(".png"));
          }

          update_listbox_items();
        }
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(" Lighting ")) {
      ImGui::BeginTabBar("l", tab_bar_flags_wdropdown);

      // static float clear_level = 0.25f;
      static bool use_cache;

      static float directional_theta;
      static float directional_phi;
      static float decay_power = 2.0f;

      static int AO_radius = 0;

      static float GI_scale_factor = 0.028;
      static float GI_alpha_thresh = 0.105;

      static glm::vec3 point_light_position = glm::vec3(0, 0, 0);
      static float point_decay_power = 2.1;
      static float point_distance_power = 2.0;

      static glm::vec3 cone_light_position = glm::vec3(0, 0, 0);
      static float cone_theta = 0;
      static float cone_phi = 0;
      static float cone_angle = 0;
      static float cone_decay_power = 0;
      static float cone_distance_power = 0;

      if (ImGui::BeginTabItem(" Clear ")) {

        WrappedText(" Clear Level ");
        ImGui::SameLine();
        HelpMarker(
            "This clears the values kept in each voxel of the lighting "
            "buffer. If you have a cached lighting volume, you can "
            "clear existing lighting information to those levels by "
            "checking the box. Otherwise, you will be using the value "
            "in the slider for all voxels.\n\n0.25 is 'neutral' in the "
            "lighting buffer, that is, it faithfully passes color through from "
            "the RGBA data before the tonemapping etc is applied.");

        OrangeText("SETTINGS");

        static glm::vec4 color0 = glm::vec4(0, 0, 0, 1);
        ImGui::ColorEdit3(" Color", (float *)&color0);
        ImGui::SliderFloat("intensity scale    ", &color0.w, 0.0, 5.0);

        ImGui::Checkbox(" Clear to Cached Levels ", &use_cache);

        if (ImGui::SmallButton(" Clear ")) {
          GPU_Data.lighting_clear(use_cache, color0);
        }

        ImGui::Separator();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem(" Direct ")) {

        WrappedText(" Point Light ");
        ImGui::SameLine();
        HelpMarker(
            "Point lights are now invoked per voxel. The ray from the light "
            "position to the voxel's position is considered, and the alpha "
            "value of samples along the ray will attenuate it to create "
            "shadows. Once the location of the voxel is reached, an "
            "approximation of the inverse square law is used to give more "
            "depth cues. The value of the attenuated light is then scaled by a "
            "fraction of one over the distance raised to distance power (to "
            "approximate inverse square at the default value of 2.0). ");

        OrangeText("POSITION");
        ImGui::SliderFloat("x", &point_light_position.x, -100, DIM + 100,
                           "%.3f");
        ImGui::SliderFloat("y", &point_light_position.y, -100, DIM + 100,
                           "%.3f");
        ImGui::SliderFloat("z", &point_light_position.z, -100, DIM + 100,
                           "%.3f");
        OrangeText("PARAMETERS");
        // ImGui::SliderFloat("value", &point_intensity, 0, 1.0, "%.3f");
        ImGui::SliderFloat("decay  ", &point_decay_power, 0, 3.0, "%.3f");
        ImGui::SliderFloat("dist power", &point_distance_power, 0, 3.0f,
                           "%.3f");

        OrangeText("COLOR");

        static glm::vec4 colorp = glm::vec4(0, 0, 0, 1);
        ImGui::ColorEdit3(" ", (float *)&colorp);
        ImGui::SliderFloat("intensity scale   ", &colorp.w, 0.0, 5.0);

        if (ImGui::SmallButton(" Point Light ")) {
          GPU_Data.compute_point_lighting(point_light_position, colorp,
                                          point_decay_power,
                                          point_distance_power);
        }

        ImGui::Text("");
        ImGui::Text("");
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("");
        ImGui::Text("");

        WrappedText(" Cone Lights ");
        ImGui::SameLine();
        HelpMarker("Cone lights are similar to point lights, but constrained "
                   "to a solid angle about the direction vector. The falloff "
                   "can also be controlled, to soften the edges of the cone.");

        OrangeText("SOURCE LOCATION");
        ImGui::SliderFloat("x ", &cone_light_position.x, -100, DIM + 100,
                           "%.3f");
        ImGui::SliderFloat("y ", &cone_light_position.y, -100, DIM + 100,
                           "%.3f");
        ImGui::SliderFloat("z ", &cone_light_position.z, -100, DIM + 100,
                           "%.3f");
        OrangeText("CONE ROTATION");
        ImGui::SliderFloat("theta ", &cone_theta, -3.14f, 3.14f, "%.3f");
        ImGui::SliderFloat("phi ", &cone_phi, -3.14f, 3.14f, "%.3f");

        OrangeText("PARAMETERS");
        ImGui::SameLine();
        HelpMarker("Cone angle sets the width of the cone.\n\nValue determines "
                   "the initial intensity of the light source.\n\nDecay "
                   "determines the falloff of how the light intensity is "
                   "attenuated by the alpha samples of the volume.\n\nDist "
                   "power is used as an approximation of the inverse square "
                   "law, similar to the point lights.");

        ImGui::SliderFloat("cone angle", &cone_angle, -3.14f, 3.14f, "%.3f");
        // something about falloff (sharp vs gradual)
        // ImGui::SliderFloat("value", &cone_intensity, 0, 1.0, "%.3f");
        ImGui::SliderFloat("decay ", &cone_decay_power, 0, 3.0, "%.3f");
        ImGui::SliderFloat("dist power ", &cone_distance_power, 0, 3.0f,
                           "%.3f");

        OrangeText("COLOR");

        static glm::vec4 colorc = glm::vec4(0, 0, 0, 1);
        ImGui::ColorEdit3("  ", (float *)&colorc);
        ImGui::SliderFloat("intensity scale  ", &colorc.w, 0.0, 5.0);

        if (ImGui::SmallButton(" Cone Light ")) {
          // the call to the cone light function
        }

        ImGui::Text("");
        ImGui::Text("");
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("");
        ImGui::Text("");

        WrappedText(" Directional Lights ");
        ImGui::SameLine();
        HelpMarker(
            "Directional lights are like point lights at an infinite distance. "
            "There is no divergence of the light rays, so all of them are "
            "parallel. This is achieved by using a uniform vector for the "
            "light ray traversal for all voxels. The angles set the rotation "
            "about the vertical and above the horizon, respectively, and the "
            "value and decay settings work very much the same as they do in "
            "the point and cone lighting functions to model attenuation by "
            "voxels with a nonzero alpha value.");

        OrangeText("DIRECTION");
        ImGui::SliderFloat("theta", &directional_theta, -3.14f, 3.14f, "%.3f");
        ImGui::SliderFloat("phi", &directional_phi, -3.14f, 3.14f, "%.3f");
        OrangeText("PARAMETERS");
        ImGui::SliderFloat("decay", &decay_power, 0.0f, 3.0f, "%.3f");
        OrangeText("COLOR");

        static glm::vec4 colord = glm::vec4(0, 0, 0, 1);
        ImGui::ColorEdit3("    ", (float *)&colord);
        ImGui::SliderFloat("intensity scale ", &colord.w, 0.0, 5.0);

        if (ImGui::SmallButton(" Directional ")) {
          GPU_Data.compute_new_directional_lighting(
              directional_theta, directional_phi, colord, decay_power);
        }

        ImGui::Separator();
        ImGui::Separator();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem(" Fake GI ")) {
        WrappedText(" Fake Global Illumination ");
        ImGui::SameLine();
        HelpMarker(
            "The algorithm comes from the original Voxel Automata "
            "Terrain project, and has been adapted from the sequential "
            "processing implementation to a compute shader that "
            "operates in 2D slices down the y-axis.\n\nFor each voxel in a "
            "slice under consideration, which is over the alpha threshold, 9 "
            "rays are considered travelling up from the voxel's location. They "
            "are generated by looking at the vector between the voxel's center "
            "and the center of the voxels in a radius-1 neighborhood in the "
            "layer above. \n\nThese rays are stepped through the volume, till "
            "one of two things happens - if they don't hit a voxel above the "
            "alpha threshold before escaping the volume, that ray contributes "
            "'sky_intensity' to the averaging of these 9 rays. If, on the "
            "other hand, it hits a voxel above this alpha threshold, some "
            "portion of the lighting value at that location, determined by the "
            "sfactor parameter, will be the contribution to the averaging.");

        OrangeText("PARAMETERS");
        ImGui::SliderFloat("sfactor", &GI_scale_factor, 0.0f, 1.0f);
        ImGui::SliderFloat("alpha threshold", &GI_alpha_thresh, 0.0f, 1.0f);
        // ImGui::SliderFloat("sky intensity", &GI_sky_intensity, 0.0f, 1.0f);
        static glm::vec4 color0 =
            glm::vec4(40. / 255., 40. / 255., 40. / 255., 1.);
        ImGui::ColorEdit3("sky color", (float *)&color0);
        ImGui::SliderFloat("intensity scale", &color0.w, 0.0, 5.0);

        if (ImGui::SmallButton(" Apply GI ")) {
          GPU_Data.compute_fake_GI(GI_scale_factor, color0, GI_alpha_thresh);
        }
        ImGui::Separator();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem(" AO ")) {
        WrappedText(" Ambient Occlusion ");
        ImGui::SameLine();
        HelpMarker(
            "Ambient occlusion is based on a weighted average of the alpha "
            "values in the specified size neighborhood. Generally speaking it "
            "will only dim your existing ligting values. Interesting results "
            "with zero radius, maybe due to divide by zero?\n\nNote that large "
            "neighborhoods incur a lot of texture reads and will thus become "
            "increasingly slow.");

        OrangeText("SETTINGS");
        ImGui::SliderInt("radius", &AO_radius, 0, 5);

        if (ImGui::SmallButton(" Apply AO ")) {
          GPU_Data.compute_ambient_occlusion(AO_radius);
        }

        ImGui::Separator();
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem(" Mash ")) {
        WrappedText(" Mash ");
        ImGui::SameLine();
        HelpMarker("Mash combines the lighting buffer and the color buffer, so "
                   "that the block can be saved with the lighting applied. By "
                   "playing with feedback effects, I have found this can have "
                   "creative applicaitons as well.");

        OrangeText("SETTINGS");
        // checkbox to tell if you want to clear to neutral level in the buffer
        static bool clear = true;
        ImGui::Checkbox(" Reset Buffer Values on Mash", &clear);
        ImGui::SameLine();
        HelpMarker(
            "Checking this box prevents feedback from clicking 'mash' again. "
            "With it unchecked, you can continue to scale existing color "
            "values in the RGBA buffer until they clip.");
        if (ImGui::SmallButton(" Mash ")) {
          GPU_Data.mash();
          if (clear) {
            GPU_Data.lighting_clear(false, glm::vec4(1.));
          }
        }

        ImGui::Separator();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(
            " Settings ")) // just going to have everything on one page here, no
                           // real point breaking it out into tabs
    {

      static ImVec4 clear_color(GPU_Data.clear_color.x, GPU_Data.clear_color.y,
                                GPU_Data.clear_color.z, GPU_Data.clear_color.w);

      OrangeText("APPLICATION SETTINGS");

      ImGui::ColorEdit3(
          "BG Color",
          (float *)&clear_color); // Edit 3 floats representing a color

      GPU_Data.clear_color =
          glm::vec4(clear_color.x, clear_color.y, clear_color.z, 1.0);
      ImGui::SameLine();
      HelpMarker(" OpenGL Clear Color ");
      ImGui::Separator();

      OrangeText("RENDERING ADJUSTMENTS");
      ImGui::SliderFloat("alpha correction power",
                         &GPU_Data.alpha_correction_power, 0.5, 4.0);

      ImGui::SliderFloat("gamma", &GPU_Data.gamma_correction, 0.5, 4.0);

      ImGui::SliderInt("color temp", &GPU_Data.color_temp, 1000, 45000);

      // dither toggle - need to add the logic about blue noise dither, then give a way to pick between them
      // ImGui::Checkbox("dither", &GPU_Data.dither);

      ImGui::Text("");

      // DROPDOWN FOR THE TONEMAP MODE
      // code from:
      //  https://www.shadertoy.com/view/4dBcD1 - todo: finish copying the rest of these
      //  https://www.shadertoy.com/view/WdjSW3
      //  https://www.shadertoy.com/view/ldlcWX
      //  https://64.github.io/tonemapping/
      //  http://filmicworlds.com/blog/filmic-tonemapping-operators/

      const char* tmodes[] = {"None (Linear)", "ACES (Narkowicz 2015)", "Unreal Engine 3", "Unreal Engine 4",
      "Uncharted 2", "Gran Turismo", "Modified Gran Turismo", "Rienhard", "Modified Rienhard", "jt", "robobo1221s",
      "robo", "reinhardRobo", "jodieRobo", "jodieRobo2", "jodieReinhard", "jodieReinhard2"};

      static int current_tmode = GPU_Data.tonemap_mode;
      static int prev_frame_tmode;

      ImGui::Combo("Tonemapping Mode", &current_tmode, tmodes, IM_ARRAYSIZE(tmodes));

      if(current_tmode != prev_frame_tmode) // if selection changed
      {
        GPU_Data.tonemap_mode = current_tmode; // pass to GL container
        prev_frame_tmode = current_tmode;
      }

      ImGui::SameLine();
      HelpMarker("Adjust Gamma factor if too dark/too bright.");

      // DROPDOWN FOR THE RENDERER SWITCHER
      const char* rmodes[] = {"Image3D", "Image3D (Supercover)", "Texture3D (Nearest)", "Texture3D (Mipmapped Linear)", "Depth Visualization", "Position Visualization"};
      static int current_rmode = 3;
      static int prev_frame_rmode;

      ImGui::Combo("Render Mode", &current_rmode, rmodes, IM_ARRAYSIZE(rmodes));

      if(current_rmode != prev_frame_rmode)
      { // if it changed, invoke desired behavior
        switch(current_rmode)
        {
          case 0: // Image3D
            GPU_Data.main_block_image();
            break;
          case 1: // Image3D (Supercover)
            // not yet implemented
            break;
          case 2: // Texture3D (Nearest)
            GPU_Data.main_block_nearest_filter();
            break;
          case 3: // Texture3D (Mipmapped Linear)
            GPU_Data.main_block_linear_filter();
            break;
          case 4: // Depth Visualization
            GPU_Data.main_block_depth();
            break;
          case 5: // Position Visualization
            GPU_Data.main_block_position();
            break;
          default:
            break;
        }

        prev_frame_rmode = current_rmode;
      }

      if (ImGui::SmallButton(" Swap Blocks ")) {
        GPU_Data.swap_blocks();
        GPU_Data.set_redraw_flag();
      }

      if(ImGui::SmallButton(" Dither Bayer ")) {
      	GPU_Data.dither_bayer();
      	GPU_Data.set_redraw_flag();
      }

      if(ImGui::SmallButton(" Dither Blue ")) {
      	GPU_Data.dither_blue();
      	GPU_Data.set_redraw_flag();
      }



      ImGui::SameLine();
      HelpMarker("Swapping blocks will undo when used once, and redo when used "
                 "again. It toggles front and back buffers, so it can't do "
                 "more than one step of undo.");

      OrangeText("ORIENTATION WIDGET");
      // x and y adjustment
      ImGui::SliderFloat("offset x", &GPU_Data.orientation_widget_offset.x, -1,
                         1);
      ImGui::SliderFloat("offset y", &GPU_Data.orientation_widget_offset.y, -1,
                         1);

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
  }
}

void engine::draw_user_editor_tab_contents() {
  // assumes an already open window
  // locally declared class, static instance held to keep the info
  struct consoleclass {
    char InputBuf[256];
    ImVector<char *> Items;
    ImVector<const char *> Commands;
    ImVector<char *> History;
    int HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
    engine *parent = NULL;
    ImGuiTextFilter Filter;
    bool AutoScroll;
    bool ScrollToBottom;
    TextEditor editor;

    char text[1 << 18];
    char origtext[265] =
        "irec is_inside(){  // check Documentation tab for details \n\n"
        " irec temp;\n\n"
        " temp.draw  = false;   // is this voxel's value going to change?\n"
        " temp.color = vec4(0); // what should its color be?\n"
        " temp.mask  = 0;       // how much do you wish to mask?\n\n"
        " return temp;\n\n"
        "}";

    consoleclass() {
      ClearLog();
      memset(InputBuf, 0, sizeof(InputBuf));
      HistoryPos = -1;

      strcpy(text, origtext);

      Commands.push_back("help"); // dump command list
      Commands.push_back("man");  // list all function names
      Commands.push_back("compile");
      Commands.push_back("list"); // list of saves in scripts/
      Commands.push_back("load");
      Commands.push_back("save");
      Commands.push_back("history");

      AutoScroll = true;
      ScrollToBottom = true;
    }
    ~consoleclass() {
      ClearLog();
      for (int i = 0; i < History.Size; i++)
        free(History[i]);
    }

    // Portable helpers
    static int Stricmp(const char *s1, const char *s2) {
      int d;
      while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
        s1++;
        s2++;
      }
      return d;
    }
    static int Strnicmp(const char *s1, const char *s2, int n) {
      int d = 0;
      while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
        s1++;
        s2++;
        n--;
      }
      return d;
    }
    static char *Strdup(const char *s) {
      size_t len = strlen(s) + 1;
      void *buf = malloc(len);
      IM_ASSERT(buf);
      return (char *)memcpy(buf, (const void *)s, len);
    }

    static void Strtrim(char *s) {
      char *str_end = s + strlen(s);
      while (str_end > s && str_end[-1] == ' ')
        str_end--;
      *str_end = 0;
    }

    void ClearLog() {
      for (int i = 0; i < Items.Size; i++)
        free(Items[i]);
      Items.clear();
      AddLog(std::string(
                 current_time_and_date() +
                 std::string("Welcome to the Voraldo v1.2 User Shader Console. "
                             "\n'help' for command list."))
                 .c_str());
    }

    void AddLog(const char *fmt, ...) IM_FMTARGS(2) {
      // FIXME-OPT
      char buf[1024];
      va_list args;
      va_start(args, fmt);
      vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
      buf[IM_ARRAYSIZE(buf) - 1] = 0;
      va_end(args);
      Items.push_back(Strdup(buf));
    }

    void Draw(const char *title, bool *p_open) {

      ImGui::SameLine();
      if (ImGui::SmallButton(" Clear Console ")) {
        ClearLog();
      }

      ImGui::Separator();

      // Reserve enough left-over height for 1 separator + 1 input text
      ImGui::PushItemWidth(ImGui::GetWindowWidth());

      const float footer_height_to_reserve =
          ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() +
          8;
      ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve),
                        false, ImGuiWindowFlags_HorizontalScrollbar);

      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                          ImVec2(4, 1)); // Tighten spacing

      for (int i = 0; i < Items.Size; i++) {
        const char *item = Items[i];
        if (!Filter.PassFilter(item))
          continue;

        // Normally you would store more information in your item than just a
        // string. (e.g. make Items[] an array of structure, store color/type
        // etc.)
        ImVec4 color;
        bool has_color = false;
        if (strstr(item, "[error]")) {
          color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
          has_color = true;
        } else if (strncmp(item, "> ", 2) == 0) {
          color = ImVec4(1.0f, 0.618f, 0.218f, 1.0f);
          has_color = true;
        }
        if (has_color)
          ImGui::PushStyleColor(ImGuiCol_Text, color);
        // ImGui::TextUnformatted(item);
        WrappedText(item);
        if (has_color)
          ImGui::PopStyleColor();
      }

      if (ScrollToBottom ||
          (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
        ImGui::SetScrollHereY(1.0f);
      ScrollToBottom = false;

      ImGui::PopStyleVar();
      ImGui::EndChild();
      ImGui::Separator();

      // Command-line
      bool reclaim_focus = false;
      ImGuiInputTextFlags input_text_flags =
          ImGuiInputTextFlags_EnterReturnsTrue |
          ImGuiInputTextFlags_CallbackCompletion |
          ImGuiInputTextFlags_CallbackHistory;

      // ImGui::PushItemWidth(ImGui::GetWindowWidth());

      if (ImGui::InputText(" ", InputBuf, IM_ARRAYSIZE(InputBuf),
                           input_text_flags, &TextEditCallbackStub,
                           (void *)this)) {
        char *s = InputBuf;
        Strtrim(s);
        if (s[0])
          ExecCommand(s);
        strcpy(s, "");
        reclaim_focus = true;
      }

      ImGui::PopItemWidth();

      // Auto-focus on window apparition
      ImGui::SetItemDefaultFocus();
      if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
    }

    void ExecCommand(const char *command_line) {
      AddLog("> %s\n", command_line);

      // Insert into history. First find match and delete it so it can be pushed
      // to the back. This isn't trying to be smart or optimal.
      HistoryPos = -1;
      for (int i = History.Size - 1; i >= 0; i--)
        if (Stricmp(History[i], command_line) == 0) {
          free(History[i]);
          History.erase(History.begin() + i);
          break;
        }
      History.push_back(Strdup(command_line));

      // Process command
      if (Stricmp(command_line, "clear") == 0) {
        ClearLog();
      } else if (Stricmp(command_line, "help") == 0) {
        AddLog("Commands:");
        for (int i = 0; i < Commands.Size; i++)
          AddLog("- %s", Commands[i]);
      } else if (Stricmp(command_line, "history") == 0) {
        int first = History.Size - 10;
        for (int i = first > 0 ? first : 0; i < History.Size; i++)
          AddLog("%3d: %s\n", i, History[i]);
      } else if (Strnicmp(command_line, "man", 3) == 0) {
        // AddLog("started with man\n");
        if (Stricmp(command_line, "man") == 0) { // if it is only 'man'
          AddLog("Use this command to access the manual entry for other "
                 "commands.\n'man list' for the list. \n");
        } else if (Strnicmp(command_line, "man ", 4) ==
                   0) { // properly formatted command
          // handle individual manual entries
          // probably easiest to use a std::map<std::string, std::string>
          // also have to still decide what goes in this header
        }
      } else if (Strnicmp(command_line, "load ", 5) == 0) {
        // try to load the string that follows 'load '
        std::ifstream file("scripts/" + std::string(command_line + 5));
        std::string loaded{std::istreambuf_iterator<char>(file),
                           std::istreambuf_iterator<char>()};
        editor.SetText(loaded);
      } else if (Strnicmp(command_line, "save ", 5) == 0) {
        // try to save the string to file
        std::ofstream file("scripts/" + std::string(command_line + 5));
        std::string savetext(editor.GetText());
        file << savetext;
      } else if (Stricmp(command_line, "list") == 0) {
        // list out all the files in scripts/
        struct path_leaf_string {
          std::string
          operator()(const std::filesystem::directory_entry &entry) const {
            return entry.path().string();
          }
        };

        std::vector<std::string> directory_strings;
        directory_strings.clear();

        std::filesystem::path p("scripts/");
        std::filesystem::directory_iterator start(p);
        std::filesystem::directory_iterator end;

        std::transform(start, end, std::back_inserter(directory_strings),
                       path_leaf_string());

        // sort these alphabetically
        std::sort(directory_strings.begin(), directory_strings.end());
        for (auto i : directory_strings) {
          AddLog("  %s\n", i.c_str());
        }
      } else if (Stricmp(command_line, "compile") == 0) {
        // compile what's in the box
        AddLog("%s\n",
               parent->GPU_Data.compile_user_script(std::string(text)).c_str());
      } else {
        AddLog("'%s' not found.\n", command_line);
      }

      // On command input, we scroll to bottom even if AutoScroll==false
      ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding
    // callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData *data) {
      consoleclass *console = (consoleclass *)data->UserData;
      return console->TextEditCallback(data);
    }

    int TextEditCallback(ImGuiInputTextCallbackData *data) {
      // AddLog("cursor: %d, selection: %d-%d", data->CursorPos,
      // data->SelectionStart, data->SelectionEnd);
      switch (data->EventFlag) {
      case ImGuiInputTextFlags_CallbackCompletion: {
        // Example of TEXT COMPLETION

        // Locate beginning of current word
        const char *word_end = data->Buf + data->CursorPos;
        const char *word_start = word_end;
        while (word_start > data->Buf) {
          const char c = word_start[-1];
          if (c == ' ' || c == '\t' || c == ',' || c == ';')
            break;
          word_start--;
        }

        // Build a list of candidates
        ImVector<const char *> candidates;
        for (int i = 0; i < Commands.Size; i++)
          if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) ==
              0)
            candidates.push_back(Commands[i]);

        if (candidates.Size == 0) {
          // No match
          AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start),
                 word_start);
        } else if (candidates.Size == 1) {
          // Single match. Delete the beginning of the word and replace it
          // entirely so we've got nice casing.
          data->DeleteChars((int)(word_start - data->Buf),
                            (int)(word_end - word_start));
          data->InsertChars(data->CursorPos, candidates[0]);
          data->InsertChars(data->CursorPos, " ");
        } else {
          // Multiple matches. Complete as much as we can..
          // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and
          // "CLASSIFY" as matches.
          int match_len = (int)(word_end - word_start);
          for (;;) {
            int c = 0;
            bool all_candidates_matches = true;
            for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
              if (i == 0)
                c = toupper(candidates[i][match_len]);
              else if (c == 0 || c != toupper(candidates[i][match_len]))
                all_candidates_matches = false;
            if (!all_candidates_matches)
              break;
            match_len++;
          }

          if (match_len > 0) {
            data->DeleteChars((int)(word_start - data->Buf),
                              (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, candidates[0],
                              candidates[0] + match_len);
          }

          // List matches
          AddLog("Possible matches:\n");
          for (int i = 0; i < candidates.Size; i++)
            AddLog("- %s\n", candidates[i]);
        }

        break;
      }
      case ImGuiInputTextFlags_CallbackHistory: {
        // Example of HISTORY
        const int prev_history_pos = HistoryPos;
        if (data->EventKey == ImGuiKey_UpArrow) {
          if (HistoryPos == -1)
            HistoryPos = History.Size - 1;
          else if (HistoryPos > 0)
            HistoryPos--;
        } else if (data->EventKey == ImGuiKey_DownArrow) {
          if (HistoryPos != -1)
            if (++HistoryPos >= History.Size)
              HistoryPos = -1;
        }

        // A better implementation would preserve the data on the current input
        // line along with cursor position.
        if (prev_history_pos != HistoryPos) {
          const char *history_str =
              (HistoryPos >= 0) ? History[HistoryPos] : "";
          data->DeleteChars(0, data->BufTextLen);
          data->InsertChars(0, history_str);
        }
      }
      }
      return 0;
    }
  };

  static bool draw = true;
  static consoleclass console;

  // first time init
  if (console.parent == NULL) {
    // console init
    console.parent = this;

    // text editor
    console.editor.SetLanguageDefinition(
        TextEditor::LanguageDefinition::GLSL());

    console.editor.SetPalette(TextEditor::GetDarkPalette());
    // editor.SetPalette(TextEditor::GetLightPalette());
    // editor.SetPalette(TextEditor::GetRetroBluePalette());

    console.editor.SetText(std::string(console.origtext));
  }

  // the first part, the editor -
  // this c style string holds the contents of the program -

  // ImGui::InputTextMultiline(
  //     "source", console.text, IM_ARRAYSIZE(console.text),
  //     ImVec2(-FLT_MIN, 2 * total_screen_height / 3), // 2/3 of screen height
  //     // ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 36), // 36 lines
  //     ImGuiInputTextFlags_AllowTabInput);

  auto cpos = console.editor.GetCursorPosition();
  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1,
              cpos.mColumn + 1, console.editor.GetTotalLines(),
              console.editor.IsOverwrite() ? "Ovr" : "Ins",
              console.editor.CanUndo() ? "*" : " ",
              console.editor.GetLanguageDefinition().mName.c_str(),
              "User Script");

  console.editor.Render("TextEditor",
                        ImVec2(-FLT_MIN, 2 * total_screen_height / 3));

  if (ImGui::SmallButton(" Compile and Run ")) {
    // do some compilation
    // report compilation result / errors / timing
    console.AddLog(
        "%s\n",
        GPU_Data.compile_user_script(std::string(console.editor.GetText()))
            .c_str());

    // run the shader for every voxel and report timing
    console.AddLog("%s\n", GPU_Data.run_user_script().c_str());
  }

  ImGui::SameLine();
  if (ImGui::SmallButton(" Clear Editor ")) {
    // strcpy(console.text, console.origtext);
    console.editor.SetText(std::string(console.origtext));
  }

  console.Draw("ex", &draw);
}

void engine::update_fps_history() {
  // push back - put in the new value
  fps_history.push_back(ImGui::GetIO().Framerate);
  // pop front - take out the oldest value
  fps_history.pop_front();
}

// small overlay to show the FPS counter, FPS graph
void engine::fps_overlay(bool *p_open) {
  if (*p_open) {
    const float DISTANCE = 0.2f;
    static int corner = 3;
    ImGuiIO &io = ImGui::GetIO();
    if (corner != -1) {
      ImVec2 window_pos =
          ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE - 80 : DISTANCE,
                 (corner & 2) ? io.DisplaySize.y - DISTANCE - 10 : DISTANCE);
      ImVec2 window_pos_pivot =
          ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
      ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.03f); // Transparent background
    if (ImGui::Begin("Example: Simple overlay", p_open,
                     (corner != -1 ? ImGuiWindowFlags_NoMove : 0) |
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoNav)) {
      // fps graph
      static float values[FPS_HISTORY_SIZE] = {};
      float average = 0;

      for (int n = 0; n < FPS_HISTORY_SIZE; n++) {
        values[n] = fps_history[n];
        average += fps_history[n];
      }

      average /= FPS_HISTORY_SIZE;
      char overlay[32];
      sprintf(overlay, "avg %.2f fps (%.2f ms)", average, 1000.0f / average);
      ImGui::PlotLines("", values, IM_ARRAYSIZE(values), 0, overlay, 0.0f,
                       140.0f, ImVec2(200, 45));
    }
    ImGui::End();
  }
}

void engine::orientation_widget_imgui() {

  static struct utils {
    bool viewDirty = false;
    bool useWindow = false;
    bool isPerspective = false;
    int gizmoCount = 1;
    float camDistance = 8.f;
    int lastUsing = 0;

    float fov = 27.f;
    float viewWidth = 10.f; // for orthographic
    float camYAngle = 165.f / 180.f * 3.14159f;
    float camXAngle = 32.f / 180.f * 3.14159f;

    ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;

    // initialize with identity
    float cameraView[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                            0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};

    float objectMatrix[4][16] = {{1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                  0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},

                                 {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                  0.f, 1.f, 0.f, 2.f, 0.f, 0.f, 1.f},

                                 {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                  0.f, 1.f, 0.f, 2.f, 0.f, 2.f, 1.f},

                                 {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                  0.f, 1.f, 0.f, 0.f, 0.f, 2.f, 1.f}};

    const float identityMatrix[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                                      0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
    bool firstFrame = true;
    float cameraProjection[16];
    void Frustum(float left, float right, float bottom, float top, float znear,
                 float zfar, float *m16) {
      float temp, temp2, temp3, temp4;
      temp = 2.0f * znear;
      temp2 = right - left;
      temp3 = top - bottom;
      temp4 = zfar - znear;
      m16[0] = temp / temp2;
      m16[1] = 0.0;
      m16[2] = 0.0;
      m16[3] = 0.0;
      m16[4] = 0.0;
      m16[5] = temp / temp3;
      m16[6] = 0.0;
      m16[7] = 0.0;
      m16[8] = (right + left) / temp2;
      m16[9] = (top + bottom) / temp3;
      m16[10] = (-zfar - znear) / temp4;
      m16[11] = -1.0f;
      m16[12] = 0.0;
      m16[13] = 0.0;
      m16[14] = (-temp * zfar) / temp4;
      m16[15] = 0.0;
    }

    void Perspective(float fovyInDegrees, float aspectRatio, float znear,
                     float zfar, float *m16) {
      float ymax, xmax;
      ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
      xmax = ymax * aspectRatio;
      Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
    }

    void OrthoGraphic(const float l, float r, float b, const float t, float zn,
                      const float zf, float *m16) {
      m16[0] = 2 / (r - l);
      m16[1] = 0.0f;
      m16[2] = 0.0f;
      m16[3] = 0.0f;
      m16[4] = 0.0f;
      m16[5] = 2 / (t - b);
      m16[6] = 0.0f;
      m16[7] = 0.0f;
      m16[8] = 0.0f;
      m16[9] = 0.0f;
      m16[10] = 1.0f / (zf - zn);
      m16[11] = 0.0f;
      m16[12] = (l + r) / (l - r);
      m16[13] = (t + b) / (b - t);
      m16[14] = zn / (zn - zf);
      m16[15] = 1.0f;
    }

    void Cross(const float *a, const float *b, float *r) {
      r[0] = a[1] * b[2] - a[2] * b[1];
      r[1] = a[2] * b[0] - a[0] * b[2];
      r[2] = a[0] * b[1] - a[1] * b[0];
    }

    float Dot(const float *a, const float *b) {
      return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    void Normalize(const float *a, float *r) {
      float il = 1.f / (sqrtf(Dot(a, a)) + FLT_EPSILON);
      r[0] = a[0] * il;
      r[1] = a[1] * il;
      r[2] = a[2] * il;
    }

    void LookAt(const float *eye, const float *at, const float *up,
                float *m16) {
      float X[3], Y[3], Z[3], tmp[3];

      tmp[0] = eye[0] - at[0];
      tmp[1] = eye[1] - at[1];
      tmp[2] = eye[2] - at[2];
      Normalize(tmp, Z);
      Normalize(up, Y);

      Cross(Y, Z, tmp);
      Normalize(tmp, X);

      Cross(Z, X, tmp);
      Normalize(tmp, Y);

      m16[0] = X[0];
      m16[1] = Y[0];
      m16[2] = Z[0];
      m16[3] = 0.0f;
      m16[4] = X[1];
      m16[5] = Y[1];
      m16[6] = Z[1];
      m16[7] = 0.0f;
      m16[8] = X[2];
      m16[9] = Y[2];
      m16[10] = Z[2];
      m16[11] = 0.0f;
      m16[12] = -Dot(X, eye);
      m16[13] = -Dot(Y, eye);
      m16[14] = -Dot(Z, eye);
      m16[15] = 1.0f;
    }

    void EditTransform(float *cameraView, float *cameraProjection,
                       float *matrix, bool editTransformDecomposition) {
      static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
      static bool useSnap = false;
      static float snap[3] = {1.f, 1.f, 1.f};
      static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
      static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
      static bool boundSizing = false;
      static bool boundSizingSnap = false;

      if (editTransformDecomposition) {
        if (ImGui::IsKeyPressed(90))
          mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(69))
          mCurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(82)) // r Key
          mCurrentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Translate",
                               mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
          mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate",
                               mCurrentGizmoOperation == ImGuizmo::ROTATE))
          mCurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale",
                               mCurrentGizmoOperation == ImGuizmo::SCALE))
          mCurrentGizmoOperation = ImGuizmo::SCALE;
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation,
                                              matrixRotation, matrixScale);
        ImGui::InputFloat3("Tr", matrixTranslation);
        ImGui::InputFloat3("Rt", matrixRotation);
        ImGui::InputFloat3("Sc", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(
            matrixTranslation, matrixRotation, matrixScale, matrix);

        if (mCurrentGizmoOperation != ImGuizmo::SCALE) {
          if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
          ImGui::SameLine();
          if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
        }
        if (ImGui::IsKeyPressed(83))
          useSnap = !useSnap;
        ImGui::Checkbox("", &useSnap);
        ImGui::SameLine();

        switch (mCurrentGizmoOperation) {
        case ImGuizmo::TRANSLATE:
          ImGui::InputFloat3("Snap", &snap[0]);
          break;
        case ImGuizmo::ROTATE:
          ImGui::InputFloat("Angle Snap", &snap[0]);
          break;
        case ImGuizmo::SCALE:
          ImGui::InputFloat("Scale Snap", &snap[0]);
          break;
        default:
          break;
        }
        ImGui::Checkbox("Bound Sizing", &boundSizing);
        if (boundSizing) {
          ImGui::PushID(3);
          ImGui::Checkbox("", &boundSizingSnap);
          ImGui::SameLine();
          ImGui::InputFloat3("Snap", boundsSnap);
          ImGui::PopID();
        }
      }

      ImGuiIO &io = ImGui::GetIO();
      float viewManipulateRight = io.DisplaySize.x;
      float viewManipulateTop = io.DisplaySize.y - 128;
      // if (useWindow) {
      // ImGui::SetNextWindowSize(ImVec2(800, 400));
      // ImGui::SetNextWindowPos(ImVec2(400, 20));
      // ImGui::PushStyleColor(ImGuiCol_WindowBg,
      //                       (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
      // ImGui::Begin("Gizmo", 0, ImGuiWindowFlags_NoMove);
      // ImGuizmo::SetDrawlist();
      // float windowWidth = (float)ImGui::GetWindowWidth();
      // float windowHeight = (float)ImGui::GetWindowHeight();
      // ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
      //                   windowWidth, windowHeight);
      // viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
      // viewManipulateTop = ImGui::GetWindowPos().y;
      // } else {
      ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
      // }

      // ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix,
      // 100.f); ImGuizmo::DrawCubes(cameraView, cameraProjection,
      // &objectMatrix[0][0], gizmoCount);
      // ImGuizmo::Manipulate(
      // cameraView, cameraProjection, mCurrentGizmoOperation,
      // mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL,
      // boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

      ImGuizmo::ViewManipulate(
          cameraView, camDistance,
          ImVec2(viewManipulateRight - 128, viewManipulateTop),
          ImVec2(128, 128), 0x10101010);

      // if (useWindow) {
      // ImGui::End();
      // ImGui::PopStyleColor(1);
      // }
    }

  } utilities;

  ImGuiIO &io = ImGui::GetIO();
  if (utilities.isPerspective) {
    utilities.Perspective(utilities.fov, io.DisplaySize.x / io.DisplaySize.y,
                          0.1f, 100.f, utilities.cameraProjection);
  } else {
    float viewHeight =
        utilities.viewWidth * io.DisplaySize.y / io.DisplaySize.x;
    utilities.OrthoGraphic(-utilities.viewWidth, utilities.viewWidth,
                           -viewHeight, viewHeight, 1000.f, -1000.f,
                           utilities.cameraProjection);
  }
  ImGuizmo::SetOrthographic(!utilities.isPerspective);
  ImGuizmo::BeginFrame();

  // create a window and insert the inspector
  ImGui::Begin("Editor");
  // if (ImGui::RadioButton("Full view", !utilities.useWindow))
  // utilities.useWindow = false;
  // ImGui::SameLine();
  // if (ImGui::RadioButton("Window", utilities.useWindow))
  // utilities.useWindow = true;

  // ImGui::Text("Camera");
  bool viewDirty = false;
  if (ImGui::RadioButton("Perspective", utilities.isPerspective))
    utilities.isPerspective = true;
  ImGui::SameLine();
  if (ImGui::RadioButton("Orthographic", !utilities.isPerspective))
    utilities.isPerspective = false;
  if (utilities.isPerspective) {
    ImGui::SliderFloat("Fov", &utilities.fov, 20.f, 110.f);
  } else {
    ImGui::SliderFloat("Ortho width", &utilities.viewWidth, 1, 20);
  }
  viewDirty |=
      ImGui::SliderFloat("Distance", &utilities.camDistance, 1.f, 10.f);
  ImGui::SliderInt("Gizmo count", &utilities.gizmoCount, 1, 4);

  if (utilities.viewDirty || utilities.firstFrame) {
    float eye[] = {cosf(utilities.camYAngle) * cosf(utilities.camXAngle) *
                       utilities.camDistance,
                   sinf(utilities.camXAngle) * utilities.camDistance,
                   sinf(utilities.camYAngle) * cosf(utilities.camXAngle) *
                       utilities.camDistance};
    float at[] = {0.f, 0.f, 0.f};
    float up[] = {0.f, 1.f, 0.f};
    utilities.LookAt(eye, at, up, utilities.cameraView);
    utilities.firstFrame = false;
  }

  ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
  // if (ImGuizmo::IsUsing()) {
  //   ImGui::Text("Using gizmo");
  // } else {
  //   ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
  //   ImGui::SameLine();
  //   ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate
  //   gizmo"
  //                                                     : "");
  //   ImGui::SameLine();
  //   ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" :
  //   ""); ImGui::SameLine(); ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ?
  //   "Over scale gizmo" : "");
  // }
  // ImGui::Separator();
  for (int matId = 0; matId < utilities.gizmoCount; matId++) {
    ImGuizmo::SetID(matId);

    utilities.EditTransform(utilities.cameraView, utilities.cameraProjection,
                            utilities.objectMatrix[matId],
                            utilities.lastUsing == matId);
    if (ImGuizmo::IsUsing()) {
      utilities.lastUsing = matId;
    }
  }

  ImGui::Text(" ");
  ImGui::Text(" ");
  ImGui::Text(" %f  %f  %f  %f ", utilities.cameraView[0],
              utilities.cameraView[1], utilities.cameraView[2],
              utilities.cameraView[3]);
  ImGui::Text(" %f  %f  %f  %f ", utilities.cameraView[4],
              utilities.cameraView[5], utilities.cameraView[6],
              utilities.cameraView[7]);
  ImGui::Text(" %f  %f  %f  %f ", utilities.cameraView[8],
              utilities.cameraView[9], utilities.cameraView[10],
              utilities.cameraView[11]);
  ImGui::Text(" %f  %f  %f  %f ", utilities.cameraView[12],
              utilities.cameraView[13], utilities.cameraView[14],
              utilities.cameraView[15]);

  ImGui::End();
}

// wrapper to do everything from the main loop
void engine::draw_windows() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  // graph of fps history
  if (show_fps_overlay)
    fps_overlay(&show_fps_overlay);

  // replacement orienation widget
  // orientation_widget_imgui();

  // this has to be the first ImGUI window drawn - control window docks to it
  static ImGuiDockNodeFlags dockspace_flags =
      ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode |
      ImGuiDockNodeFlags_AutoHideTabBar;
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

  // show the demo window
  static bool show_demo_window = false;
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);

  // do the voraldo menu
  if (show_voraldo_window)
    show_voraldo_menu(&show_voraldo_window);

  // show quit confirm window if the user hit escape last frame, and again every
  // frame till they choose to exit - shift+esc to override this behavior
  quit_conf(&quitconfirm);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(
      ImGui::GetDrawData()); // put imgui data into the framebuffer
}

void engine::handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    // executes regardless
    if (event.type == SDL_QUIT)
      pquit = true;

    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window))
      pquit = true;

    if ((event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE) ||
        (event.type == SDL_MOUSEBUTTONDOWN &&
         event.button.button ==
             SDL_BUTTON_X1)) // x1 is browser back on the mouse
      quitconfirm = !quitconfirm;

    // wrap in this check, to prevent issues with text editing, etc, colliding
    // with intended keyboard usage (e.g. typing in numbers triggering the
    // snap-to-view behavior, every time you hit 's' it triggers the screenshot
    // behavior...) - really quite elegant the way imgui handles it
    if (!ImGui::GetIO().WantCaptureKeyboard) {
      if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE &&
          SDL_GetModState() & KMOD_SHIFT)
        pquit = true; // force quit

      float smallstep = 0.008;
      float largestep = 0.120;

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          GPU_Data.rotate_vertical(largestep);
        } else {
          GPU_Data.rotate_vertical(smallstep);
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          GPU_Data.rotate_vertical(-largestep);
        } else {
          GPU_Data.rotate_vertical(-smallstep);
        }
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          GPU_Data.rotate_horizontal(largestep);
        } else {
          GPU_Data.rotate_horizontal(smallstep);
        }
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          GPU_Data.rotate_horizontal(-largestep);
        } else {
          GPU_Data.rotate_horizontal(-smallstep);
        }
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEDOWN) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          GPU_Data.rolltate(-largestep);
        } else {
          GPU_Data.rolltate(-smallstep);
        }
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_PAGEUP) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          GPU_Data.rolltate(largestep);
        } else {
          GPU_Data.rolltate(smallstep);
        }
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_MINUS) {
        GPU_Data.scale += 0.1f; // make scale smaller (offsets are larger)
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_EQUALS) {
        // SDLK_PLUS requires that you hit the shift
        GPU_Data.scale -= 0.1f; // make scale larger  (offsets are smaller)
      }

      // this needs to be converted, also check against TRIPLE_MONITOR flag to
      // disable

      if(TRIPLE_MONITOR)
      {
      // put on screen 1
      // if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1)
      // ("Display 0")SDL_SetWindowPosition(window, 0, 0)

      // put on screen 2
      // if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F2)
      // ("Display 1")SDL_SetWindowPosition(window, total_screen_width, 0)

      // put on screen 3
      // if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F3)
      // ("Display 2")SDL_SetWindowPosition(window, 2*total_screen_width, 0)
      }

      // snap to cardinal directions
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_1)
        GPU_Data.view_front();
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_2)
        GPU_Data.view_right();
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_3)
        GPU_Data.view_back();
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_4)
        GPU_Data.view_left();
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_5)
        GPU_Data.view_up();
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_6)
        GPU_Data.view_down();

      // till I come up with a good way to maintain state for the mouse click
      // and drag, this is how that offset is controlled - got a line on this
      // with some of the imgui functions related to click and drag. Note it
      // will need to go inside the wantcapturemouse check, so that there aren't
      // collisions with existing imgui behavior - see note below
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h) {
        GPU_Data.clickndragx += SDL_GetModState() & KMOD_SHIFT ? 50 : 5;
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_l) {
        GPU_Data.clickndragx -= SDL_GetModState() & KMOD_SHIFT ? 50 : 5;
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_k) {
        GPU_Data.clickndragy += SDL_GetModState() & KMOD_SHIFT ? 50 : 5;
      }
      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_j) {
        GPU_Data.clickndragy -= SDL_GetModState() & KMOD_SHIFT ? 50 : 5;
      }

      if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
        if (SDL_GetModState() & KMOD_SHIFT) {
          std::string animation_file_location = std::string("animation.json");
          GPU_Data.animation(animation_file_location);
        } else {
          GPU_Data.single_screenshot();
        }
      }
    }

    // NEED TO ADD CLICK AND DRAG CODE HERE - wrapping in wantcapturemouse check
    // is a good way to make sure that I'm not triggering the click and drag
    // behavior when not intended. This has already been a huge improvement for
    // the keyboard stuff, with how much it was triggering the behavior while I
    // was inputting text in the text editor, etc, now it's not doing any of
    // that anymore.

    // mouse operations
    if (!ImGui::GetIO().WantCaptureMouse) {
      if (event.type == SDL_MOUSEWHEEL) {
        // allow scroll to do the same thing as +/-
        if (event.wheel.y > 0) // scroll up
        {
          GPU_Data.scale -= 0.1f;
        } else if (event.wheel.y < 0) // scroll down
        {
          GPU_Data.scale += 0.1f;
        }
      }
    }
  }
}

// quit confirmation, just to prevent accidental quit - shift-esc override
// vim muscle memory can be an issue when you just quit with esc
void engine::quit_conf(bool *open) {
  if (*open) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;

    // create centered window
    ImGui::SetNextWindowPos(
        ImVec2(total_screen_width / 2 - 120, total_screen_height / 2 - 25));
    ImGui::SetNextWindowSize(ImVec2(215, 40));
    ImGui::Begin("quit", open, flags);

    ImGui::Text("Are you sure you want to quit?");

    ImGui::Text("  ");
    ImGui::SameLine();

    // button to cancel -> set this window's bool to false
    if (ImGui::SmallButton(" Cancel "))
      *open = false;

    ImGui::SameLine();
    ImGui::Text("    ");
    ImGui::SameLine();

    // button to quit -> set pquit to true
    if (ImGui::SmallButton(" Quit "))
      pquit = true;

    ImGui::End();
  }
}

void engine::quit() {
  // shutdown everything
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  // destroy window
  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  // really shoould be destroying textures, etc
  cout << "goodbye." << endl;
}
