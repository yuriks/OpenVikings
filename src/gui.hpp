#pragma once
#include "draw.hpp"
#include <string>

typedef int WidgetId;

struct Gui {
	int mouse_x = 0, mouse_y = 0;
	int mouse_buttons = 0;

	WidgetId hot_widget = -1;
	WidgetId active_widget = -1;

	DrawSurface surface;

	void update_input(int new_mouse_x, int new_mouse_y, int new_mouse_buttons) {
		mouse_x = new_mouse_x;
		mouse_y = new_mouse_y;
		mouse_buttons = new_mouse_buttons;
	}

	void label(const std::string& text, int x, const int y) {
		for (int c : text) {
			drawTextTile(c, surface, x, y);
			x += 8;
		}
	}
};