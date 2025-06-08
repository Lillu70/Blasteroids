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
    ones,
    loop
};


Sound_ID Play_Sound(Sound* sound, Play_Mode play_mode = Play_Mode::ones, f32 volume = 1.0f, f32 speed = 1.0f);

bool Stop_Sound(Sound_ID id);

void Set_Volume(f32 _volume);

void Make_Sin_Wave_Sound(Sound* out_sound, u32 samples_per_second, u32 lenght_seconds, Allocator_Shell* allocator);

Sound Load_Wave(char* file_path, Linear_Allocator* serialization_arena, Allocator_Shell* allocator);
