
//#define _DB

#include "Platform/Win32.cpp"
#include "Game/Asteroids.cpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{    
    win32_init(instance, 620, 480, CW_USEDEFAULT, CW_USEDEFAULT, (char*)"Blasteroids");
	
	Platform_Call_Table call_table = win32_get_call_table();
    init_asteroids_game(call_table, s_app.game_state_memory, GAME_STATE_MEMORY_SIZE);

    f64 frame_time = 0;
    
    while(is_flag_set(s_app.flags, (u32)App_Flags::is_running))
    {
		win32_flush_events();

		bool update_surface = false;
		update_asteroids_game(frame_time, update_surface);

		win32_update_surface(update_surface);
		
		frame_time = win32_update_frame_time();
    }
    
    return 0;
}