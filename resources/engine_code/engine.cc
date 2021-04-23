#include "engine.h"
#include "../debug/debug.h" // temporarily broken
// This contains the very high level expression of what's going on

engine::engine():uiInit(false)
{
}

bool engine::init()
{
    SDL2_setup();           // all SDL setup, window hidden
    gl_setup();             // gl3w init, glEnables, blendfunc
    gl_debug_enable();

    if(!GPU_Data.init())        // wrapper for GPU-side setup
        return false;

    imgui_setup();          // colors, other config
    SDL_ShowWindow(window); // show the window when done

    uiInit=true;

    return true;
}

engine::~engine()
{
    quit();
}
