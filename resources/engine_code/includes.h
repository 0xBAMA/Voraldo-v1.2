#ifndef INCLUDES
#define INCLUDES

#include <stdio.h>

// stl includes
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <filesystem> //c++20 support not quite universal yet
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// iostream aliases
using std::cerr;
using std::cin;
using std::cout;

using std::endl;
using std::flush;

// vector math library GLM
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "../glm/glm.hpp"                  //general vector types
#include "../glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "../glm/gtc/type_ptr.hpp"         //to send matricies gpu-side
#include "../glm/gtx/transform.hpp"

// not sure as to the utility of this
#define GLX_GLEXT_PROTOTYPES

// GUI library (dear ImGUI)
#include "../ocornut_imgui/imgui.h"
#include "../ocornut_imgui/imgui_impl_opengl3.h"
#include "../ocornut_imgui/imgui_impl_sdl.h"

// GLEW replacment
#include "../ocornut_imgui/gl3w.h"

// SDL includes - windowing, gl context, system info
#include <SDL2/SDL.h>
// allows you to run OpenGL inside of SDL2
#include <SDL2/SDL_opengl.h>

// png loading library - very powerful
#include "../lodev_lodePNG/lodepng.h"

// shader compilation wrapper - may need to be extended
#include "shader.h"

// up to 3d perlin noise generation
#include "../perlin/perlin.h"

// diamond square heightmap generation
#include "../mafford_diamond_square/diamond_square.h"

// Brent Werness' Voxel Automata Terrain
#include "../VAT/VAT.h"

// Niels Lohmann - JSON for Modern C++
#include "../nlohmann_JSON/json.hpp"
using json = nlohmann::json;

// contains the OpenGL wrapper class
#include "gpu_data.h"

// pi definition
constexpr double pi = 3.14159265358979323846;

#define TRIPLE_MONITOR // enable to span all three monitors

// voxel dimension
#define DIM 256

// supersampling factor for main display shader
// #define SSFACTOR 5.0   // tanks performance
#define SSFACTOR 2.8 // this is for 8x multisampling
// #define SSFACTOR 2.0  // this is for 4x multisampling
// #define SSFACTOR 1.65
// #define SSFACTOR 1.25  // small amount of multisampling
// #define SSFACTOR 1.0  // no multisampling
// #define SSFACTOR 0.4 // this is <1x multisampling

// for the tile based rendering - needs to be a multiple of 32
#define TILESIZE 64

#define NUM_ROTATION_STEPS 1000

// #define WIDTH 640
// #define HEIGHT 480

#endif
