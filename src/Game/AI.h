
#pragma once

// Because of rendering order changes, AI visualization doesn't work anymore.
#define AI_Visualization
#undef AI_Visualization

#ifdef AI_Visualization

#define AIV_Call(X) X

#else

#define AIV_Call(X)

#endif // AI_Visualization


struct Threat_Map_Element
{
	v2f position = { 0, 0 };
	v2f velocity = { 0, 0 };
	v2f nv = { 0, 0 };
	v2f ro = { 0, 0 };
	v2f lo = { 0, 0 };
	
	f32 mv = 0;
	f32 size = 0;
};


struct Threat_Pizza
{
	static constexpr u32 slice_count = 64;
	f32 threat_pizza[slice_count];
	
	inline void clear_threat_pizza();
	inline u32 get_threat_pizza_idx_from_angle(f32 angle);
	f32 get_safest_direction(f32 start_angle);
	
	void set_threat_level(f32 level, f32 start_angle, f32 end_angle, f32 middle_angle);
};



static v2f pick_enemy_target_location(v2f position, f32 desired_velocity);


static void threat_map_add_asteroid(Entity* entity, Threat_Map_Element* map, u32* element_count, u32 array_size);


static f32 add_threat_element_to_pizza(Threat_Pizza* pizza, 
										Threat_Map_Element* threat, 
										f32 ship_size, 
										v2f threat_mesh_origin,
										v2f* threat_mesh,
										v2f ship_position);


static void check_for_enemy_AI_Interupt(Entity* enemy_ship, Threat_Map_Element* map, u32 map_element_count);


static inline void handle_enemy_AI(Entity* e_ship_entity);
