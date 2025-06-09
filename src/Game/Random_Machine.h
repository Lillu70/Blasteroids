
#pragma once

static inline u32 noise_squirrel3(s32 position, u32 seed)
{
    constexpr u32 BIT_NOISE1 = 0xB5297A3D;
    constexpr u32 BIT_NOISE2 = 0x68E31DA4;
    constexpr u32 BIT_NOISE3 = 0x1B56C4E9;
    
    s32 mangled = position;
    mangled *= BIT_NOISE1;
    mangled += seed;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOISE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOISE3;
    mangled ^= (mangled >> 8);
    return mangled;
}


struct Random_Machine
{
    s32 noise_position = 0;
    static inline s32 seed = 1;
    
    u32 random_u32(u32 max)
    {
        u32 result = (noise_squirrel3(noise_position++, seed) % max);
        return result;
    }
    
    f32 random_f32()
    {
        u32 r = noise_squirrel3(noise_position++, seed) % 1000;
        
        f32 result = ((f32)r / (f32)1000);
        
        return result;
    } 
};
static Random_Machine s_global_random_machine = {};