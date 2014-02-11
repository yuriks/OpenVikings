#include "debug.hpp"
#include "draw.hpp"
#include "gui.hpp"

VideoWindow debug_win;
static Gui gui;

void debug_initialize() {
	debug_win.initialize(640, 480, "OpenVikings - Debugger");
	gui.surface = debug_win.getSurface();

	debug_win.palette[0] = 0x000000;
	debug_win.palette[1] = 0x000000;
	debug_win.palette[2] = 0xFBFBFB;
	debug_win.palette[3] = 0x920000;
}

void debug_deinitialize() {
	debug_win.deinitialize();
}

void debug_frame(std::vector<SDL_Event>& events) {
	int mouse_x = gui.mouse_x;
	int mouse_y = gui.mouse_y;
	int mouse_buttons = gui.mouse_buttons;

	for (const auto& ev : events) {
		switch (ev.type) {
		case SDL_MOUSEMOTION:
			mouse_x = ev.motion.x;
			mouse_y = ev.motion.y;
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			int button_bit = BIT(ev.button.button);
			mouse_buttons = (mouse_buttons & (~button_bit)) | (ev.button.state == SDL_MOUSEBUTTONDOWN ? button_bit : 0);
			break;
		}
	}
	events.clear();

	gui.update_input(mouse_x, mouse_y, mouse_buttons);

	fillSurface(3, debug_win.getSurface());
	gui.label("HELLO WORLD!", 8, 8);

	debug_win.present();
}
