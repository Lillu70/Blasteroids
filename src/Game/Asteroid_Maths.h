
#pragma once

#include "..\Utility\Maths.h"
#include "..\Utility\Utility.h"
#include "..\Utility\Assert.h"
#include "Random_Machine.h"


static void rotate_mesh(v2f* ref, v2f* mesh, u32 p_count, f32 cos, f32 sin)
{
	for(i32 i = 0; i < p_count; ++i)
		mesh[i] = rotate_point(ref[i], cos, sin);
}


static void rotate_local_mesh(v2f* local_mesh, Mesh mesh, f32 new_orientation)
{
	f32 cos = cosf(new_orientation);
	f32 sin = sinf(new_orientation);
	
	for(i32 i = 0; i < mesh.p_count; ++i)
		local_mesh[i] = rotate_point(mesh.data[i], cos, sin);
}


static void rotate_local_mesh(v2f* local_mesh, Mesh mesh, f32 cos, f32 sin)
{
	for(i32 i = 0; i < mesh.p_count; ++i)
		local_mesh[i] = rotate_point(mesh.data[i], cos, sin);
}


static u8* u32_to_char_buffer(u8* buffer, u32 buffer_size, u32 integer)
{
	// TODO: This works, but's odd and not very intuitive, so rethink and rework this.
	
	Assert(buffer_size == 11);
	
	buffer[buffer_size - 1] = 0;
	
	u32 ascii_numeric_offset = 48;
	u32 last_non_zero = buffer_size - 2;
	for(u32 i = 0; i < buffer_size - 1; ++i)
	{
		u32 digit = 0;
		if(i > 0)
			digit = (u32)(integer / pow(10, i)) % 10;	
		else
			digit = integer % 10;
		u32 write_pos = buffer_size - 2 - i;
		if(digit)
			last_non_zero = write_pos;
		
		buffer[write_pos] = ascii_numeric_offset + digit;
	}
	
	return buffer + last_non_zero;
}


static inline void update_position(v2f* position, v2f* velocity, v2f acceleration, f32 t)
{
	Assert(position && velocity);
	
	//P1 (position) (1/2)a*(t*t)+v*t+p<(old position)
	//P2 (velocity) a*t+v<(old velocity)
	//P3 (acceleration) input
	
	v2f p = *position;
	v2f a = acceleration;
	v2f v = a * t + *velocity;
	*velocity = v;
	*position = a * 0.5f * square(t) + v * t + p;
}


static inline void update_position(v2f* position, v2f* velocity, f32 t)
{
	*position =  (*velocity) * t + (*position);
}


static inline void clamp_position_to_rect(v2f* position, v2i end)
{
	while(position->x < 0)
		position->x += end.x;
	
	while(position->y < 0)
		position->y += end.y;
	
	while(position->x > end.x - 1)
		position->x -= end.x;
	
	while(position->y > end.y - 1)
		position->y -= end.y;
}


static inline void clamp_position_to_rect(v2f* position, v2i start, v2i end)
{
	i32 width = end.x - start.x;
	i32 height = end.y - start.y;
	
	while(position->x < start.x)
		position->x += width;
	
	while(position->y < start.y)
		position->y += height;
	
	while(position->x > end.x)
		position->x -= width;
	
	while(position->y > end.y)
		position->y -= height;
}


static inline void build_rect_mesh(v2f left_offset, v2f right_offset, v2f length, v2f* target)
{
	target[0] = left_offset;  
	target[1] = left_offset + length;
	target[2] = right_offset + length;
	target[3] = right_offset;
}


static inline void build_rect_mesh(v2f p_offset, v2f left_offset, v2f right_offset, v2f length, v2f* target)
{
	target[0] = p_offset + left_offset;  
	target[1] = p_offset + left_offset + length;
	target[2] = p_offset + right_offset + length;
	target[3] = p_offset + right_offset;
}


static inline bool vector_intersects_axis(f32 p_test, f32 p_cross, f32 v_test, f32 v_cross, f32 w, f32 cross_min, f32 cross_max)
{
	f32 t = (w - p_test) / v_test;
	f32 cross = t * v_cross + p_cross;
	return (cross >= cross_min && cross <= cross_max);
}


static inline f32 vector_intersects_axis(f32 p, f32 d, f32 w)
{
	return (w - p) / d;
}

//TODO: Rename this!
// and add the tau_minus_input to after the function call,
// or under some other function.

//NOTE: This is stupidly named. Beware of what it does.
// It return a value between 0 - PI32, correct for distances on the circle, 
// but not good for actual angle wrapping as the name suggests.
static inline f32 radian_wrap(f32 input)
{
	while(input < 0)
		input += TAU32;
	while(input > TAU32)
		input -= TAU32;
	
	f32 tau_minus_input = TAU32 - input; 
	if(tau_minus_input < input)
		input = tau_minus_input;
	return input;
}


static inline f32 radian_wrap2(f32 input)
{
	while(input < 0)
		input += TAU32;
	while(input > TAU32)
		input -= TAU32;
	
	return input;
}

static bool point_inside_mesh(v2f point, Mesh mesh)
{
	i32 i, j;
	bool c = false;
	for (i = 0, j = (i32)mesh.p_count - 1; i < (i32)mesh.p_count; j = i++)
	{
		if 
		(
			((mesh.data[i].y > point.y) != (mesh.data[j].y > point.y)) &&
			(point.x < (mesh.data[j].x - mesh.data[i].x) * (point.y - mesh.data[i].y)
			/ (mesh.data[j].y - mesh.data[i].y) + mesh.data[i].x)
		)
			c = !c;
	}
	return c;
}


static bool meshes_overlap(v2f pos_a, Mesh mesh_a, v2f pos_b, Mesh mesh_b)
{
	if(mesh_b.p_count >= 3)
	{
		for(u32 a = 0; a < mesh_a.p_count; ++a)
		{
			v2f pos = mesh_a.data[a] + (pos_a - pos_b);
			if(point_inside_mesh(pos, mesh_b))
				return true;
		}
	}
	
	if(mesh_a.p_count >= 3)
	{
		for(u32 b = 0; b < mesh_b.p_count; ++b)
		{
			v2f pos = mesh_b.data[b] + (pos_b - pos_a);
			if(point_inside_mesh(pos, mesh_a))
				return true;
		}
	}
	
	return false;
}


static bool meshes_overlap2(v2f pos_a, Mesh mesh_a, v2f pos_b, Mesh mesh_b)
{
	if(mesh_b.p_count >= 3)
	{
		for(u32 a = 0; a < mesh_a.p_count; ++a)
		{
			v2f pos = mesh_a.data[a] + (pos_a - pos_b);
			if(point_inside_mesh(pos, mesh_b))
				return true;
		}
	}
	
	if(mesh_a.p_count >= 3)
	{
		for(u32 b = 0; b < mesh_b.p_count; ++b)
		{
			v2f pos = mesh_b.data[b] + (pos_b - pos_a);
			if(point_inside_mesh(pos, mesh_a))
				return true;
		}
	}
	
	
	v2f ap1 = pos_a + mesh_a.data[mesh_a.p_count - 1];
	for(u32 a = 0; a < mesh_a.p_count; ++a)
	{
		v2f ap2 = pos_a + mesh_a.data[a];
		
		v2f bp1 = pos_b + mesh_b.data[mesh_b.p_count - 1];
		for(u32 b = 0; b < mesh_b.p_count; ++b)
		{
			v2f bp2 = pos_b + mesh_b.data[b];
			
			v2f hit;
			if(line_intersection(ap1, ap2, bp1, bp2, hit))
				return true;
			
			bp1 = bp2;
		}
		ap1 = ap2;
	}
	
	
	return false;
}


static inline bool point_inside_rect(v2f p, v2u rect)
{
	return p.x >= 0 && p.y >= 0 && p.x < rect.x - 1 && p.y < rect.y - 1;
}


static inline v2f random_point_on_rim(Random_Machine* rm, v2i start, v2i end)
{    
	Assert(rm);
	
	v2i area_dim = end - start;
	u32 spawn_pixel_count = (area_dim.x + area_dim.y) * 2;
	
	i32 random_pixel = (i32)rm->random_u32(spawn_pixel_count);
	v2i p;
	if(random_pixel < area_dim.x)
		p = { start.x + random_pixel, start.y};
	
	else if(random_pixel < area_dim.x + area_dim.y)
		p = { end.x, start.y + random_pixel - area_dim.x};
	
	else if(random_pixel < area_dim.x * 2 + area_dim.y)
		p = {start.x + random_pixel - ( area_dim.x * 2), end.y};
	
	else
		p  = {start.x, start.y + random_pixel - ( area_dim.x * 2 + area_dim.y)};
	
	return p.As<f32>();
}

static inline v2f random_point_in_rect(Random_Machine* rm, v2u rect)
{
	return { rm->random_f32() * rect.x, rm->random_f32() * rect.y };
}


static f32 get_turn_direction_to_face_target(f32 orientation, f32 target_orientation, f32& out_distance)
{
	target_orientation += PI32;
	if(orientation < 0)
		orientation += TAU32;
	
	f32 ccwd = orientation - target_orientation;
	while(ccwd < 0)
		ccwd += TAU32;
	while(ccwd > TAU32)
		ccwd -= TAU32;
	
	f32 cwd = TAU32 - ccwd;
	f32 turn_dir = 0;
	if(ccwd <= cwd)
	{
		out_distance = PI32 - ccwd;
		turn_dir = 1;
	}		    
	else
	{
		out_distance = PI32 - cwd;
		turn_dir = -1;
	}
	
	return turn_dir;
}


static f32 find_mesh_p_furthest_distance_from_origin(Mesh mesh)
{
	f32 result = 0;
	
	for(u32 p = 0; p < mesh.p_count; ++p)
		result = max(distance({0,0}, mesh.data[p]), result);
	
	return result;
}


static Rect create_bounding_box_from_mesh(Mesh mesh)
{
	f32 min_x = 0, min_y = 0;
	f32 max_x = 0, max_y = 0;
	
	for(u32 i = 0; i < mesh.p_count; ++i)
	{
		v2f p = mesh.data[i];
		
		if(p.x < min_x)
			min_x = p.x;
		
		else if(p.x > max_x)
			max_x = p.x;
		
		if(p.y < min_y)
			min_y = p.y;
		
		else if(p.y > max_y)
			max_y = p.y;
	}
	
	return create_rect_min_max({ min_x, min_y }, { max_x, max_y });
}


static Rect create_bounding_box_from_mesh(Mesh mesh, v2f offset)
{
	Rect result = create_bounding_box_from_mesh(mesh);
	
	add_offset_to_rect(offset, &result);
	
	return result;
}
