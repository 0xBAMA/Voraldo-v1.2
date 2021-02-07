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

  GPU_Data.orientation_widget_offset = glm::vec3(0.9, -0.74, 0.0);

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

#define FPS_HISTORY_SIZE 95
  fps_history.resize(FPS_HISTORY_SIZE); // initialize the array of fps values

  ImVec4 *colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_Text] = ImVec4(0.67f, 0.50f, 0.16f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.33f, 0.27f, 0.16f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.05f, 0.00f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.23f, 0.17f, 0.02f, 0.05f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.07f, 0.01f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.25f, 0.18f, 0.09f, 0.33f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.33f, 0.15f, 0.02f, 0.17f);
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
  colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.18f, 0.06f, 0.17f);
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

  style.FrameRounding = 2;
  style.WindowRounding = 3;
  style.ScrollbarSize = 10;
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
  // ImGui::Begin("Voraldo Menu", show, ImGuiWindowFlags_NoDecoration);
  ImGui::Begin("Voraldo Menu", show,
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

  // window contents
  // ImGui::Text("This is some text");
  ImGuiTabBarFlags tab_bar_flags =
      ImGuiTabBarFlags_None | ImGuiTabBarFlags_FittingPolicyScroll;

  if (ImGui::BeginTabBar("top_level", tab_bar_flags)) {
    if (ImGui::BeginTabItem(" Shapes ")) {
      ImGui::BeginTabBar("shapes", tab_bar_flags);
      if (ImGui::BeginTabItem(" AABB ")) {

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Cuboid ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Cylinder ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Ellipsoid ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Grid ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Heightmap ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Icosahedron ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Noise ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Sphere ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Tube ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Triangle ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" VAT ")) {
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem(" Utilities ")) {
      ImGui::BeginTabBar("utils", tab_bar_flags);
      if (ImGui::BeginTabItem(" Clear ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Masking ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Box Blur ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Gaussian Blur ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Limiter ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Shift ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Copy/Paste ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Load/Save ")) {
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(" Lighting ")) {
      ImGui::BeginTabBar("lighting", tab_bar_flags);
      if (ImGui::BeginTabItem(" Clear ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Point ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Cone ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Directional ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Fake GI ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Ambient Occlusion ")) {
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(" Mash ")) {
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

      ImGui::Text("");

      ImGui::Separator();

      ImGui::Text("");
      ImGui::ColorEdit3(
          "BG Color",
          (float *)&clear_color); // Edit 3 floats representing a color

      GPU_Data.clear_color =
          glm::vec4(clear_color.x, clear_color.y, clear_color.z, 1.0);
      ImGui::SameLine();
      HelpMarker("OpenGL Clear Color");
      ImGui::Text("");
      ImGui::Separator();
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
    ImGui::End();
  }
}

void engine::quit_conf(bool *open) {
  if (*open) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration;

    // create centered window
    ImGui::SetNextWindowPos(
        ImVec2(total_screen_width / 2 - 120, total_screen_height / 2 - 25));
    ImGui::SetNextWindowSize(ImVec2(240, 55));
    ImGui::Begin("quit", open, flags);

    ImGui::Text("Are you sure you want to quit?");

    ImGui::Text("  ");
    ImGui::SameLine();

    // button to cancel -> set this window's bool to false
    if (ImGui::Button(" Cancel "))
      *open = false;

    ImGui::SameLine();
    ImGui::Text("      ");
    ImGui::SameLine();

    // button to quit -> set pquit to true
    if (ImGui::Button(" Quit "))
      pquit = true;

    ImGui::End();
  }
}

void engine::draw_windows() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  // this has to be the first ImGUI window drawn - control window docks to it
  static ImGuiDockNodeFlags dockspace_flags =
      ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode |
      ImGuiDockNodeFlags_AutoHideTabBar;
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

  // show the demo window
  static bool show_demo_window = true;
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);

  // do the voraldo menu
  if (show_voraldo_window)
    show_voraldo_menu(&show_voraldo_window);

  // show quit confirm window if the user hit escape last frame, and again every
  // frame till they choose to exit
  quit_conf(&quitconfirm);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(
      ImGui::GetDrawData()); // put imgui data into the framebuffer
}

void engine::handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

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
  }
}

void engine::update_fps_history() {
  // push back - put in the new value
  fps_history.push_back(ImGui::GetIO().Framerate);
  // pop front - take out the oldest value
  fps_history.pop_front();
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

  cout << "goodbye." << endl;
}
