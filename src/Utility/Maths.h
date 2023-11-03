
#pragma once

#include <utility>
#include <cmath>
#include <limits>

#include "Primitives.h"


template<typename T>
inline T min(T a, T b) { return a < b ? a: b; }


template<typename T>
inline T max(T a, T b) { return a > b ? a : b; }

/*
template<typename T>
inline T abs(T val) { return val >= 0 ? val : val * -1; }
*/

inline f32 angle_between_points(v2f a, v2f b)
{
    return atan2f(a.y - b.y, a.x - b.x);
}

template<typename T>
inline auto square(T val)
{
    return val * val;
}

u32 pow(u32 base, u32 exp)
{
    u32 result = 1;
    for (;;)
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

inline bool line_intersection(v2f l0p0, v2f l0p1, v2f l1p0, v2f l1p1, v2f& hit_location)
{
    v2f s1;
    s1.x = l0p1.x - l0p0.x;
    s1.y = l0p1.y - l0p0.y;

    v2f s2;
    s2.x = l1p1.x - l1p0.x;
    s2.y = l1p1.y - l1p0.y;

    f32 x = -s2.x * s1.y + s1.x * s2.y;
    f32 s = (-s1.y * (l0p0.x - l1p0.x) + s1.x * (l0p0.y - l1p0.y)) / x;
    f32 t = (s2.x * (l0p0.y - l1p0.y) - s2.y * (l0p0.x - l1p0.x)) / x;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
	hit_location.x = l0p0.x + (t * s1.x);
	hit_location.y = l0p0.y + (t * s1.y);
	return true;
    }

    return false;
}


template<typename T>
inline f32 squared_distance(v2<T> a, v2<T> b)
{
	f32 result = (f32)(square(a.x - b.x) + square(a.y - b.y));
	return result;
}


template<typename T>
inline f32 distance(v2<T> a, v2<T> b)
{
	f32 result = sqrtf(squared_distance(a, b));
    return result;
}


template<typename T>
inline f32 slope(v2<T> a, v2<T> b)
{					  
    return  (f32)(b.y - a.y) / (f32)(b.x - a.x);  
}

template<typename T>
inline f32 magnitude(v2<T> a)
{
    return sqrtf(square(a.x) + square(a.y));
}

template<typename T>
inline v2<T> normalize(v2<T> a)
{
    f32 m = magnitude(a);
    return v2<T>(a.x / m, a.y / m);
}

template<typename T>
inline v2<T> rotate_point(v2<T> p, f32 a)
{
    f32 c = cosf(a);
    f32 s = sinf(a);
    return v2<T>(p.x * c - p.y * s, p.x * s + p.y * c);
}

template<typename T>
inline v2<T> rotate_point(v2<T> p, f32 cos_a, f32 sin_a)
{
    return v2<T>(p.x * cos_a - p.y * sin_a, p.x * sin_a + p.y * cos_a);
}

template<typename T>
inline v2<T> flip_y(v2<T> p)
{
    return { p.x, p.y * -1 };
}

//Returns y, for the provided x.
template<typename T>
inline T linear_interpolation(v2<T> p1, v2<T> p2, T x)
{
    return p1.y + (T)(slope(p1, p2) * (x - p1.x));
}

inline u32 multiply_accross_color_channels(u32 color, f32 mult)
{
    u8* p = (u8*)&color;
    for (u32 i = 0; i < 3; i++)
    {
		i32 mult_val = (i32)((*(p + i)) * mult);
		i32 val = min(255, max(0, mult_val));
		*(p + i) = (u8)val;
    }

    return color;
}


inline u32 multiply_accross_color_channels(u32 color, u32 min_values, f32 mult)
{
    u8* p = (u8*)&color;
	u8* m = (u8*)&min_values;
    for (u32 i = 0; i < 3; i++)
    {
		i32 mult_val = (i32)((*(p + i)) * mult);
		i32 val = min(255, max((i32)(*(m+i)), mult_val));
		*(p + i) = (u8)val;
    }

    return color;
}



inline static i32 round_to_int(f32 real)
{
    return (i32)(real + 0.5f);
}


inline static f32 rad_to_deg(f32 radian_value)
{
    return radian_value * DEG_HALF_CIRCLE / PI32;
}


inline static f32 deg_to_rad(f32 degree_value)
{
    return degree_value * PI32 / DEG_HALF_CIRCLE;
}


inline static f32 dot2(v2f a, v2f b)
{
    return a.x * b.x + a.y * b.y;
}


inline static v2f v2_average(v2f a, v2f b)
{
	v2f result = v2f{a.x + b.x, a.y + b.y} * 0.5f;
	return result;
}

static inline bool rects_overlap(Rect a_rect, Rect b_rect)
{
	
	Assert(a_rect.min.x < a_rect.max.x);
	Assert(a_rect.min.y < a_rect.max.y);
	
	Assert(b_rect.min.x < b_rect.max.x);
	Assert(b_rect.min.y < b_rect.max.y);
	
	
	if(a_rect.max.x < b_rect.min.x)
		return false;
	
	if(a_rect.max.y < b_rect.min.y)
		return false;
	
	if(a_rect.min.x > b_rect.max.x)
		return false;
	
	if(a_rect.min.y > b_rect.max.y)
		return false;

	return true;
}


static inline bool rects_overlap(v2f a_offset, Rect a_rect, v2f b_offset, Rect b_rect)
{
	add_offset_to_rect(a_offset, &a_rect);
	add_offset_to_rect(b_offset, &b_rect);
	
	return rects_overlap(a_rect, b_rect);
}


static inline bool point_inside_rect(v2f p, Rect rect)
{
	if(p.x < rect.min.x)
		return false;
	if(p.y < rect.min.y)
		return false;
	if(p.x > rect.max.x)
		return false;
	if(p.y > rect.max.y)
		return false;
	
	return true;
}