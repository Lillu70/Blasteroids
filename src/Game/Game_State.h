
#pragma once

#include "Action_Definations.h"
#include "Score.cpp"


struct Timed_Event;
struct Player;
struct Laser;

static constexpr u32 max_asteroid_count = 30;
static constexpr u32 max_enemy_ship_count = 3;

struct Game
{
    static constexpr u32 max_entity_count = 256;
    static inline Entity* entities = 0;
    
	u32 active_entity_count = 0;
	u32 zombie_entity_count = 0;
	
    static constexpr u32 max_player_count = 4;
    static inline Player* player_table = 0;
    u32 active_player_count = 0;

    static constexpr u32 max_laser_count = 10;
    static inline Laser* laser_table = 0;
    u32 active_laser_count = 0;
    
	static inline v2i asteroid_area_start = 0;
    static inline v2i asteroid_area_end = 0;

    static inline Timed_Event* timed_events = 0;
    u32 timed_event_count = 0;
    u32 max_timed_event_count = 16;

    u32 wave_count = 0;
	u32 last_enemy_wave = 0;
	u32 active_asteroid_count = 0;
	u32 active_enemy_ship_count = 0;
	u32 pickup_count = 0;

    Random_Machine rm;
    
    f32 update_tick = 1 / 120.f;
    f32 draw_frequency = 1 / 60.f;
	
	f64 total_pause_time = 0;
    f64 pause_time_start = 0;
    f64 physics_time = 0;
    f64 game_time = 0;
    f64 pickup_time_stop_time = 0;
	f64 next_draw_time = 0;
	
	GUI_Handler gui_handler = GUI_Handler();
	
	// TODO: Make these flags.
    bool draw_ui = true;
    bool is_paused = false;
    
	u32 get_active_hostile_count()
	{
		u32 result = active_asteroid_count + active_enemy_ship_count;
		return result;
	}
	
	u32 get_next_entity_id() { return next_entity_id++; }
	u32 get_next_player_id() { return next_player_id++; }
	
private:
	u32 next_entity_id = 1;
	u32 next_player_id = 1;
};


struct Settings
{
    f32 sfx_volume = 0.8f;
    f32 music_volume = 0.8f;
    
    //CONSIDER: Compres the bools in a flags field 
    //(because of padding, only worth while if there are more than 4 bools).
    bool is_muted = 0;
    
    bool dirty = 0;
};


enum class Event_Type : u32
{
    spawn_ship,
    spawn_wave,
    game_over,
    game_over_highscore,
    restart_game,
    spawn_pickups,
	debug_respawn_enemy_ship,
};


struct Timed_Event
{  
    f64 trigger_time;
    Event_Type type;
    u8 payload[12];
};


struct Transient_Data
{
    static constexpr u32 enemy_mesh_count = 3;
    Mesh enemy_meshes[enemy_mesh_count];
	f32 enemy_mesh_widths[enemy_mesh_count];
	
	Mesh ship_mesh;
	f32 ship_mesh_width = 0;
	
    Mesh pickup_mesh;
	
	v2u pixel_buffer_dimensions = {0, 0};
	u32* pixel_buffer = 0;
};


enum class Game_Mode : u32
{
	none = 0,
	asteroids_sp,
	main_menu,
};


static Game game;

static Pixel_Canvas canvas;

static Pixel_Canvas ui_canvas;

static Pixel_Canvas screen_canvas;

static Platform_Call_Table platform;

static Transient_Data transient;

static General_Allocator mem;

static GUI_Theme s_gui_theme = GUI_Theme();

static Game_Mode s_game_mode = Game_Mode::none;

static Settings s_settings = Settings();


static inline Entity* add_entity(Entity_Type type);

static inline void kill_entity(Entity* entity);

static bool find_entity_by_id(u32 entity_id, u32* out_idx);

static bool find_entity_by_id(u32 entity_id, Entity** out_entity);

static inline void add_timed_event(Timed_Event event);

static Player* find_ship_owner(Ship* ship);

static inline void murder_entity(Entity* entity, Entity* murderer);

static inline void murder_entity(Entity* entity, u32 murderer_id);