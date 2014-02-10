#pragma once
#include "vga_emu.hpp"
#include <vector>
#include <SDL2/SDL_events.h>

extern VideoWindow debug_win;

void debug_initialize();
void debug_deinitialize();
void debug_frame(std::vector<SDL_Event>& events);
