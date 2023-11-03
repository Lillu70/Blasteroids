
#pragma once

#include "..\Utility\Primitives.h"
#include "..\Utility\Utility.h"
#include "..\Utility\Allocator.h"
#include "..\Utility\Assert.h"
#include "..\Platform\Platform_Interface.h"

#include "Terminus_Font.h"

#include "Pixel_Canvas.h"
#include "Pixel_Canvas.cpp"

#include "Random_Machine.h"

#include "Asteroid_Maths.h"

#include "Entity.h"
#include "Entity_Definations.h"

#include "Action.h"

#include "GUI.h"
#include "GUI.cpp"

#include "Game_State.h"
#include "Game_State.cpp"

#include "AI.h"
#include "AI.cpp"

#include "Particle_System.h"
#include "Particle_System.cpp"

#include "Weapon_Factory.h"
#include "Weapon_Factory.cpp"

#include "Mesh_Factory.h"

#include "Serialization.cpp"


static void init_asteroids_game(Platform_Call_Table platform_calltable, void* game_state_memory, u32 game_state_memory_size);
static void update_asteroids_game(f64 delta_time, bool& update_surface);

static void reset_game();
static void restart_game();
static void start_game();
static void physics_update();
static void process_timed_events();
static void check_lasers_againt_targets();
static void draw_game();
static void draw_ui();
static void draw_pause_menu();
static void remove_dead_entities();

static inline void add_player(u32 color, v2f p, Weapon::type weapon = Weapon::type::def);
static void record_ship_input();
static void clear_ship_input();

static Entity* spawn_player_ship(v2f pos, u32 color, f64 itime, Weapon::type weapon);
static Pickup generate_pickup();
static void spawn_pickup(Pickup pickup);
static void spawn_new_asteroid(Size size);
static Entity* create_asteroid(Size size);
static void create_enemy_ship(v2f pos, f32 facing_direction, Enemy_Ship::Type type, AI::State start_state = AI::State::face_movement_target);
static inline void spawn_enemy_ship(Enemy_Ship::Type type);

static inline bool check_interactions(Entity* a, Entity* b);
static void bullet_vs_asteroid(Entity* bullet, Entity* asteroid);
static void bullet_vs_bullet(Entity* bullet1, Entity* bullet2);
static void bullet_vs_ship(Entity* bullet_entity, Entity* ship_entity);
static void player_ship_vs_asteroid(Entity* player_ship, Entity* asteroid);
static void enemy_ship_vs_asteroid(Entity* enemy_ship, Entity* asteroid);
static void player_ship_vs_pickup(Entity* player_ship, Entity* pickup);

static inline void force_next_wave();
static inline f64 generate_wave();
static inline void generate_vector_that_crosses_play_area(v2f* position, v2f* velocity);
static v2f pick_enemy_target_location(v2f position, f32 desired_velocity);

static inline void process_global_actions();
static inline void process_global_game_keyboard_events(f64* delta_time);
static void pause_game();
static void unpause_game();
static void unpause_game_without_destroy_frame();
static inline void remove_ui_canvas();

static void generate_mesh_data(Linear_Allocator* alloc);


static void quit_to_desktop();
static void set_mode_main_menu();
static void set_mode_asteroids_sp();

static inline Action* get_action(Game_Actions action);
