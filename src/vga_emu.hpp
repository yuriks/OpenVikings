#pragma once

#include <cstdint>

extern uint8_t vga_framebuffer[4][0x10000];

void vga_copyToVRAM(int plane_mask, uint8_t* source, uint16_t dest, uint16_t len);
void vga_copyPlanesToVRAM(uint8_t* source, uint16_t dest, uint16_t len_plane);
void vga_fillVRAM(int plane_mask, uint8_t value, uint16_t dest, uint16_t len);