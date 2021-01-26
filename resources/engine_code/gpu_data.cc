#include "gpu_data.h" 
#include "includes.h"

void GLContainer::display_block()
{
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // check redraw flag
}

void GLContainer::display_orientation_widget()
{
    
}
