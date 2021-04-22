#include "engine.h"
#include "../debug/debug.h" // temporarily broken
// This contains the very high level expression of what's going on

engine::engine()
{
}

void engine::init()
{
    SDL2_setup();           // all SDL setup, window hidden
    gl_setup();             // gl3w init, glEnables, blendfunc
    gl_debug_enable();
    GPU_Data.init();        // wrapper for GPU-side setup
    imgui_setup();          // colors, other config
    SDL_ShowWindow(window); // show the window when done
}

engine::~engine()
{
    quit();
}
