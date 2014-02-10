#include "game.hpp"
#include "draw.hpp"
#include "vga_emu.hpp"

static bool pressing_key = false;

void game_frame(std::vector<SDL_Event>& events) {
	for (const auto& ev : events) {
		switch (ev.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (ev.key.keysym.scancode == SDL_SCANCODE_A) {
				pressing_key = ev.key.type == SDL_KEYDOWN;
			}
		}
	}
	events.clear();

	drawTextTile(pressing_key ? '*' : 'O', vga_window.getSurface(), 0, 0);

	vga_window.present();
}
