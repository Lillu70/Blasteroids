

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

#include "math.h"

#if 1

template<typename T>
static inline T Max(T value, T max)
{
    T result = (value > max)? value : max;
    return result;
}


template<typename T>
static inline T Min(T value, T min)
{
    T result = (value < min)? value : min;
    return result;
}


template<typename T>
static inline void Swap(T* a, T* b)
{
    T temp = *a;
    
    *a = *b;
    *b = temp;
}


template<typename T>
static constexpr inline T Abs(T v)
{
    T result = v;
    if(result < 0)
        result *= -1;
    
    return result;
}
#endif


static inline f32 Inv(f32 value)
{
    f32 result = 1.f - value;
    return result;
}


static inline f64 Sin(f64 value)
{
    f64 result = sin(value);
    return result;    
}


static inline f32 Sin(f32 value)
{
    f32 result = sinf(value);
    return result;    
}


static inline f32 Cos(f32 value)
{
    f32 result = cosf(value);
    return result;    
}


static inline f32 Tan(f32 value)
{
    f32 result = tanf(value);
    return result;    
}


static inline f32 ArcTan2(f32 y, f32 x)
{
    f32 result = 0;
    result = atan2f(y, x);
    return result;
}


static inline f32 ArcTan2(v2f v)
{
    f32 result = ArcTan2(v.y, v.x);
    return result;
}


static inline f32 Root(f32 value)
{
    f32 result = sqrtf(value);
    return result;
}


static inline f32 Square(f32 value)
{
    f32 result = value * value;
    return result;
}


static inline f64 Square(f64 value)
{
    f64 result = value * value;
    return result;
}


static constexpr inline s32 Square(s32 value)
{
    s32 result = value * value;
    return result;
}


static inline f64 Cube(f64 value)
{
    f64 result = value * value * value;
    return result;
}


static inline f32 Length(v2s v)
{
    s32 result = Root(Square(v.x) + Square(v.y));
    return result;
}

static inline f32 Length(v2f v)
{
    f32 result = Root(Square(v.x) + Square(v.y));
    return result;
}


static inline f32 Length(v3f v)
{
    f32 result = Root(Square(v.x) + Square(v.y) + Square(v.z));
    return result;
}


static inline f32 Length_Squared(v2f v)
{
    f32 result = Square(v.x) + Square(v.y);
    return result;
}


static inline v2f Normalize(v2f v)
{
    f32 lenght = Length(v);
    v2f result =  {};
    if(lenght > 0)
    {
        result = v / lenght;
    }
    
    return result;
}

static inline v3f Normalize(v3f v)
{
    f32 lenght = Length(v);
    v3f result =  {};
    if(lenght > 0)
    {
        result = v / lenght;
    }
    
    return result;
}


// Inner Product
static inline f32 Dot_Product(v2f a, v2f b)
{
    f32 result = (a.x * b.x) + (a.y * b.y);
    return result;    
}


static inline f32 Sign(f32 v)
{
    u32 vv = *(u32*)&v;
    u32 sign = (vv >> 31) & 1;
    f32 result = 1.f + (f32(sign) * -2.f);
    
    return result;
}


// Opposite of truncate... What ever that should be called. Rounds away from zero.
static inline s32 Ceil_Away_From_Zero(f32 v)
{
    f32 v2 = Abs(v);
    f32 f  = v2 - f32(s32(v2));
    
    if(f > 0)
    {
        v += Sign(v);
    }
    
    s32 result = s32(v);
    return result;
}


// Rounds to the nearest whole number towards negative infinity.
static inline f32 Floor(f32 value)
{
    f32 result = f32((s32)value);
    if(result > value)
        result -= 1;
    
    return result;    
}


static inline f64 Floor(f64 value)
{
    f64 result = f64((s64)value);
    if(result > value)
        result -= 1;
    
    return result;    
}


static inline v2f Floor(v2f v)
{
    v2f result = { Floor(v.x), Floor(v.y) };
    return result;
}


static inline v2f Trunc(v2f v)
{
    v2f result = { f32(s32(v.x)), f32(s32(v.y)) };
    return result;
}


static inline f32 Trunc(f32 v)
{
    f32 result = f32(s32(v));
    return result;
}


// Rounds to the nearest whole number towards positive infinity.
static inline constexpr f32 Ceil(f32 value)
{
    f32 result = f32((s32)value);
    if(result < value)
        result += 1;
    
    return result;
}


// Rounds to the nearest whole number towards positive infinity.
static inline constexpr f64 Ceil(f64 value)
{
    f64 result = f64((s64)value);
    if(result < value)
        result += 1;
    
    return result;
}


static inline v2f Ceil(v2f v)
{
    v2f result = { Ceil(v.x), Ceil(v.y) };
    return result;
}


static inline f32 Round(f32 real)
{
    f32 shift = 0.5f;
    if(real < 0)
        shift *= -1;
    
    f32 result = (f32)((s32)(real + shift));
    return result;
}


static inline v2f Round(v2f v)
{
    v2f result = v2f{Round(v.x), Round(v.y)};
    return result;
}


static inline v2f Hadamar_Division(v2f a, v2f b)
{
    v2f result = {a.x / b.x, a.y / b.y};
    return result;
}


static inline v2f Hadamar_Product(v2f a, v2f b)
{
    v2f result = {a.x * b.x, a.y * b.y};
    return result;
}


static inline v3f Hadamar_Product(v3f a, v3f b)
{
    v3f result = {a.x * b.x, a.y * b.y, a.z * b.z};
    return result;
}


static inline f32 Componentwise_Mult(v2f v)
{
    f32 result = v.x * v.y;
    return result;
}


static inline f32 Componentwise_Add(v4f v)
{
    f32 result = v.x + v.y + v.z + v.w;
    return result;
}


static inline f32 Componentwise_Add(v2f v)
{
    f32 result = v.x + v.y;
    return result;
}


static inline s32 Round_To_Signed_Int32(f32 real)
{
    s32 result = (u32)(real + 0.5f);
    return result;
}


static inline v2s Round_To_Signed_Int32(v2f v)
{
    v += 0.5f;
    v2s result = v2f::Cast<s32>(v);    
    return result;
}


static inline f32 Lerp(f32 a, f32 b, f32 t)
{
    f32 result = a + t * (b - a);
    return result;
}


static inline v2f Lerp(v2f a, v2f b, f32 t)
{
    v2f result = { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
    return result;
}


static inline v3f Lerp(v3f a, v3f b, f32 t)
{
    v3f result = { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t) };
    return result;
}


static inline v4f Lerp(v4f a, v4f b, f32 t)
{
    v4f result = { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t), Lerp(a.w, b.w, t) };
    return result;
}


static inline f32 Clamp_To_Barycentric(f32 real)
{
    if(real != real) // NOTE: This is to deal with NaN.
        return 0.f;
    
    if(real > 1.f)
        return 1.f;
    
    if(real < 0.f)
        return 0.f;
    
    return real;
}


static inline f64 Clamp_To_Barycentric(f64 real)
{
    if(real != real) // NOTE: This is to deal with NaN.
        return 0.0;
    
    if(real > 1.0)
        return 1.0;
    
    if(real < 0.0)
        return 0.0;
    
    return real;
}


static inline u32 Noise_Squirrel3(s32 np, u32 seed)
{
    constexpr u32 BIT_NOISE1 = 0xB5297A3D;
    constexpr u32 BIT_NOISE2 = 0x68E31DA4;
    constexpr u32 BIT_NOISE3 = 0x1B56C4E9;
    
    s32 mangled = np;
    mangled *= BIT_NOISE1;
    mangled += seed;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOISE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOISE3;
    mangled ^= (mangled >> 8);
    return mangled;
}


static inline u32 Noise_Squirrel3_2D(v2s np, u32 seed)
{
    constexpr int PRIME_NUMBER = 198491317;
    return Noise_Squirrel3(np.x + (PRIME_NUMBER * np.y), seed);
}


static constexpr inline u32 PCG_Hash(u32 input)
{
    u32 state = input * 747796405u + 2891336453u;
    u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}


static inline f32 Clamp_Zero_To_One(f32 real)
{
    f32 result = Max(0.f, Min(1.f, real));
    return result;
}


static inline f32 Clamp_Zero_To_Max(f32 real, f32 max)
{
    f32 result = Max(0.f, Min(max, real));
    return result;
}


static inline f64 Powf64(f64 base, f64 exponent)
{
    f64 result = pow(base, exponent);
    return result;
}


static inline u32 Pow32(u32 base, u32 exp)
{
    u32 result = 1;
    for(;;)
    {
        if (exp & 1)
            result *= base;
        
        exp >>= 1;
        
        if (!exp)
            break;
        
        base *= base;
    }
    
    return result;
}


static inline f32 Distance(v2f p0, v2f p1)
{
    v2f v = p1 - p0;
    f32 result = Abs(Length(v));
    
    return result;
}


static inline s32 Distance(v2s p0, v2s p1)
{
    v2s v = p1 - p0;
    s32 result = Abs(Length(v));
    
    return result;
}


static inline v2f Perp_CW(v2f v)
{
    v2f result = {v.y, -v.x};
    return result;
}


static inline v2f Perp_CCW(v2f v)
{
    v2f result = {-v.y, v.x};
    return result;
}