#include "debug.hpp"
#include "draw.hpp"

VideoWindow debug_win;

void debug_initialize() {
	debug_win.initialize(640, 480);
}

void debug_deinitialize() {
	debug_win.deinitialize();
}

void debug_frame(std::vector<SDL_Event>& events) {
	events.clear();

	fillSurface(0, debug_win.getSurface());
	debug_win.present();
}
