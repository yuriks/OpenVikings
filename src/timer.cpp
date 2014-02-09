#include "timer.hpp"

#include "vga_emu.hpp"
#include "input.hpp"
#include "main.hpp"

#include <cstdint>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>

bool timer_initialized = false; // addr seg04:A39A
static uint16_t timer_wait_count; // addr seg04:A39C
static Uint64 timer_deadline;

void timer_initialize()
{
	SDL_InitSubSystem(SDL_INIT_TIMER);

	if (vga_has_vsync)
		vga_present(); // Pull closer to vsync before setting initial deadline
	timer_deadline = SDL_GetPerformanceCounter();

	timer_initialized = true;
}

static Uint64 timer_getFrameDuration()
{
	return SDL_GetPerformanceFrequency() / 60;
}

void timer_setWaitCount(int val)
{
	assert(val >= 0);

	if (timer_wait_count == 0 && val != 0)
	{
		timer_deadline += timer_getFrameDuration();
	}
	else if (timer_wait_count != 0 && val == 0)
	{
		timer_deadline -= timer_getFrameDuration();
	}

	timer_wait_count = val;
}

uint16_t timer_getWaitCount()
{
	return timer_wait_count;
}

// addr seg00:0130
void timer_wait() {
	while (timer_wait_count > 0)
	{
		if (!vga_has_vsync || SDL_GetPerformanceCounter() < timer_deadline)
		{
			vga_present();
		}

		if (!vga_has_vsync)
		{
			Uint64 counter;
			while ((counter = SDL_GetPerformanceCounter()) < timer_deadline)
			{
				Uint64 ticks_diff = timer_deadline - counter;
				Uint64 freq = SDL_GetPerformanceFrequency();
				Uint64 ms_diff = ticks_diff * 1000 / freq;
				if (ms_diff > 0)
					SDL_Delay((Uint32)ms_diff);
			}
		}

		handleSDLEvents();
		timer_wait_count--;

		//updateVidRegsAndPal();
	}
}
