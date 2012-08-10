#include "timer.hpp"

#include "vga_emu.hpp"
#include "input.hpp"

#include <cstdint>
#include <SDL.h>
#include <SDL_timer.h>

bool timer_initialized = false; // addr seg04:A39A
static uint16_t timer_wait_count; // addr seg04:A39C
static Uint64 timer_deadline;

void timer_initialize()
{
	SDL_InitSubSystem(SDL_INIT_TIMER);
	timer_deadline = SDL_GetPerformanceCounter();

	timer_initialized = true;
}

void timer_setWaitCount(int val)
{
	int diff = val - timer_wait_count;
	bool neg_diff = false;
	if (val < 0)
	{
		neg_diff = true;
		val = -val;
	}

	Uint64 shift_amount = SDL_GetPerformanceFrequency() * val / 60;

	if (neg_diff == false)
		timer_deadline += shift_amount;
	else
		timer_deadline -= shift_amount;

	timer_wait_count = val;
}

uint16_t timer_getWaitCount()
{
	return timer_wait_count;
}

// addr seg00:0130
void timer_wait() {
	while (timer_wait_count > 0 && SDL_GetPerformanceCounter() < timer_deadline)
	{
		vga_present();
		handleSDLEvents();

		if (vga_has_vsync)
		{
			timer_wait_count--;
		}
		else
		{
			timer_wait_count = 0;
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
	}
}
