
#pragma once

#define NOMINMAX
#include <Windows.h>
#undef small
#include <Xinput.h>

#include "../Utility/Assert.h"
#include "Platform_Interface.h"

#define GAME_STATE_MEMORY_SIZE MiB

struct Win32_App
{
	i32 window_width = 0, window_height = 0;
	i32 flags = 0;
	
	HDC dc = 0;
	HINSTANCE instance = 0;
	HWND window = 0;
	
	WINDOWPLACEMENT window_placement = { sizeof(window_placement) };
	
	u64 frame_counter = 0;
	i64 timer_counter_freg = 0;
	LARGE_INTEGER last_time_counter;
	
	f64 next_controler_test_time = 0;
	
	void* game_state_memory = 0;
	
	inline static constexpr i32 max_controllers = 1;
	
	Controller_State controller_state[max_controllers];
	
	bool force_update_surface = false;
};


struct Win32_Bitmap
{
	inline static constexpr i32 bytes_per_pixel = 4;
	
	u32* memory = nullptr;
	i32 width = 0, height = 0;
	BITMAPINFO info{};
};

static LRESULT win32_message_handler(HWND win_handle, UINT message, WPARAM wparam, LPARAM lParam);
static u32* win32_resize_pixel_buffer(i32 new_width, i32 new_height);
static void win32_init(HINSTANCE instance, u32 window_width, u32 window_height, u32 window_pos_x, u32 window_pos_y, char* window_title);
static void win32_flush_events();
static void win32_update_surface(bool update_from_game);
static void win32_set_flag(App_Flags flag, bool value);
static f64 win32_update_frame_time();
static u32 win32_get_window_width();
static u32 win32_get_window_height();
static u32* win32_get_pixel_buffer();
static i32 win32_get_pixel_buffer_width();
static i32 win32_get_pixel_buffer_height();
static u64 win32_get_cpu_time_stamp();
static u32 win32_get_flags();
static Controller_State win32_get_controller_state(i32 contoller_idx);
static f64 win32_get_time_stamp();
static v2i win32_get_cursor_position();
static Platform_Call_Table win32_get_call_table();
static bool win32_get_keyboard_key_down(Key_Code key_code);
static u64 win32_get_frame_count();
static bool win32_write_file(const char* file_name, u8* buffer, u32 bytes_to_write);
static bool win32_get_file_size(const char* file_name, u32* out_file_size);
static bool win32_read_file(const char* file_name, u8* out_buffer, u32 bytes_to_read);


// ---------------------------------------------------------------
// Silly maps begin here.



static constexpr i32 s_controller_map[(u64)Button::BUTTON_COUNT] =
{
	0x0001,	//0
	0x0002,	//1
	0x0004,	//2
	0x0008,	//3
	0x0010,	//4
	0x0020,	//5
	0x0040,	//6
	0x0080,	//7
	0x0100,	//8
	0x0200,	//9
	0x1000,	//10
	0x2000,	//11
	0x4000,	//12
	0x8000  //13
};


static constexpr i32 s_keycode_map[(u64)Key_Code::COUNT] = 
{
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	VK_NUMPAD0,
	VK_NUMPAD1,
	VK_NUMPAD2,
	VK_NUMPAD3,
	VK_NUMPAD4,
	VK_NUMPAD5,
	VK_NUMPAD6,
	VK_NUMPAD7,
	VK_NUMPAD8,
	VK_NUMPAD9,
	VK_DIVIDE,
	VK_MULTIPLY,
	VK_ADD,
	VK_SUBTRACT,
	VK_DECIMAL,
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	VK_F1,
	VK_F2,
	VK_F3,
	VK_F4,
	VK_F5,
	VK_F6,
	VK_F7,
	VK_F8,
	VK_F9,
	VK_F10,
	VK_F11,
	VK_F12,
	VK_LEFT,
	VK_RIGHT,
	VK_UP,
	VK_DOWN,
	VK_ESCAPE,
	VK_LCONTROL,
	VK_LSHIFT,
	VK_LMENU,
	VK_LWIN,
	VK_LBUTTON,
	VK_RCONTROL,
	VK_RSHIFT,
	VK_RMENU,
	VK_RWIN,
	VK_RBUTTON,
	VK_SPACE,
	VK_PRIOR,
	VK_NEXT,
	VK_END,
	VK_HOME,
	VK_INSERT,
	VK_DELETE,
	VK_BACK,
	VK_RETURN
};


