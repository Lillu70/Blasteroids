
#pragma once


struct Particle
{
    v2f position = { 0, 0 };
    v2f velocity = { 0, 0 };
    f64 fade_start_time = 0;
    f64 life_time = 0;
    u32 full_color = 0;
};


struct Emission_Cone
{
    f32 direction_angle = 0;
    f32 half_radius = 0;
    f32 offset = 0;
};


struct Particle_Definition
{
    f32 min_speed = 0;
    f32 max_speed = 0;
    f64 fade_start_time = 0;
    f64 life_time = 0;
    
    u32 full_color = 0;
};


struct Particle_System
{
    static constexpr u32 max_particle_count = 5000;
    static inline Particle* particles = 0;
    u32 active_particle_count = 0;
    
    Random_Machine rm = {0};
};


static Particle_System particle_system = Particle_System();

static inline void particle_system_clear();

static void particle_system_emit(
    v2f source_position, 
    Emission_Cone cone, 
    Particle_Definition* pd, 
    u32 count);

static void particle_system_emit(Mesh* mesh, Particle_Definition* pd, u32 count);

static void particle_system_update_and_draw(Pixel_Canvas* canvas, f32 delta_time, f64 game_time, u32 background_color);
