#include "input.hpp"

#include <cstdlib>
#include <SDL2/SDL_events.h>

bool input_quit_requested;

static const uint16_t scancode_to_ascii[128] =
{
	  0,   0, '1', '2', '3', '4', '5', '6', //  0
	'7', '8', '9', '0',   0,   0, 128,   0, //  8
	'Q', 'W',   0, 'R', 'T', 'Y',   0,   0, // 10
	  0, 'P', 129, 129, 129,   0,   0, 'S', // 18
	'D', 'F', 'G', 'H', 'J', 'K', 'L',   0, // 20
	  0,   0,   0,   0, 'Z', 'X', 'C', 'V', // 28
	'B', 'N', 'M',   0,   0,   0,   0,   0, // 30
	  0, 129,   0,   0,   0,   0,   0,   0, // 38
};

static const int MAX_TRANSLATED_SDL_SCANCODES = 128;
static const uint8_t scancode_SDL_to_XT_table[MAX_TRANSLATED_SDL_SCANCODES] =
{
    0,    // SDL_SCANCODE_UNKNOWN = 0,
	0,    // 1
	0,    // 2
	0,    // 3
    0x1E, // SDL_SCANCODE_A = 4,
    0x30, // SDL_SCANCODE_B = 5,
    0x2E, // SDL_SCANCODE_C = 6,
    0x20, // SDL_SCANCODE_D = 7,
    0x12, // SDL_SCANCODE_E = 8,
    0x21, // SDL_SCANCODE_F = 9,
    0xA2, // SDL_SCANCODE_G = 10,
    0xA3, // SDL_SCANCODE_H = 11,
    0x17, // SDL_SCANCODE_I = 12,
    0x24, // SDL_SCANCODE_J = 13,
    0x25, // SDL_SCANCODE_K = 14,
    0x26, // SDL_SCANCODE_L = 15,
    0x32, // SDL_SCANCODE_M = 16,
    0x31, // SDL_SCANCODE_N = 17,
    0x18, // SDL_SCANCODE_O = 18,
    0x19, // SDL_SCANCODE_P = 19,
    0x10, // SDL_SCANCODE_Q = 20,
    0x13, // SDL_SCANCODE_R = 21,
    0x1F, // SDL_SCANCODE_S = 22,
    0x14, // SDL_SCANCODE_T = 23,
    0x16, // SDL_SCANCODE_U = 24,
    0x2F, // SDL_SCANCODE_V = 25,
    0x11, // SDL_SCANCODE_W = 26,
    0x2D, // SDL_SCANCODE_X = 27,
    0x15, // SDL_SCANCODE_Y = 28,
    0x2C, // SDL_SCANCODE_Z = 29,
    0x02, // SDL_SCANCODE_1 = 30,
    0x03, // SDL_SCANCODE_2 = 31,
    0x04, // SDL_SCANCODE_3 = 32,
    0x05, // SDL_SCANCODE_4 = 33,
    0x06, // SDL_SCANCODE_5 = 34,
    0x07, // SDL_SCANCODE_6 = 35,
    0x08, // SDL_SCANCODE_7 = 36,
    0x09, // SDL_SCANCODE_8 = 37,
    0x0A, // SDL_SCANCODE_9 = 38,
    0x0B, // SDL_SCANCODE_0 = 39,
    0x1C, // SDL_SCANCODE_RETURN = 40,
    0x01, // SDL_SCANCODE_ESCAPE = 41,
    0x0E, // SDL_SCANCODE_BACKSPACE = 42,
    0x0F, // SDL_SCANCODE_TAB = 43,
    0x39, // SDL_SCANCODE_SPACE = 44,
    0x0C, // SDL_SCANCODE_MINUS = 45,
    0x0D, // SDL_SCANCODE_EQUALS = 46,
    0x1A, // SDL_SCANCODE_LEFTBRACKET = 47,
    0x1B, // SDL_SCANCODE_RIGHTBRACKET = 48,
    0x2B, // SDL_SCANCODE_BACKSLASH = 49,
    0x2B, // SDL_SCANCODE_NONUSHASH = 50, // See SDL comment
    0x27, // SDL_SCANCODE_SEMICOLON = 51,
    0x28, // SDL_SCANCODE_APOSTROPHE = 52,
    0x29, // SDL_SCANCODE_GRAVE = 53,
    0x33, // SDL_SCANCODE_COMMA = 54,
    0x34, // SDL_SCANCODE_PERIOD = 55,
    0x35, // SDL_SCANCODE_SLASH = 56,
    0x3A, // SDL_SCANCODE_CAPSLOCK = 57,
    0x3B, // SDL_SCANCODE_F1 = 58,
    0x3C, // SDL_SCANCODE_F2 = 59,
    0x3D, // SDL_SCANCODE_F3 = 60,
    0x3E, // SDL_SCANCODE_F4 = 61,
    0x3F, // SDL_SCANCODE_F5 = 62,
    0x40, // SDL_SCANCODE_F6 = 63,
    0x41, // SDL_SCANCODE_F7 = 64,
    0x42, // SDL_SCANCODE_F8 = 65,
    0x43, // SDL_SCANCODE_F9 = 66,
    0x44, // SDL_SCANCODE_F10 = 67,
    0x57, // SDL_SCANCODE_F11 = 68,
    0x58, // SDL_SCANCODE_F12 = 69,
    0,    // SDL_SCANCODE_PRINTSCREEN = 70,
    0x46, // SDL_SCANCODE_SCROLLLOCK = 71,
    0,    // SDL_SCANCODE_PAUSE = 72,
    0x52, // SDL_SCANCODE_INSERT = 73,
    0x47, // SDL_SCANCODE_HOME = 74,
    0x49, // SDL_SCANCODE_PAGEUP = 75,
    0x53, // SDL_SCANCODE_DELETE = 76,
    0x4F, // SDL_SCANCODE_END = 77,
    0x51, // SDL_SCANCODE_PAGEDOWN = 78,
    0x4D, // SDL_SCANCODE_RIGHT = 79,
    0x4B, // SDL_SCANCODE_LEFT = 80,
    0x50, // SDL_SCANCODE_DOWN = 81,
    0x48, // SDL_SCANCODE_UP = 82,
    0x45, // SDL_SCANCODE_NUMLOCKCLEAR = 83,
    0x35, // SDL_SCANCODE_KP_DIVIDE = 84,
    0x37, // SDL_SCANCODE_KP_MULTIPLY = 85,
    0x4A, // SDL_SCANCODE_KP_MINUS = 86,
    0x4E, // SDL_SCANCODE_KP_PLUS = 87,
    0x1C, // SDL_SCANCODE_KP_ENTER = 88,
    0x4F, // SDL_SCANCODE_KP_1 = 89,
    0x50, // SDL_SCANCODE_KP_2 = 90,
    0x51, // SDL_SCANCODE_KP_3 = 91,
    0x4B, // SDL_SCANCODE_KP_4 = 92,
    0x4C, // SDL_SCANCODE_KP_5 = 93,
    0x4D, // SDL_SCANCODE_KP_6 = 94,
    0x47, // SDL_SCANCODE_KP_7 = 95,
    0x48, // SDL_SCANCODE_KP_8 = 96,
    0x49, // SDL_SCANCODE_KP_9 = 97,
    0x52, // SDL_SCANCODE_KP_0 = 98,
    0x53, // SDL_SCANCODE_KP_PERIOD = 99,
    0x2B, // SDL_SCANCODE_NONUSBACKSLASH = 100, // See SDL comment
    0,    // SDL_SCANCODE_APPLICATION = 101,
    0,    // SDL_SCANCODE_POWER = 102,
    0x0D, // SDL_SCANCODE_KP_EQUALS = 103,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0x1D, // SDL_SCANCODE_LCTRL = 224,
    0x2A, // SDL_SCANCODE_LSHIFT = 225,
    0x38, // SDL_SCANCODE_LALT = 226, /**< alt, option */
    0,    // SDL_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
    0x1D, // SDL_SCANCODE_RCTRL = 228,
    0xB6, // SDL_SCANCODE_RSHIFT = 229,
    0x38, // SDL_SCANCODE_RALT = 230, /**< alt gr, option */
    0,    // SDL_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */
};

// Translates SDL/USB scancodes to XT scancodes
static uint8_t sdlToXTScancode(SDL_Scancode sdl_scancode_code)
{
	int sdl_scancode = sdl_scancode_code;

	// These were wedged at the end of the table so I could make it smaller
	if (sdl_scancode >= SDL_SCANCODE_LCTRL)
		sdl_scancode -= SDL_SCANCODE_RGUI - (MAX_TRANSLATED_SDL_SCANCODES - 1);

	// All keys I care about are in this range
	if (sdl_scancode >= MAX_TRANSLATED_SDL_SCANCODES)
		sdl_scancode = SDL_SCANCODE_UNKNOWN;

	return scancode_SDL_to_XT_table[sdl_scancode];
}

void input_handleSDLEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			//keyboardHandler(event.key);
			break;
		case SDL_QUIT:
			input_quit_requested = true;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				// This needs to be caught in addition to SDL_QUIT when using multi-windows.
				input_quit_requested = true;
				break;
			}
		}
	}
}
