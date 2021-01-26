#include "engine.h"
#include "../debug/debug.h" // temporarily broken
// This contains the very high level expression of what's going on

engine::engine()
{
    init();             // set up SDL2, OpenGL, dearImGUI
    gl_debug_enable(); // debug callback for error reporting

    while(!pquit) // main loop
        draw_everything();
}

engine::~engine()
{
    quit();
}
