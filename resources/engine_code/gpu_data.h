#ifndef GPU_DATA
#define GPU_DATA

#include "includes.h"

class GLContainer
{
    public:

        GLContainer()  {}
        ~GLContainer() {}

        // GPU side initialization
        void init()
        {
            // compile_shaders();
            // buffer_geometry();
            // load_textures();
        } 

        // display functions
        bool show_widget = true;
        void display()
        {
            display_block();
            if(show_widget)
                display_orientation_widget();
        }

        // OpenGL clear color
        glm::vec4 clear_color; 

        // screen dimensions
        unsigned int screen_width, screen_height; 

    private:
        // display helper functions
        void display_block();
        void display_orientation_widget();

};

#endif
