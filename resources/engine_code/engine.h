#ifndef ENGINE
#define ENGINE

#include "includes.h"

class engine
{
	public:

		engine();
		~engine();

	private:

		SDL_Window * window;
		SDL_GLContext GLcontext;
		
		// screen dimensions
		int total_screen_width;
		int total_screen_height;

		GLContainer GPU_Data;
		
		void init()
		{
			SDL2_setup();
			gl_setup();
			imgui_setup();
		}
		
		// init helper functions
		void SDL2_setup();
		void gl_setup();
		void imgui_setup();

		// called from main loop
		void draw_everything();

		// shows general control window
		bool show_voraldo_window = true;
		void show_voraldo_menu(bool *open);


		// shows the fps history
		bool show_fps_overlay = true;
		std::deque<float> fps_history;
		void FPSOverlay(bool* p_open);


		// everything associated with quitting
		bool quitconfirm = false; // show quit prompt
		void quit_conf(bool *open); // draws quit prompt window
		bool pquit = false; // loop control on main loop
		void quit(); // close window, delete GPU data

	public:

		// OBJ data (per mesh) - used in OBJ loading
		void load_OBJ(std::string filename);

		// this may vary in length, as they are indexed into independently
		std::vector<glm::vec4> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> texcoords;

		// these should all be the same length, the number of triangles
		std::vector<glm::ivec3> triangle_indices;
		std::vector<glm::ivec3> normal_indices;
		std::vector<glm::ivec3> texcoord_indices;
		
};

#endif
