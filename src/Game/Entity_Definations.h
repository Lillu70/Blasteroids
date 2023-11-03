
#pragma once

struct Weapon
{
	enum class type
	{
		slow_scatter = -3,
		enemy_def = -2,
		def = -1,
		scatter,
		laser,
		COUNT,
		missile,
	};
	
	void(*_fire_func)(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* weapon) = 0;
	
	f64 next_fire_time = 0;
	f32 fire_rate = 0;
	u32 ammo = 0;
	u32 max_ammo = 0;
	
	type _type = Weapon::type::def;
	static constexpr u32 data_buffer_size = 20;
	u8 data[data_buffer_size];
	
	inline bool can_fire(f64 time)
	{
		return ammo > 0 && time >= next_fire_time;
	}
	
	inline void fire(f64 time, Entity* source, v2f weapon_mount_p, f32 weapon_dir)
	{
		if(can_fire(time))
		{
			next_fire_time = time + fire_rate;
			_fire_func(source, weapon_mount_p, weapon_dir, this);   
		}
	}
};

struct Ship
{   
	v2f* local_mesh = 0;
	Mesh mesh;
	f64 next_thrust_emit_time = 0;
	u32 itime = 0;
	u32 icolor = 0;
	f32 turn_speed = 0;
	f32 acceleration_speed = 0;
	f32 orientation = 0;
	f32 width = 0;
	
	Weapon weapon;
	Weapon::type default_weapon_type;
	
	bool passed_screen;
	
	struct Input
	{
		f32 turn_dir = 0;
		bool apply_thrust = false;
		bool shoot = false;
		bool shoot_key_not_down = false;
	};
	Input input = {0};
};


static f32 turn_ship_towards_angle(f32 target_angle, Ship* ship, f32 t)
{
	f32 result = 1;
	target_angle = radian_wrap2(target_angle - HALF_PI32);
	f32 look_angle = ship->orientation; // + HALF_PI32;		    
	f32 dist;
	ship->input.turn_dir = get_turn_direction_to_face_target(look_angle, target_angle, dist);
	f32 max_turn = ship->turn_speed * t;
	
	if(dist < max_turn)
	{
		ship->input.turn_dir = dist / max_turn * ship->input.turn_dir;
		result = ship->input.turn_dir;
	}
	
	return result;
}


// Tightly packed asteroid that is just the right size for internal alloc.
struct Asteroid
{
	Rect bounding_box;
	v2f* _mesh;
	i32 hp;
	u8 mesh_p_count;
	Size size;
	
	inline Mesh mesh()
	{
		Mesh result = { _mesh, (u32)mesh_p_count };
		return result;
	}
};


struct Player
{
	Ship* ship  = 0;
	Weapon::type spawn_weapon;
	u32 score = 0;
	i32 lives = 0;
	u32 color = WHITE;
	u32 player_id = 0;
	u32 ship_id = 0;
	
	static constexpr u32 max_score = 999999;
	void give_score(i32 _score)
	{
		if(_score < 0 && score + _score > score)
			score = 0;
		
		score += _score;
		if(score > max_score)
			score = max_score;
	}
};


struct Bullet
{
	static inline f32 trace_lenght = 10;
	u32 source_id = 0;
	Entity_Type source_type = Entity_Type::none;
	i32 damage = 0;
};


struct AI
{
	enum class State
	{
		none = 0,
		face_movement_target,
		accelerate_towards_target,
		slow_down,
		fire_and_move,
		fire_and_stay,
		evasive_manuvers,
		do_nothing,
	};
	
	State state = AI::State::none;
	State last_state = AI::State::none;
	State next_state = AI::State::none;
	
	void set_state(State new_state, f64 now)
	{
		state_start_time = now;
		if(next_state != AI::State::none)
		{
			new_state = next_state;
			next_state = AI::State::none;
		}
		
		last_state = state;
		state = new_state;
	}
	
	f32 evade_direction = 0;
	f32 threat_towards_orientation;
	f32 threat_towards_velocity;
	f32 spook_range = 0;
	f32 thread_range = 0;
	v2f target_position = 0;
	f32 desired_velocity_magnitude = 0;
	f64 state_start_time = 0;
	u32 state_tick_count = 0;
};


struct Enemy_Ship
{
	enum class Type
	{
		def = 0,
		fast,
		slow,
		player_clone
	};
	
	Ship ship;
	i32 hp;
	AI ai;
	Type type;
};


struct Pickup
{
	enum class type : u16
	{
		none,
		weapon,
		life,
		time_stop,
	};
	
	u32 data = 0;
	type _type = Pickup::type::none;
	bool passed_screen = false;
	char display = 0;
	
	static constexpr f32 radius = 12;
};


struct Laser
{
	// Recording the pos and angle could cause issues if weapon holder is displaced.
	
	Weapon* source_weapon = 0; // <- this can never move in memory (heap alloc).
	f64 next_damage_time = 0;
	v2f pos = {0, 0};
	v2f impact_p = {0, 0};
	v2f last_dir = {0, 0};
	u32 last_hit_id = 0;
	f32 dir = 0;
	u32 color = 0;
	u32 source_id = 0;
	Entity_Type impact_target_type = Entity_Type::none;
	bool alive = false;
};


struct Asteroid_Properties
{
	u32 mesh_p_count;
	u32 hp;
	u32 min_radius;
	u32 max_radius;
};


static u32 get_enemy_mesh_idx_from_type(Enemy_Ship::Type type)
{
	switch(type)
	{
	case Enemy_Ship::Type::slow:
		return 1;
	
	case Enemy_Ship::Type::fast:
		return 2;
	
	default:
		return 0;
	}
}


static Asteroid_Properties get_asteroid_properties(Size size)
{
    Asteroid_Properties prop;
    
    switch(size)
    {
		case Size::small:
		{
			prop.mesh_p_count = 5;
			prop.hp = 1;
			prop.min_radius = 12;
			prop.max_radius = 20;
		}break;
		   
		case Size::medium:
		{
			prop.mesh_p_count = 8;
			prop.hp = 60;
			prop.min_radius = 40;
			prop.max_radius = 70;
		}break;
		
		case Size::large:
		{
			prop.mesh_p_count = 16;
			prop.hp = 100;
			prop.min_radius = 70;
			prop.max_radius = 100;
		}break;
    }

    return prop;
}

static inline u32 get_asteroid_score(Size size)
{
	switch(size)
	{
		case Size::small:
			return 2;
		
		case Size::medium:
			return 6;
		
		case Size::large:
			return 8;
	}
	
	return 0;
}


static inline u32 get_kill_score_for_entity(Entity* entity)
{
	switch(entity->type)
	{
		case Entity_Type::bullet:
			return 1;
			
		case Entity_Type::pickup:
			return 15;
		
		case Entity_Type::asteroid:
			return get_asteroid_score(entity->retrive_internal<Asteroid>()->size);
		
		case Entity_Type::enemy_ship:
			return 20;
		
		default:
			return 0;
	}
}

