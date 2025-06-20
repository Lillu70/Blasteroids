// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once


static inline Sound_Player Create_Default_Sound_Player()
{
    Sound_Player sp = {};
    sp.next_play_id = 1;
    sp.master_volume = 1.f;
    sp.hearing_distance = 700.f;
    sp.ear_seperation = 100.f;
    sp.time_scale = 1;
    
    for(u32 i = 0; i < Array_Lenght(sp.volumes); ++i)
    {
        sp.volumes[i] = 1.f;
    }
    
    return sp;
}


static Sound_Player s_sound_player = Create_Default_Sound_Player();


void Set_Listener_Location(v2f p)
{
    s_sound_player.listener_location = p;
}


Mixer_Slot* Find_Sound_By_ID(Sound_ID id)
{
    Mixer_Slot* result = 0;
    for(s32 i= 0; i < Array_Lenght(s_sound_player.mixer_slots); ++i)
    {
        Mixer_Slot* slot = s_sound_player.mixer_slots + i;
        
        if(slot->id.v == id.v)
        {
            result = slot;
            break;
        }
    }
    
    return result;
}


bool Update_Sound_Position(Sound_ID id, v2f new_pos)
{
    bool result = false;
    
    Mixer_Slot* slot = Find_Sound_By_ID(id);
    if(slot && slot->id.v)
    {
        slot->pos = new_pos;
        slot->flags |= Sound_Flags::positional;
    }
    
    return result;
}


Sound_ID Play_Sound(Sound* sound, v2f* pos, Play_Mode play_mode, Sound_Types::T type, f32 volume, f32 speed)
{
    Sound_ID id = {};
    
    if(sound->memory)
    {
        Assert(sound->samples_per_channel);
        
        Mixer_Slot* slot = Find_Sound_By_ID({});
        if(slot)
        {
            *slot = {};
            id.v = ++s_sound_player.next_play_id;
            
            u8 flags = Sound_Flags::playing;
            if(play_mode == Play_Mode::loop)
            {
                flags |= Sound_Flags::looping;
            }
            
            slot->id = id;
            slot->sound = sound;
            
            slot->type = type;
            slot->speed = speed;
            slot->volume = volume;
            
            if(pos)
            {
                slot->pos = *pos;
                flags |= Sound_Flags::positional;
            }
            
            slot->flags = flags;
        }
        else
        {
            // There are no empty slots...
            // What to do here???
            // - A) Override one of the sounds?
            // - B) Just skip playing this one?
            
            // Right not just crash to notify that this did indeed happen.
            Terminate;
        }
    }
    
    return id;
}


Sound_ID Play_Sound(Sound* sound, v2f* pos, Play_Mode play_mode, Sound_Types::T type, Range volume_range, Range speed_range)
{
    f32 volume = Random_From_Range(volume_range);
    f32 speed  = Random_From_Range(speed_range);
    
    Sound_ID result = Play_Sound(sound, pos, play_mode, type, volume, speed);
    
    return result;
}


bool Stop_Sound(Sound_ID id)
{
    bool result = false;
    
    Mixer_Slot* slot = Find_Sound_By_ID(id);
    if(slot)
    {
        *slot = {};
        result = true;
    }
    
    return result;
}


bool Fade_Sound(Sound_ID id, f64 fade_in_time, Fade_Direction direction)
{
    bool result = false;
    
    Mixer_Slot* slot = Find_Sound_By_ID(id);
    if(slot && slot->id.v)
    {
        slot->fade_start = Platform_Get_Time_Stamp();
        slot->fade_end = slot->fade_start + fade_in_time;
        
        u8 clear_mask = Sound_Flags::fading_in | Sound_Flags::fading_out;
        Inverse_Bit_Mask(&slot->flags, clear_mask);
        switch(direction)
        {
            case Fade_Direction::in:
            {
                slot->flags |= Sound_Flags::fading_in;
            }break;
            
            case Fade_Direction::out:
            {
                slot->flags |= Sound_Flags::fading_out;
            }break;
        }
        
        Assert(!((slot->flags & Sound_Flags::fading_in) && (slot->flags & Sound_Flags::fading_out)));
        
        slot->flags |= Sound_Flags::playing;
        
        result = true;
    }
    
    return result;
}


void Stop_All_Sounds()
{
    for(s32 i = 0; i < Array_Lenght(s_sound_player.mixer_slots); ++i)
    {
        Sound_ID id = s_sound_player.mixer_slots[i].id;
        if(id.v)
        {
            Stop_Sound(id);
        }
    }
}


void Stop_All_Sounds_Of_Type(Sound_Types::T type)
{
    for(s32 i = 0; i < Array_Lenght(s_sound_player.mixer_slots); ++i)
    {
        Mixer_Slot* slot = s_sound_player.mixer_slots + i;
        
        Sound_ID id = slot->id;
        if(id.v && slot->type == type)
        {
            Stop_Sound(id);
        }
    }
}


void Pause_All_Sounds_Of_Type(Sound_Types::T type)
{
    for(s32 i = 0; i < Array_Lenght(s_sound_player.mixer_slots); ++i)
    {
        Mixer_Slot* slot = s_sound_player.mixer_slots + i;
        
        Sound_ID id = slot->id;
        if(id.v && slot->type == type)
        {
            Inverse_Bit_Mask(&slot->flags, Sound_Flags::playing);
        }
    }
}


void Continue_All_Sounds_Of_Type(Sound_Types::T type)
{
    for(s32 i = 0; i < Array_Lenght(s_sound_player.mixer_slots); ++i)
    {
        Mixer_Slot* slot = s_sound_player.mixer_slots + i;
        
        Sound_ID id = slot->id;
        if(id.v && slot->type == type)
        {
            slot->flags |= Sound_Flags::playing;
        }
    }
}


void Output_Sound(Target_Buffer_Sound_Sample* buffer, u64 sample_count, u32 samples_per_second)
{
    if(!s_sound_player.paused)
    {
        f32 sample_time = 1.0f / f32(samples_per_second); // seconds
        f32 master_volume = s_sound_player.master_volume;
        
        f32 half_ear_sep = s_sound_player.ear_seperation / 2;
        v2f listerner_pos = s_sound_player.listener_location;
        f32 hearing_d = s_sound_player.hearing_distance;
        v2f lpos = AddX(listerner_pos, -half_ear_sep);
        v2f rpos = AddX(listerner_pos, half_ear_sep);
        
        f32 time_scale = s_sound_player.paused? 0 : s_sound_player.time_scale;
        
        f64 now = Platform_Get_Time_Stamp();

        for(s32 i = 0; i < Array_Lenght(s_sound_player.mixer_slots); ++i)
        {
            Mixer_Slot* slot = s_sound_player.mixer_slots + i;
            if(slot->id.v && slot->sound->channel_buffers[0] && (slot->flags & Sound_Flags::playing))
            {
                u32 type = u32(slot->type);
                Assert(type < u32(Sound_Types::T::COUNT));
                
                f32 type_volume = s_sound_player.volumes[type];
                f32 volume = type_volume * slot->volume * master_volume;
                if((slot->flags & Sound_Flags::fading_in) || (slot->flags & Sound_Flags::fading_out))
                {
                    f32 r = slot->fade_end - slot->fade_start;
                    f32 t2 = (now - slot->fade_start) / r;
                    f32 t = Clamp_Zero_To_One(t2);
                    if(slot->flags & Sound_Flags::fading_out)
                    {
                        t = Inv(t);
                        
                        // pause or kill sound after it's been faded out.
                        if(t2 > 1.0)
                        {
                            if(slot->flags & Sound_Flags::looping)
                            {
                                u8 mask = Sound_Flags::fading_out | Sound_Flags::playing;
                                Inverse_Bit_Mask(&slot->flags, mask);
                            }
                            else
                            {
                                Stop_Sound(slot->id);
                            }
                            
                            continue; // <--- NOTE THE CONTINUE HERE!
                        }
                    }
                    else
                    {
                        // Remove the fade in flag after it's been faded in.
                        if(t2 > 1.0)
                        {
                            Inverse_Bit_Mask(&slot->flags, Sound_Flags::fading_in);
                        }
                    }
                    
                    volume *= t;
                }
                
                volume *= Inv(s_sound_player.muted);
                
                f32 rd = Distance(rpos, slot->pos);
                f32 ld = Distance(lpos, slot->pos);
                
                bool has_position = slot->flags & Sound_Flags::positional;
                
                f32 r_volume = has_position? Inv(Clamp_Zero_To_One(rd / hearing_d)) : 1.f;
                f32 l_volume = has_position? Inv(Clamp_Zero_To_One(ld / hearing_d)) : 1.f;
                
                for(Target_Buffer_Sound_Sample* sample = buffer; sample < buffer + sample_count; ++sample)
                {
                    u64 sample_cursor = (u64)Round(slot->time_cursor * f32(samples_per_second));
                    if(sample_cursor < slot->sound->samples_per_channel)
                    {
                        s16 sample_c1 = *(slot->sound->channel_buffers[0] + sample_cursor);
                        s16 sample_c2 = *(slot->sound->channel_buffers[1] + sample_cursor);
                        
                        sample->left += s16(f32(sample_c1) * volume * l_volume);
                        sample->right += s16(f32(sample_c1) * volume * r_volume);
                        
                        slot->time_cursor += sample_time * slot->speed * time_scale;
                    }
                    else
                    {
                        if(slot->flags & Sound_Flags::looping)
                        {
                            slot->time_cursor = 0;
                        }
                        else
                        {
                            *slot = {};
                            break;
                        }
                    }
                }
            }
        }
    }
}


void Make_Sin_Wave_Sound(Sound* out_sound, u32 samples_per_second, u32 lenght_seconds, Allocator_Shell* allocator)
{
    *out_sound = {};
    
    u32 sample_size = sizeof(Target_Buffer_Sound_Sample);
    u32 total_samples = samples_per_second * lenght_seconds;
    
    out_sound->samples_per_channel = total_samples;
    out_sound->memory = allocator->push(u32(total_samples * sample_size) / 2);
    out_sound->channel_buffers[0] = (s16*)out_sound->memory;
    out_sound->channel_buffers[1] = (s16*)out_sound->memory;
    
    u16 hrz = 240;
    u16 wave_period = samples_per_second / hrz;
    f32 P = 5000.0;
    
    for(u32 i = 0; i < total_samples; ++i)
    {
        s16 v = s16(Sin((i % wave_period) / f32(wave_period) * TAU32) * (Inv(i / f32(total_samples)) * P));
        out_sound->channel_buffers[0][i] = v;
        // out_sound->channel_buffers[1][i] = v;
    }
}


#pragma pack(push, 1)
struct Wave_Master_RIFF_Chunk
{
    u32 chunk_id;              // 4  Chunk ID: RIFF
    u32 chunk_size;            // 4  Chunk size: 4+n
    u32 wave_id;               // 4  WAVE ID: WAVE
};


struct Wave_FMT_Chunk
{
    u32 chunk_id;              //  4   Chunk ID: "fmt " 
    u32 chunk_size;            //  4   Chunk size: 16, 18 or 40
    u16 format_tag;            //  2   Format code
    u16 channels_count;        //  2   Number of interleaved channels
    u32 samples_per_sec;       //  4   Sampling rate (blocks per second)
    u32 avg_bytes_per_sec;     //  4   Data rate
    u16 block_align;           //  2   Data block size (bytes)
    u16 bits_per_sample;       //  2   Bits per sample
    u16 cb_size;               //  2   Size of the extension (0 or 22)
    u16 valid_bits_per_sample; //  2   Number of valid bits
    u32 channel_mask;          //  4   Speaker position mask
    u8 sub_format[16];         //  16  GUID, including the data format code
};


struct Wave_Data_Chunk
{
    u32 id;                     // 4  Chunk ID: data
    u32 size;                   // 4  Chunk size: n
};

#define WAVE_FORMAT_PCM         0x0001 // PCM
#define WAVE_FORMAT_IEEE_FLOAT  0x0003 // IEEE float
#define WAVE_FORMAT_ALAW        0x0006 // 8-bit ITU-T G.711 A-law
#define WAVE_FORMAT_MULAW       0x0007 // 8-bit ITU-T G.711 µ-law
#define WAVE_FORMAT_EXTENSIBLE  0xFFFE // Determined by SubFormat
#define WAVE_WAVE_TAG           u32(('W' << 0) | ('A' << 8) | ('V' << 16) | ('E' << 24)) // "WAVE"
#define WAVE_RIFF_CHUNK         u32(('R' << 0) | ('I' << 8) | ('F' << 16) | ('F' << 24)) // "RIFF"
#define WAVE_FMT_CHUNK          u32(('f' << 0) | ('m' << 8) | ('t' << 16) | (' ' << 24)) // "fmt "
#define WAVE_DATA_CHUNK         u32(('d' << 0) | ('a' << 8) | ('t' << 16) | ('a' << 24)) // "data"
#define WAVE_FACT_CHUNK         u32(('f' << 0) | ('a' << 8) | ('c' << 16) | ('t' << 24)) // "fact"

#pragma pack(pop)


Sound Load_Wave(char* file_path, Linear_Allocator* serialization_arena, Allocator_Shell* allocator)
{
    Sound result = {};
    
    u32 file_size;
    if(Platform_Get_File_Size(file_path, &file_size))
    {
        if(serialization_arena->capacity < file_size)
        {
            serialization_arena->init(file_size);
        }
        
        serialization_arena->clear();
        
        
        Wave_FMT_Chunk* fmt = 0;
        
        u8* buffer = (u8*)serialization_arena->push(file_size);
        u8* read_head = buffer;
        if(Platform_Read_File(file_path, (u8*)buffer, file_size))
        {
            Wave_Master_RIFF_Chunk* riff = (Wave_Master_RIFF_Chunk*)read_head;
            read_head += sizeof(*riff);
            
            if(riff->chunk_id == WAVE_RIFF_CHUNK && riff->wave_id == WAVE_WAVE_TAG)
            {
                Wave_Data_Chunk* chunk = (Wave_Data_Chunk*)read_head;
                read_head += (chunk->size + sizeof(*chunk) + 1) & ~1;
                
                if(chunk->id == WAVE_FMT_CHUNK)
                {
                    fmt = (Wave_FMT_Chunk*)chunk;   
                    Assert(fmt->format_tag == WAVE_FORMAT_PCM);
                    Assert(fmt->channels_count == 2 || fmt->channels_count == 1);
                    Assert(fmt->samples_per_sec == Platform_Get_Sound_Samples_Per_Second());
                    Assert(fmt->bits_per_sample == 16);
                }
                
                chunk = (Wave_Data_Chunk*)read_head;
                
                if(chunk->id == WAVE_DATA_CHUNK)
                {
                    s16* data = (s16*)(chunk + 1);
                    
                    switch(fmt->channels_count)
                    {
                        case 1:
                        {
                            result.samples_per_channel = chunk->size / sizeof(s16);
                            result.memory = allocator->push(chunk->size);
                            Mem_Copy(result.memory, data, chunk->size);
                            
                            for(u32 i = 0; i < (Array_Lenght(result.channel_buffers)); ++i)
                            {
                                result.channel_buffers[i] = (s16*)result.memory;
                            }
                            
                        }break;
                        
                        case 2:
                        {
                            result.samples_per_channel = chunk->size / sizeof(s16) / 2;
                            
                            result.memory = allocator->push(chunk->size);
                            result.channel_buffers[0] = (s16*)result.memory;
                            result.channel_buffers[1] = result.channel_buffers[0] + (chunk->size / sizeof(s16) / 2);
                            
                            for(u32 i = 0; i < result.samples_per_channel; ++i)
                            {
                                result.channel_buffers[0][i] = *(data + (i * fmt->channels_count));
                                result.channel_buffers[1][i] = *(data + (i * fmt->channels_count) + 1);
                            }
                            
                        }break;
                    }
                }
            }
        }
    }
    
    return result;
}