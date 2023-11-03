
#pragma once



struct Default_Weapon_Data
{
	Default_Weapon_Data()
	{
		Assert(sizeof(Default_Weapon_Data) <= Weapon::data_buffer_size);
	}
	
	f32 bullet_speed = 0;
	i32 bullet_damage = 0;
	f32 color_effect = 0;
};


struct Scatter_Weapon_Data
{
	Scatter_Weapon_Data()
	{
		Assert(sizeof(Scatter_Weapon_Data) <= Weapon::data_buffer_size);
	}  
	
	f32 pellet_speed = 0;
	i32 pellet_damage = 0;
	u32 per_shot_pellet_count = 0;
	f32 shot_arc = 0;
	f32 color_effect = 0;
};


struct Laser_Weapon_Data
{
	Laser_Weapon_Data()
	{
		Assert(sizeof(Laser_Weapon_Data) <= Weapon::data_buffer_size);
	}
	
	f32 damage_freq = 0;
	i32 laser_damage = 0;
};


static Weapon create_weapon(Weapon::type type);

static void fire_default_weapon(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* _weapon);
static void fire_laser(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* _weapon);
static void fire_scatter_gun(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* _weapon);
static char get_display_char_for_weapon(Weapon::type weapon);
