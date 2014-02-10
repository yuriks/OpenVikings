#include "main.hpp"

#include "vga_emu.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "data.hpp"
#include "decoding.hpp"
#include "draw.hpp"
#include "debug.hpp"
#include "game.hpp"
#include <array>
#include <vector>
#include <SDL2/SDL_events.h>

static void deinitializeGame() {
	closeDataFiles();
	debug_deinitialize();
	vga_deinitialize();
}

void errorQuit(const char* message, unsigned int code) {
	std::printf("*** OpenVikings encoutered a runtime error:\n\n%s\nError code: %ud\n", message, code);

	deinitializeGame();
	exit(1);
}

void eventLoop() {
	std::vector<SDL_Event> game_events;
	std::vector<SDL_Event> debug_events;

	auto get_queue = [&game_events, &debug_events](uint32_t window_id) -> std::vector<SDL_Event>& {
		if (window_id == vga_window.getWindowId()) {
			return game_events;
		} else if (window_id == debug_win.getWindowId()) {
			return debug_events;
		} else {
			assert(false);
			exit(1);
		};
	};

	while (true) {
		// Dispatch SDL events to the appropriate window.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
				// This isn't the correct union member to use for any of these
				// events, but all events with windowID have it in the same
				// position, and SDL maintains ABI compatibility, so this is
				// most likely fine.
				get_queue(event.window.windowID).push_back(event);
				break;
			case SDL_QUIT:
				input_quit_requested = true;
				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					// This needs to be caught in addition to SDL_QUIT when using multi-windows.
					input_quit_requested = true;
					break;
				}
			}
		}

		if (input_quit_requested) {
			break;
		}

		game_frame(game_events);
		debug_frame(debug_events);
	}
}

int main() {
	vga_initialize();
	debug_initialize();
	openDataFiles();
	loadFont();

	vga_window.palette[0] = 0x000000;
	vga_window.palette[1] = 0x000000;
	vga_window.palette[2] = 0xFBFBFB;
	vga_window.palette[3] = 0x920000;

	eventLoop();

	deinitializeGame();
}
