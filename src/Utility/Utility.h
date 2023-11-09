
#pragma once

#include "Primitives.h"

static inline u32 put_color(u8 r, u8 g, u8 b, u8 a = 0xff)
{
	return u32(a << 24) | u32(r << 16) | u32(g << 8) | u32(b << 0);
}


struct Mesh
{
	v2f* data = 0;
	u32 p_count = 0;
};


// Returns the lenght of the buffer, excluding the null terminator.
static u32 null_terminated_buffer_lenght(u8* buffer)
{
	u8* b;
	for(b = buffer; *b != 0; ++b){}
	return u32(b - buffer);
}

b32 is_flag_set(u32 field, u32 shift)
{
	return field & 1 << shift;
}
