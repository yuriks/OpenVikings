#pragma once
#include "draw.hpp"
#include <string>
#include <cstdint>

typedef int WidgetId;

struct Gui {
	int mouse_x = 0, mouse_y = 0;
	int mouse_buttons = 0;

	WidgetId hot_widget = -1;
	WidgetId active_widget = -1;

	DrawSurface surface;

	void update_input(int new_mouse_x, int new_mouse_y, int new_mouse_buttons);

	void label(const std::string& text, int x, const int y, uint8_t color = 2);
};
