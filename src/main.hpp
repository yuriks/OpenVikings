#pragma once

#include <cstdint>

void errorQuit(const char* msg, uint16_t error_code);
uint8_t* decompressChunk(uint16_t chunk_id, uint8_t* dst, size_t dst_size);
void loadImage(uint16_t chunk_id, uint16_t offset);
