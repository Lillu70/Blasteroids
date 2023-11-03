
#pragma once

#include <inttypes.h>
#include "Assert.h"

typedef uint8_t	u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t	i16;
typedef int32_t	i32;
typedef int64_t	i64;

typedef float f32;
typedef double f64;

typedef i32 b32;


constexpr f64 PI = 3.141592653589793;
constexpr f32 PI32 = f32(PI);
constexpr f32 HALF_PI32 = PI32 / 2;
constexpr f64 TAU = 6.283185307179586;
constexpr f32 TAU32 = f32(TAU);
constexpr u32 DEG_FULL_CIRCLE = 360;
constexpr u32 DEG_HALF_CIRCLE = DEG_FULL_CIRCLE / 2;
constexpr f32 F32_MAX = 3.402823466e+38F;
constexpr f64 F64_MAX = 1.7976931348623157E+308;
constexpr u32 U32_MAX = 0xFFFFFFFF;


#define WHITE 0xffffffff
#define BLACK 0xff000000
#define RED 0xffff0000
#define YELLOW 0xffffff00
#define GREEN 0xff00ff00
#define BLUE 0xff0000ff
#define MAGENTA 0xffff00ff


#define KiB 1024
#define MiB KiB * KiB


template<typename T>
struct v2
{
	v2() = default;
	v2(T xy) : x(xy), y(xy) {}
	v2(T x, T y) : x(x), y(y) {}
	v2(const v2<T>& other) : x(other.x), y(other.y) {}
	
	T x = 0;
	T y = 0;
	
	template<typename U>
	inline v2<U> As()
	{
		return v2<U>((U)x, (U)y);
	}
	
	v2<T> operator * (const v2<T>& other)	const	{ return { x * other.x, y * other.y };	}
	v2<T> operator * (const T& comp)		const	{ return { x * comp,	y * comp    };	}
	v2<T> operator / (const v2<T>& other)	const	{ return { x / other.x, y / other.y };	}
	v2<T> operator / (const T& comp)		const	{ return { x / comp,	y / comp    };	}
	v2<T> operator + (const v2<T>& other)	const	{ return { x + other.x, y + other.y };	}
	v2<T> operator + (const T& comp)		const	{ return { x + comp,	y + comp    };	}
	v2<T> operator - (const v2<T>& other)	const	{ return { x - other.x, y - other.y };	}
	v2<T> operator - (const T& comp)		const	{ return { x - comp,	y - comp    };	}
	v2<T> operator - ()						const	{ return { x * -1, y * -1};				}
	
	bool operator != (const v2<T>& other)	const	{ return (x != other.x || y != other.y);}
	bool operator == (const v2<T>& other)	const	{ return (x == other.x && y == other.y);}
	
	void operator = (const v2<T>& other)			{ x = other.x; y = other.y;				}
	void operator = (const T comp)					{ x = comp; y = comp;					}
	
	void operator *= (const v2<T>& other)			{ x *= other.x; y *= other.y;			}
	void operator *= (const T& comp)				{ x *= comp; y *= comp;		      		}
	void operator /= (const v2<T>& other)			{ x /= other.x; y /= other.y;			}
	void operator /= (const T& comp)				{ x /= comp; y /= comp;	       			}
	void operator += (const v2<T>& other)			{ x += other.x; y += other.y;			}
	void operator += (const T& comp)				{ x += comp; y += comp;					}
	void operator -= (const v2<T>& other)			{ x -= other.x; y -= other.y;			}
	void operator -= (const T& comp)				{ x -= comp; y -= comp;					}
};
	
	
typedef v2<i32> v2i;
typedef v2<u32> v2u;
typedef v2<f32> v2f;
	
	
struct Rect
{
	v2f min = { 0, 0 };
	v2f max = { 0, 0 };
};


static inline Rect create_rect_min_max(v2f min, v2f max)
{
	Rect result = { min, max };
	
	Assert(min.x < max.x);
	Assert(min.y < max.y);
	
	return result;
}


static inline Rect create_rect_center_halfdim(v2f center, v2f half_dim)
{
	Rect result = { center - half_dim, center + half_dim };
	return result;
}


static inline Rect create_rect_center(v2f center, v2f dim)
{
	v2f half_dim = dim * 0.5;
	Rect result = { center - half_dim, center + half_dim };
	return result;
}


static inline void add_offset_to_rect(v2f offset, Rect* rect)
{
	rect->min += offset;
	rect->max += offset;	
}
