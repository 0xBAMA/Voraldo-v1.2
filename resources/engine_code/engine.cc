#include "engine.h"
#include "../debug/debug.h" // temporarily broken
// This contains the very high level expression of what's going on

engine::engine()
{
    pquit = false;

    create_window();

    gl_debug_enable(); 

    gl_setup();

    while(!pquit) // main loop
    {
        draw_everything();
    }
}

engine::~engine()
{
    quit();
}
