// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


struct Target_Buffer_Sound_Sample
{
    s16 left;
    s16 right;
};


struct Sound
{
    union
    {
        void* memory;
        s16* channel_buffers[2];
    };
    
    u32 samples_per_channel;
};


struct Sound_ID
{
    u64 v;
};


enum class Play_Mode : u8
{
    once,
    loop
};


enum class Fade_Direction : u8
{
    none = 0,
    out,
    in,
};


namespace Sound_Types
{
    enum T : u8
    {
        music,
        effect,
        dialog,
        COUNT
    };    
};


namespace Sound_Flags
{
    enum T : u8
    {
        playing     = 1 << 0,
        muted       = 1 << 1,
        positional  = 1 << 2,
        looping     = 1 << 3,
        fading_in   = 1 << 4,
        fading_out  = 1 << 5,
    };
};


struct Mixer_Slot
{
    Sound* sound;
    Sound_ID id;
    f32 time_cursor;
    
    v2f pos; // 2D Sound
    
    f32 speed;
    f32 volume;
    
    f64 fade_start;
    f64 fade_end;
    
    Sound_Types::T type;
    u8 flags;
};


struct Sound_Player
{
    Mixer_Slot mixer_slots[32];
    u64 next_play_id;
    f32 master_volume;
    f32 time_scale;
    f32 hearing_distance;
    f32 ear_seperation;
    v2f listener_location;
    f32 volumes[Sound_Types::COUNT];
    
    bool muted;
    bool paused;
};


void Set_Listener_Location(v2f p);

bool Update_Sound_Position(Sound_ID id, v2f new_pos);

Sound_ID Play_Sound(Sound* sound, v2f* pos = 0, Play_Mode play_mode = Play_Mode::once, Sound_Types::T type = Sound_Types::effect, f32 volume = 1.0f, f32 speed = 1.0f);

Sound_ID Play_Sound(Sound* sound, v2f* pos, Play_Mode play_mode, Sound_Types::T type, Range volume_range, Range speed_range);

bool Fade_Sound(Sound_ID id, f64 fade_in_time, Fade_Direction direction);

bool Stop_Sound(Sound_ID id);

// TODO: Clear up the terminology.
// Sounds have multiple states and "stop" here is more like remove from the mixer and free up the slot.
void Stop_All_Sounds();
void Stop_All_Sounds_Of_Type(Sound_Types::T type);

void Continue_All_Sounds_Of_Type(Sound_Types::T type);
void Pause_All_Sounds_Of_Type(Sound_Types::T type);

void Set_Volume(f32 _volume);

void Make_Sin_Wave_Sound(Sound* out_sound, u32 samples_per_second, u32 lenght_seconds, Allocator_Shell* allocator);

Sound Load_Wave(char* file_path, Linear_Allocator* serialization_arena, Allocator_Shell* allocator);
