#pragma once

#include <cstdint>

extern bool timer_initialized; // addr seg04:A39A

void timer_initialize();
void timer_setWaitCount(int val);
uint16_t timer_getWaitCount();
void timer_wait();
