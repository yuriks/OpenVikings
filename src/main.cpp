#include "main.hpp"

#include "vga_emu.hpp"
#include "input.hpp"
#include "timer.hpp"

static void deinitializeGame() {
	vga_deinitialize();
}

void errorQuit(const char* message, unsigned int code) {
	std::printf("*** OpenVikings encoutered a runtime error:\n\n%s\nError code: %ud\n", message, code);

	deinitializeGame();
	exit(1);
}

int main() {
	vga_initialize();

	deinitializeGame();
}
