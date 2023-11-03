
#pragma once

#include "Input.h"

enum class App_Flags
{
	is_running = 0,
	wants_to_exit,
	is_focused,
	is_fullscreen,
	cursor_is_visible,
};


struct Platform_Call_Table
{
	u32  (*get_window_width)() = 0;
	
	u32  (*get_window_height)() = 0;
	
	u32* (*get_pixel_buffer)() = 0;
	
	i32  (*get_pixel_buffer_width)() = 0;
	
	i32  (*get_pixel_buffer_height)() = 0;
	
	u32* (*resize_pixel_buffer)(i32 width, i32 height) = 0;
	
	u32  (*get_flags)() = 0;
	
	void (*set_flag)(App_Flags flag, bool val) = 0;
	
	bool (*get_keyboard_key_down)(Key_Code) = 0;
	
	Controller_State(*get_controller_state)(i32 controler_idx) = 0;
	
	v2i (*get_cursor_position)() = 0;
	
	f64 (*get_time_stamp)() = 0;
	
	u64 (*get_cpu_time_stamp)() = 0;
	
	u64 (*get_frame_count)() = 0;
	
	bool (*write_file)(const char*, u8*, u32) = 0;
	
	bool (*get_file_size)(const char*, u32*) = 0;
	
	bool (*read_file)(const char*, u8*, u32) = 0;
};
