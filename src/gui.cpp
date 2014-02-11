#include "gui.hpp"

void Gui::update_input(int new_mouse_x, int new_mouse_y, int new_mouse_buttons) {
	mouse_x = new_mouse_x;
	mouse_y = new_mouse_y;
	mouse_buttons = new_mouse_buttons;
}

void Gui::label(const std::string& text, int x, const int y, uint8_t color) {
	for (int c : text) {
		drawTextTilePlus(c, color, surface, x, y);
		x += 8;
	}
}
