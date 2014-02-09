#include "main.hpp"

#include "vga_emu.hpp"
#include "input.hpp"
#include "timer.hpp"
#include "data.hpp"
#include <array>

static void deinitializeGame() {
	closeDataFiles();
	vga_deinitialize();
}

void errorQuit(const char* message, unsigned int code) {
	std::printf("*** OpenVikings encoutered a runtime error:\n\n%s\nError code: %ud\n", message, code);

	deinitializeGame();
	exit(1);
}

int main() {
	vga_initialize();
	openDataFiles();

	std::array<uint8_t, 0x400> buffer;
	decompressChunk(0x0c6, buffer.data(), buffer.size());

	deinitializeGame();
}
