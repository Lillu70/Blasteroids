
#pragma once

static Weapon create_weapon(Weapon::type type)
{
    Weapon weapon;
    
    switch(type)
    {
        case Weapon::type::enemy_def:
        {
            weapon.ammo = 0xFFFFFFFF;
            weapon.fire_rate = 0.7f;
            weapon.next_fire_time = 0;
            weapon._fire_func = fire_default_weapon;
            
            Default_Weapon_Data data;
            data.bullet_damage = 1;
            data.bullet_speed = 220;
            data.color_effect = 1.f;
            *((Default_Weapon_Data*)&weapon.data[0]) = data;
        }break;
        
        case Weapon::type::def:
        {
            weapon.ammo = 0xFFFFFFFF;
            weapon.fire_rate = 0.2f;
            weapon.next_fire_time = 0;
            weapon._fire_func = fire_default_weapon;
            
            Default_Weapon_Data data;
            data.bullet_damage = 10;
            data.bullet_speed = 500;
            data.color_effect = 0.65f;
            *((Default_Weapon_Data*)&weapon.data[0]) = data;
        }break;
        
        case Weapon::type::scatter:
        {
            weapon.ammo = 25;
            weapon.fire_rate = 0.2f;
            weapon.next_fire_time = 0;
            weapon._fire_func = fire_scatter_gun;
            
            Scatter_Weapon_Data data;
            data.pellet_damage = 5;
            data.pellet_speed = 250;
            data.per_shot_pellet_count = 5;
            data.shot_arc = PI32 / 4;
            data.color_effect = 0.4f;
            *((Scatter_Weapon_Data*)&weapon.data[0]) = data;
        }break;
        
        case Weapon::type::slow_scatter:
        {
            weapon.ammo = 10;
            weapon.fire_rate = 2.3f;
            weapon.next_fire_time = 0;
            weapon._fire_func = fire_scatter_gun;
            
            Scatter_Weapon_Data data;
            data.pellet_damage = 5;
            data.pellet_speed = 100;
            data.per_shot_pellet_count = 5;
            data.shot_arc = PI32 / 4;
            data.color_effect = 0.4f;
            *((Scatter_Weapon_Data*)&weapon.data[0]) = data;   
        }break;
        
        case Weapon::type::laser:
        {
            weapon.ammo = 50;
            weapon.fire_rate = 0;
            weapon.next_fire_time = 0;
            weapon._fire_func = fire_laser;
            
            Laser_Weapon_Data data;
            data.damage_freq = 0.2f;
            data.laser_damage = 10;
            *((Laser_Weapon_Data*)&weapon.data[0]) = data;
        }break;
        
        default:
        {
            Terminate;
            return Weapon();
        }
    }
    
    weapon.max_ammo = weapon.ammo;
    weapon._type = type;
    return weapon;
}


static void fire_default_weapon(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* _weapon)
{
    Assert(source);
    
    if(game.active_entity_count >= game.max_entity_count)
        return;
    
    Default_Weapon_Data* weapon = (Default_Weapon_Data*)&(_weapon->data[0]);
    
    v2f weapon_dir_vec = {sinf(weapon_dir) * -1 , cosf(weapon_dir) };
    
    Entity* entity = add_entity(Entity_Type::bullet);
    set_inheritet_entity_flags(entity, source);
    entity->position = weapon_mount_p;
    entity->velocity = weapon_dir_vec * weapon->bullet_speed;
    entity->color = multiply_accross_color_channels(source->color, weapon->color_effect); 
    
    *entity->alloc_internal<Bullet>() = { source->id, source->type, weapon->bullet_damage };
    
    Sound* sound = get(Sounds::default_weapon_shoot);
    Play_Sound(sound, &weapon_mount_p, Play_Mode::once, Sound_Types::effect, volume_range, pitch_range);
}


static void fire_laser(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* _weapon)
{
    Assert(source);
    
    Particle_Defination pd;     
    pd.min_speed = 50;
    pd.max_speed = 50;
    pd.fade_start_time = game.game_time + 0.1f;
    pd.life_time = game.game_time + 0.2f;
    pd.full_color = source->color;
    
    
    Emission_Cone cone = { weapon_dir+HALF_PI32, HALF_PI32 };
    
    particle_system_emit(weapon_mount_p, cone, &pd, 10);
    
    u32 laser_count = game.active_laser_count;
    for(u32 i = 0; i < laser_count; ++i)
    {
        Laser* laser = &game.laser_table[i];
        if(laser->source_weapon == _weapon)
        {
            laser->alive = true;
            laser->pos = weapon_mount_p;
            laser->dir = weapon_dir;
            return;
        }
    }
    
    {
        if(laser_count >= game.max_laser_count)
            return;
        
        Laser laser = {};
        laser.source_weapon = _weapon;
        laser.pos = weapon_mount_p;
        laser.dir = weapon_dir;
        laser.color = multiply_accross_color_channels(source->color, 255);
        laser.source_id = source->id;
        laser.alive = true;
        
        game.laser_table[laser_count] = laser;
        game.active_laser_count += 1;
    }
}


static void fire_scatter_gun(Entity* source, v2f weapon_mount_p, f32 weapon_dir, Weapon* _weapon)
{
    Assert(source);
    
    _weapon->ammo -= 1;
    game.draw_ui = true;
    Scatter_Weapon_Data* weapon = (Scatter_Weapon_Data*)&(_weapon->data[0]);
    
    f32 step_arc = weapon->shot_arc / weapon->per_shot_pellet_count;
    
    u32 color = multiply_accross_color_channels(source->color, weapon->color_effect);
    for(u32 i = 0; i < weapon->per_shot_pellet_count; ++i)
    {
        if(game.active_entity_count >= game.max_entity_count)
            return;
        
        f32 pellet_dir = weapon_dir - weapon->shot_arc / 2 + step_arc * i;
        v2f weapon_dir_vec = {sinf(pellet_dir) * -1 , cosf(pellet_dir) };
        
        
        Entity* entity = add_entity(Entity_Type::bullet);
        set_inheritet_entity_flags(entity, source);
        entity->color = color;
        entity->position = weapon_mount_p;
        entity->velocity = weapon_dir_vec * weapon->pellet_speed;
        
        *entity->alloc_internal<Bullet>() = { source->id, source->type, weapon->pellet_damage };
    }
    
    Sound* sound = get(Sounds::scatter_gun_shoot);
    Play_Sound(sound, &weapon_mount_p, Play_Mode::once, Sound_Types::effect, volume_range, pitch_range);
}

static char get_display_char_for_weapon(Weapon::type weapon)
{
    switch(weapon)
    {
    case Weapon::type::scatter:
        return 'S';
    case Weapon::type::laser:
        return 'L';
    }
    
    return 'x';
}
