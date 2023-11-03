
#include "Asteroids.h"
#include "GUI_Factory.h"

//#define QUICKSTART

/*
TODOS:

Big:
- Sound system/effects

Low:
- Improved wave generation system that get's harder over time.
- More pickups/weapons
- Visual asteroid damage effects (hp representation with cracks)
- Make it so that enemies can't shoot for a bit after entering the screen.
- Redesign fast enemy.


Bug:
- partially off buffer GUI widget rendering

*/


static void init_asteroids_game(Platform_Call_Table platform_calltable, void* game_state_memory, u32 game_state_memory_size)
{	
	platform = platform_calltable;
    
    // Set up the font for text rendering.
    {
        GUI_Font* font = &s_gui_theme.font;
        
        font->char_width = s_terminus_font_char_width;
        font->char_height = s_terminus_font_char_height;
        font->data_buffer = (u8*)s_terminus_font;
    }
    
	// Push transient memory in, and give the rest for the general allocator.
	{	
		Linear_Allocator transient_mem;
		transient_mem.init(game_state_memory, game_state_memory_size);
		generate_mesh_data(&transient_mem, &transient);
		
		game.timed_events = (Timed_Event*)transient_mem.push(sizeof(Timed_Event) * game.max_timed_event_count);
		game.entities = (Entity*)transient_mem.push(sizeof(Entity) * game.max_entity_count);
		game.laser_table = (Laser*)transient_mem.push(sizeof(Laser) * game.max_laser_count);
		game.player_table = (Player*)transient_mem.push(sizeof(Player) * game.max_player_count);
		particle_system.particles = (Particle*)transient_mem.push(sizeof(Particle) * particle_system.max_particle_count);
		
		
		mem.init(transient_mem.next_free, transient_mem.get_free_capacity());		
	}
	
    transient.pixel_buffer_dimensions = 
        {
            (u32)platform.get_pixel_buffer_width(), 
            (u32)platform.get_pixel_buffer_height()
        };  
        
    transient.pixel_buffer = platform.get_pixel_buffer();

    if(!load_settings_and_score())
    {
        load_default_menu_action();
        load_default_game_action();
        load_default_global_actions();        
    }
    
	screen_canvas = Pixel_Canvas(transient.pixel_buffer, transient.pixel_buffer_dimensions);
	
    game.gui_handler.active_theme = &s_gui_theme;
    
	set_mode_main_menu();
}


static void set_mode_main_menu()
{
    #ifdef QUICKSTART
    
    set_mode_asteroids_sp();
    return;
    
    #endif
    
	Random_Machine::seed = (u32)game.game_time;
	
	s_game_mode = Game_Mode::main_menu;
	
	reset_game();
	
    remove_ui_canvas();

    i32 asteroid_buffer_area = 100;
    game.asteroid_area_start = -asteroid_buffer_area;
    game.asteroid_area_end = canvas.m_dimensions.As<i32>() + asteroid_buffer_area + 1;
	
	gui_create_main_menu();
	
	//Spawn things.
	{
		spawn_enemy_ship(Enemy_Ship::Type::player_clone);
		
		static constexpr u32 menu_asteroid_count = 6;
		
		for(u32 i = 0; i < menu_asteroid_count; ++i)
		{
			spawn_new_asteroid((Size)game.rm.random_u32(2));
		}
	}
}


static void set_mode_asteroids_sp()
{
	s_game_mode = Game_Mode::asteroids_sp;
    restart_game();
}


static void update_asteroids_game(f64 delta_time, bool& update_surface)
{
    if(platform.get_flags() & (1 << (u32)App_Flags::wants_to_exit))
    {
        platform.set_flag(App_Flags::wants_to_exit, false);
        gui_create_quit_menu();
        return;
    }

    if(platform.get_flags() & 1 << (u32)App_Flags::is_focused)
        process_global_actions();
    
    else if(!gui_menu_is_up(&game.gui_handler) && !game.is_paused)
        gui_create_pause_menu();
    
    gui_handle_input(&game.gui_handler, &platform, &s_menu_actions[0]);

    if(!game.is_paused)
	{
        if(!gui_menu_is_up(&game.gui_handler))
        {
            update_actions(&platform, &s_game_actions[0], (u32)Game_Actions::COUNT);
            if(get_action(Game_Actions::pause)->is_pressed())
            {
                gui_create_pause_menu();
            }
            
            record_ship_input();
        }
        else
        {
            clear_ship_input();
        }
        
        // Fast forward mode
        if(platform.get_keyboard_key_down(Key_Code::T))
        {
            f32 real_delta = delta_time;
            delta_time *= 5;
            
            game.total_pause_time -= (delta_time - real_delta);
        }
        
        u32 update_count = 0;
        
        for(game.physics_time += delta_time; game.physics_time >= 0; game.physics_time -= game.update_tick)
        {
            physics_update();
            update_count += 1;
        }
        
        if(update_count > 0)
        {
            clear_ship_input();
        }
    }
	
    
	f64 time_stamp = platform.get_time_stamp();
	if(time_stamp >= game.next_draw_time)
	{
		game.next_draw_time = time_stamp + game.draw_frequency;
        u32 clear_color = put_color(10, 10, 10);
		
		canvas.clear(clear_color);
	
		f32 particle_delta = game.is_paused? 0 : (f32)game.draw_frequency;
        
        particle_system_update_and_draw(&canvas, particle_delta, game.game_time, clear_color);
		
        draw_game();
		
		gui_draw_widgets(&game.gui_handler, &screen_canvas);
		
		update_surface = true;
    }
}


static void reset_game()
{
    GUI_Theme* temp_theme = game.gui_handler.active_theme;
	
    mem.clear();
    
    game = Game();
	particle_system_clear();

    game.gui_handler.active_theme = temp_theme;
}


static void restart_game()
{
    reset_game();
    start_game();
}


static void start_game()
{
	platform.set_flag(App_Flags::cursor_is_visible, false);
        
    // add ui canvas
    {
        v2u pixel_buffer_dimensions = transient.pixel_buffer_dimensions;
        
        u32 score_area_height = 28;
        pixel_buffer_dimensions.y -= score_area_height;
        canvas = Pixel_Canvas(transient.pixel_buffer + score_area_height * pixel_buffer_dimensions.x, pixel_buffer_dimensions);
        ui_canvas = Pixel_Canvas(transient.pixel_buffer, { pixel_buffer_dimensions.x, score_area_height });

        i32 asteroid_buffer_area = 100;
        game.asteroid_area_start = -asteroid_buffer_area;
        game.asteroid_area_end = canvas.m_dimensions.As<i32>() + asteroid_buffer_area + 1;
        
    }
    
	game.game_time = platform.get_time_stamp();
	
	Random_Machine::seed = (u32)game.game_time;
	
	// Set pickup mesh
    {
		f32 a = TAU32 / transient.pickup_mesh.p_count;
		for(i32 i = 0; i < transient.pickup_mesh.p_count; ++i)
		{
			transient.pickup_mesh.data[i].x = cosf(a * i - HALF_PI32) * Pickup::radius;
			transient.pickup_mesh.data[i].y = sinf(a * i - HALF_PI32) * Pickup::radius;
		}
    }

    add_player(put_color(235,00,235), canvas.get_middle());
	
    #if 1
	add_timed_event(Timed_Event{ game.game_time + 0.5f, Event_Type::spawn_wave});
    add_timed_event(Timed_Event{ 0, Event_Type::spawn_pickups, 0 });	
    #endif

}


static void physics_update()
{
	game.game_time = platform.get_time_stamp() - game.total_pause_time;
    f64 now = game.game_time;
    
	if(s_game_mode == Game_Mode::main_menu)
	{
		if(game.active_enemy_ship_count == 0)
			spawn_enemy_ship(Enemy_Ship::Type::player_clone);
		
		if(game.active_asteroid_count == 0)
		{
			static constexpr u32 menu_asteroid_count = 6;
		
			for(u32 i = 0; i < menu_asteroid_count; ++i)
			{
				spawn_new_asteroid((Size)game.rm.random_u32(2));
			}
		}
	}
	
    if(game.wave_count > 0 && game.get_active_hostile_count() == 0)
		force_next_wave();
    
    process_timed_events();
    
	static constexpr u32 grid_size = 9;
	u32 threat_map_size = sizeof(Threat_Map_Element) * game.active_asteroid_count * grid_size;
	u32 threat_map_element_count = 0;
	Threat_Map_Element* threat_map = (Threat_Map_Element*)mem.push(threat_map_size);
	
    //Update all entity positions.
    for(u32 i = 0; i < game.active_entity_count; ++i)
    {
		Entity* entity = &game.entities[i];
		
		if(now < game.pickup_time_stop_time && !get_entity_flag(entity, Entity_Flags::immune_to_time_stop))
			continue;
	
		switch(entity->type)
		{
			case Entity_Type::enemy_ship:
				handle_enemy_AI(entity);
				
			case Entity_Type::player_ship:
			{
				Ship* ship = entity->retrive_external<Ship>();
				f32 turn_dir = ship->input.turn_dir;
				
				f32 orien_cos = cosf(ship->orientation);
				f32 orien_sin = sinf(ship->orientation);
				
				if(turn_dir)
				{	
					ship->orientation += turn_dir * ship->turn_speed * game.update_tick;
					ship->orientation = radian_wrap2(ship->orientation);
					
					rotate_local_mesh(ship->local_mesh, ship->mesh, orien_cos, orien_sin);
				}
				
				
				if(ship->input.apply_thrust)
				{
					v2f ship_acceleration;		
					ship_acceleration.x += -orien_sin;
					ship_acceleration.y += orien_cos;
					ship_acceleration *= ship->acceleration_speed;
						
					update_position(&entity->position, &entity->velocity, ship_acceleration, game.update_tick);
					
					if(game.game_time >= ship->next_thrust_emit_time)
					{
						ship->next_thrust_emit_time = game.game_time + 0.05f;
						
						Emission_Cone cone = { ship->orientation - HALF_PI32, 0.2f };
			
						Particle_Defination pd;
						pd.min_speed = magnitude(entity->velocity) + 10;
						pd.max_speed = pd.min_speed + 20;
						pd.full_color = entity->color;//put_color(255,128,40);
						pd.fade_start_time = game.game_time + 0.1f;
						pd.life_time = game.game_time + 0.3f;
						
						f32 inverse_ship_facing_angle = ship->orientation - HALF_PI32;
						v2f inverse_ship_facing_vector = 
							{ cosf(inverse_ship_facing_angle), sinf(inverse_ship_facing_angle) };
						
						v2f emit_pos = inverse_ship_facing_vector * 5 + entity->position;
						
						particle_system_emit(emit_pos, cone, &pd, 10);
					}
				
				}
				else
					update_position(&entity->position, &entity->velocity, game.update_tick);
				
				if(ship->passed_screen && entity->type == Entity_Type::player_ship)
					clamp_position_to_rect(&entity->position, canvas.m_dimensions.As<i32>());
				
				else
				{
					if(point_inside_rect(entity->position, canvas.m_dimensions))
						ship->passed_screen = true;
					
					clamp_position_to_rect(&entity->position, game.asteroid_area_start, game.asteroid_area_end);
				}
				
			
				if(ship->input.shoot)
				{
					constexpr f32 spawn_distance = 20;
					f32 bx = entity->position.x + (orien_sin * spawn_distance * -1);
					f32 by = entity->position.y + (orien_cos * spawn_distance);
					v2f gun_mount_p = { bx, by };

					ship->weapon.fire(now, entity, gun_mount_p, ship->orientation);
					if(ship->weapon.ammo == 0)
					{
						ship->weapon = create_weapon(ship->default_weapon_type);
						ship->weapon.next_fire_time = now + ship->weapon.fire_rate;
					}
				}
			}
			break;


			case Entity_Type::bullet:
			{
				update_position(&entity->position, &entity->velocity, game.update_tick);
				if(!point_inside_rect(entity->position, canvas.m_dimensions))
				{
					kill_entity(entity);
				}
				
			}break;


			case Entity_Type::pickup:
			{	
				update_position(&entity->position, &entity->velocity, game.update_tick);
				Pickup* pickup = entity->retrive_internal<Pickup>();

				bool on_screen = point_inside_rect(entity->position, canvas.m_dimensions);
				if(on_screen)
					pickup->passed_screen = true;
				else if(pickup->passed_screen)
				{
					kill_entity(entity);
				}
				else
					clamp_position_to_rect(&entity->position, game.asteroid_area_start, game.asteroid_area_end);
			
			}break;
			
			case Entity_Type::asteroid:
			{
				update_position(&entity->position, &entity->velocity, game.update_tick);
				clamp_position_to_rect(&entity->position, game.asteroid_area_start, game.asteroid_area_end);
				
				if(game.active_enemy_ship_count > 0)
					threat_map_add_asteroid(entity, threat_map, &threat_map_element_count, threat_map_size);
			}break;
		}
    }

	// Check entity vs entity.
	{
		Entity* end = game.entities + game.active_entity_count;
		for(Entity* a = game.entities; a < end; ++a)
		{
			if(!get_entity_flag(a, Entity_Flags::alive))
				continue;
		
			if(a->type == Entity_Type::enemy_ship)
				check_for_enemy_AI_Interupt(a, threat_map, threat_map_element_count);
		
			for(Entity* b = a + 1; b < end; ++b)
			{
				if(!get_entity_flag(b, Entity_Flags::alive))
					continue;
				
				check_interactions(a, b) || check_interactions(b, a);
		
				
				if(!get_entity_flag(a, Entity_Flags::alive))
					break;
			}
		}
	}
	
	mem.free(threat_map);
	
	check_lasers_againt_targets();

    // Rotate pickup mesh.
    if(now >= game.pickup_time_stop_time)
    {
		f32 a = PI32 / 3 * game.update_tick;
		f32 c = cosf(a);
		f32 s = sinf(a);
		rotate_mesh(transient.pickup_mesh.data, transient.pickup_mesh.data, transient.pickup_mesh.p_count, c, s);
    }
	
	remove_dead_entities();
}

static inline bool check_interactions(Entity* a, Entity* b)
{
	Assert(a && b);
	
	switch(a->type)
	{
		case Entity_Type::bullet:
		{
			switch(b->type)
			{
				case Entity_Type::asteroid:
					bullet_vs_asteroid(a, b);
					return true;
				
				case Entity_Type::bullet:
					bullet_vs_bullet(a, b);
					return true;
					
				case Entity_Type::enemy_ship:
				case Entity_Type::player_ship:
				{
					bullet_vs_ship(a, b);
				}break;
			}
		}break;
	
		case Entity_Type::player_ship:
		{
			switch(b->type)
			{
				case Entity_Type::asteroid:
					player_ship_vs_asteroid(a, b);
					return true;
				
				case Entity_Type::pickup:
					player_ship_vs_pickup(a, b);
					return true;
			}
			
		}break;
		
		case Entity_Type::enemy_ship:
		{
			switch(b->type)
			{
				case Entity_Type::asteroid:
					enemy_ship_vs_asteroid(a, b);
					return true;
			}
			
		}break;
	}
	
	return false;
}


static void bullet_vs_asteroid(Entity* bullet_entity, Entity* asteroid_entity)
{
	Assert(bullet_entity && asteroid_entity);
	
	v2f bullet_point = bullet_entity->position - asteroid_entity->position;
    Asteroid* asteroid = asteroid_entity->retrive_internal<Asteroid>();

    if(point_inside_mesh(bullet_point, asteroid->mesh()))
    {
		kill_entity(bullet_entity);
		
		Bullet* bullet = bullet_entity->retrive_internal<Bullet>();
		asteroid->hp -= bullet->damage;
		
		
		// Also kill the asteroid!
		if(asteroid->hp <= 0)
			murder_entity(asteroid_entity, bullet->source_id);
		
    }
}


static void bullet_vs_bullet(Entity* bullet1, Entity* bullet2)
{
	Bullet* a = bullet1->retrive_internal<Bullet>();
	Bullet* b = bullet2->retrive_internal<Bullet>();
	
	if(a->source_id == b->source_id)
		return;
	
	v2f a1 = bullet1->position;
    v2f av = bullet1->velocity;
    v2f a2 = a1 - (av * game.update_tick) - (normalize(av) * Bullet::trace_lenght);

    v2f b1 = bullet2->position;
    v2f bv = bullet2->velocity;
    v2f b2 = b1 - (bv * game.update_tick) - (normalize(bv) * Bullet::trace_lenght);

    v2f impact;
    if(line_intersection(a1, a2, b1, b2, impact))
    {
		if(a->source_type == Entity_Type::player_ship && b->source_type != Entity_Type::player_ship)
		{
			murder_entity(bullet2, a->source_id);
			kill_entity(bullet1);
		}
		
		else if(b->source_type == Entity_Type::player_ship && a->source_type != Entity_Type::player_ship)
		{	
			murder_entity(bullet1, b->source_id);			
			kill_entity(bullet2);
		}
		else
		{			
			kill_entity(bullet1);
			kill_entity(bullet2);
		}
	}
}


static void player_ship_vs_asteroid(Entity* player_ship_entity, Entity* asteroid_entity)
{
	Ship* ship = player_ship_entity->retrive_external<Ship>();
    Asteroid* asteroid = asteroid_entity->retrive_internal<Asteroid>();

    Mesh ship_mesh = { ship->local_mesh, ship->mesh.p_count };
	
	Rect ship_bounding_box = create_rect_center_halfdim({0,0}, ship->width);
	bool bb_overlap = rects_overlap(player_ship_entity->position, 
									ship_bounding_box, 
									asteroid_entity->position, 
									asteroid->bounding_box);
	
	if(bb_overlap && meshes_overlap2(player_ship_entity->position, ship_mesh, asteroid_entity->position, asteroid->mesh()))
	{
		if(game.game_time < ship->itime)
			kill_entity(asteroid_entity);
		
		else
			kill_entity(player_ship_entity);
		
    }
}


static void player_ship_vs_pickup(Entity* player_ship, Entity* pickup_entity)
{
	Ship* ship = player_ship->retrive_external<Ship>();
    
    Rect ship_bounding_box = create_rect_center_halfdim(player_ship->position, ship->width);
    Rect pickup_bounding_box = create_rect_center_halfdim(pickup_entity->position, Pickup::radius * 2);
    
    if(!rects_overlap(ship_bounding_box, pickup_bounding_box))
		return;
    
    
    
    Mesh ship_mesh = { ship->local_mesh, ship->mesh.p_count };
    if(meshes_overlap(player_ship->position, ship_mesh, pickup_entity->position, transient.pickup_mesh))
    {
        Pickup* pickup = pickup_entity->retrive_internal<Pickup>();
        
        Player* player = find_ship_owner(ship);
        Assert(player);
        
        switch(pickup->_type)
        {
        case Pickup::type::weapon:
            ship->weapon = create_weapon((Weapon::type)pickup->data);
            break;

        case Pickup::type::life:
            player->lives += 1;
            break;

        case Pickup::type::time_stop:
            game.pickup_time_stop_time = game.game_time + 5.f;
            break;
        }
        
        murder_entity(pickup_entity, player_ship);
        
        Particle_Defination pd;
        
        u32 emission_count = 0;
        pd.full_color = pickup_entity->color;
        pd.fade_start_time = game.game_time + 0.2f;
        pd.life_time = game.game_time + 0.75f;
        
        pd.min_speed = 50;
        pd.max_speed = 70;
        emission_count = 30;

        Emission_Cone cone = { 0, TAU32, Pickup::radius };
        
        particle_system_emit(pickup_entity->position, cone, &pd, emission_count);
    }
}


static void enemy_ship_vs_asteroid(Entity* enemy_ship_entity, Entity* asteroid_entity)
{
	Ship* ship = enemy_ship_entity->retrive_external<Ship>();
	
	Rect ship_bounding_box = create_rect_center_halfdim({0,0}, ship->width);
	
	if(!rects_overlap(ship_bounding_box, create_rect_min_max({0,0}, canvas.m_dimensions.As<f32>())))
		return;
	
	Asteroid* asteroid = asteroid_entity->retrive_internal<Asteroid>();
	
	bool bb_overlap = rects_overlap(enemy_ship_entity->position, 
								ship_bounding_box, 
								asteroid_entity->position, 
								asteroid->bounding_box);

	Mesh ship_mesh = { ship->local_mesh, ship->mesh.p_count };
	if(bb_overlap && meshes_overlap(enemy_ship_entity->position, ship_mesh, asteroid_entity->position, asteroid->mesh()))
	{
		kill_entity(enemy_ship_entity);
	}
}


static void bullet_vs_ship(Entity* bullet_entity, Entity* ship_entity)
{
	Bullet* bullet = bullet_entity->retrive_internal<Bullet>();
	if(bullet->source_id == ship_entity->id)
		return;
	
	Ship* ship = ship_entity->retrive_external<Ship>();
	
	v2f b[2];
    v2f bv = bullet_entity->velocity;
	
	b[0] = bullet_entity->position;
    b[1] = b[0] - (bv * game.update_tick) - (normalize(bv) * Bullet::trace_lenght);
	
	Mesh ship_mesh =  { ship->local_mesh, ship->mesh.p_count};

	Rect ship_bounding_box = create_rect_center_halfdim(ship_entity->position, ship->width);
	Rect bullet_bounding_box = create_rect_center_halfdim(bullet_entity->position, Bullet::trace_lenght);
	bool bb_overlap = rects_overlap(ship_bounding_box, bullet_bounding_box);

	
	if(bb_overlap && meshes_overlap2(0, { &b[0], 2}, ship_entity->position, ship_mesh))
	{
		murder_entity(ship_entity, bullet->source_id);
	}
}


static void check_lasers_againt_targets()
{
    for(u32 i = 0; i < game.active_laser_count; ++i)
    {
		if(game.laser_table[i].alive == false)
		{
			game.active_laser_count -= 1;
			game.laser_table[i] = game.laser_table[game.active_laser_count];
			i -= 1;
			continue;
		}
		else
		{
			
			Laser* laser = &game.laser_table[i];
			laser->alive = false;
			
			v2f laser_pos = laser->pos;
			v2f laser_dir = { sinf(laser->dir) * -1, cosf(laser->dir) };
			Assert(laser_dir != 0);
			
			if(point_inside_rect(laser_pos, canvas.m_dimensions) == false)
			{
				laser->impact_p = laser_pos;
				laser->last_dir = laser_dir;
				continue;
			}
			
			laser->impact_target_type = Entity_Type::none;


			
			f32 t = F32_MAX;
			if(laser_dir.x != 0)
			{
				f32 tx = vector_intersects_axis(laser_pos.x, laser_dir.x, 0);
				if(tx < 0)
					tx = vector_intersects_axis(laser_pos.x, laser_dir.x, canvas.m_dimensions.x);
				
				t = tx;
			}
			
			if(laser_dir.y != 0)
			{
				f32 ty = vector_intersects_axis(laser_pos.y, laser_dir.y, 0);
				if(ty < 0)
					ty = vector_intersects_axis(laser_pos.y, laser_dir.y, canvas.m_dimensions.y);
				
				t = min(t, ty);
			}

			v2f edge = laser->pos + laser_dir * t;
			v2f closest_hit = edge;
			f32 short_distance = distance(laser_pos, edge);
			u32 hit_idx = 0;
			Entity_Type hit_type = Entity_Type::none;
			
			for(u32 i2 = 0; i2 < game.active_entity_count; ++i2)
			{
				Entity_Type type = game.entities[i2].type;
				switch(type)
				{
					case Entity_Type::asteroid:
					{
						// Todo an opitimazation pass for this. First check againt a bounding box.
						// Cross bb line check?
						// use sqr distance instead, just relative comparison, so no need for root.
						Asteroid* asteroid = game.entities[i2].retrive_internal<Asteroid>();
						v2f asteroid_pos = game.entities[i2].position;
						v2f p1 = asteroid_pos + asteroid->_mesh[(u32)asteroid->mesh_p_count - 1];
					  
						for(u32 j = 0; j < asteroid->mesh_p_count; ++j)
						{
							v2f hit;
							v2f p2 = asteroid_pos + asteroid->_mesh[j];
							if(line_intersection(laser_pos, closest_hit, p2, p1, hit))
							{
								f32 _distance = distance(laser_pos, hit);
								if(_distance < short_distance)
								{
									closest_hit = hit;
									short_distance = _distance;
									hit_idx = i2;
									hit_type = type;
								}
							}
							p1 = p2;
						}
					}break;
					
					case Entity_Type::enemy_ship:
					{
						// Todo an opitimazation pass for this. First check againt a bounding box.
						// Cross bb line check?
						// use sqr distance instead, just relative comparison, so no need for root.
						Ship* ship = game.entities[i2].retrive_external<Ship>();
						v2f ship_pos = game.entities[i2].position;
						v2f p1 = ship_pos + ship->local_mesh[ship->mesh.p_count - 1];
					  
						for(u32 j = 0; j < ship->mesh.p_count; ++j)
						{
							v2f hit;
							v2f p2 = ship_pos + ship->local_mesh[j];
							if(line_intersection(laser_pos, closest_hit, p2, p1, hit))
							{
								f32 _distance = distance(laser_pos, hit);
								if(_distance < short_distance)
								{
									closest_hit = hit;
									short_distance = _distance;
									hit_idx = i2;
									hit_type = type;
								}
							}
							p1 = p2;
						}
						
					}break;

					case Entity_Type::bullet:
					{
						// TODO: Find a better way to do this. Sometimes misses bullets.
						
						v2f hit;
						v2f v = game.entities[i2].velocity;
						
						v2f bullet[2];
						
						bullet[0] = game.entities[i2].position;
						bullet[1] = bullet[0] + (normalize(v) * (-Bullet::trace_lenght * 2));
						
						// Sweep check. eka the cone between last facing dir and current.
						// "Better" accuracy, but only works if the player is rotating.
						// ^ as in less likely to miss the thin bullet.
						
						bool impact = false;
						if(laser_dir != laser->last_dir)
						{
				
							v2f _laser_mesh[3] = { laser_pos, laser_pos + laser_dir * 1000, laser_pos + laser->last_dir * 1000 };
							Mesh laser_mesh = { &_laser_mesh[0], 3};
							
							Mesh bullet_mesh = {&bullet[0], 2};
							
							//canvas.draw_mesh(0, laser_mesh, WHITE);
							
							if(meshes_overlap2(0, bullet_mesh, 0, laser_mesh))
							{
								f32 _distance = distance(laser_pos, bullet[0]);
								if(_distance < short_distance)
								{
									closest_hit = laser_pos + laser_dir * _distance;
									short_distance = _distance;
									hit_idx = i2;
									hit_type = type;
									impact = true;
								}
							}  
						}
						
						if(!impact && line_intersection(laser_pos, closest_hit, bullet[0], bullet[1], hit))
						{
							f32 _distance = distance(laser_pos, hit);
							if(_distance < short_distance)
							{
								closest_hit = hit;
								short_distance = _distance;
								hit_idx = i2;
								hit_type = type;
							}
						}
					}break;
				}
			}

			u32 hit_id = (hit_type == Entity_Type::none)? 0 : game.entities[hit_idx].id;
			if(game.game_time >= laser->next_damage_time || hit_id != laser->last_hit_id)
			{
				game.draw_ui = true;
				laser->last_hit_id = hit_id;
				laser->source_weapon->ammo -= 1;
				Laser_Weapon_Data* weapon = (Laser_Weapon_Data*)&(laser->source_weapon->data[0]);
				laser->next_damage_time = game.game_time + weapon->damage_freq;
				
				Entity* hit_entity = &game.entities[hit_idx];
				
				//TODO: Implement this stuff!
				switch(hit_type)
				{
					case Entity_Type::asteroid:
					{
						Asteroid* asteroid = hit_entity->retrive_internal<Asteroid>();
						asteroid->hp -= weapon->laser_damage;
						if(asteroid->hp <= 0)
							murder_entity(hit_entity, laser->source_id);
						
					}break;
					
					case Entity_Type::bullet:
					{
						Bullet* bullet = hit_entity->retrive_internal<Bullet>();
						if(bullet->source_id != laser->source_id)
						{
							if(bullet->source_type != Entity_Type::player_ship)
								murder_entity(hit_entity, laser->source_id);
							else
								kill_entity(hit_entity);
						}
					}break;
					
					case Entity_Type::enemy_ship:
					{
						Enemy_Ship* enemy_ship = hit_entity->retrive_external<Enemy_Ship>();
						enemy_ship->hp -= weapon->laser_damage;
						
						if(enemy_ship->hp <= 0)
							murder_entity(hit_entity, laser->source_id);
					}break;
				}
			}
			
			laser->last_dir = laser_dir;
			laser->impact_p = closest_hit;
		}
    }
}


static void remove_dead_entities()
{
	for(u32 i = 0; i < game.active_entity_count && game.zombie_entity_count > 0; ++i)
	{
		Entity* entity = game.entities + i;
		
		if(!get_entity_flag(entity, Entity_Flags::alive))
		{
			switch(entity->type)
			{
				
				case Entity_Type::asteroid:
				{
					Asteroid* asteroid = entity->retrive_internal<Asteroid>();
					
					Particle_Defination pd;
			
					u32 emission_count = 0;
					pd.full_color = entity->color;
					pd.fade_start_time = game.game_time + 1;
					pd.life_time = game.game_time + 2;
					
					switch(asteroid->size)
					{
						case Size::small:
						{
							pd.min_speed = 10;
							pd.max_speed = 30;
							emission_count = 10;
						}break;
						
						case Size::medium:
						{
							pd.min_speed = 20;
							pd.max_speed = 40;
					
							
							emission_count = 20;
						}break;
						
						case Size::large:
						{
							pd.min_speed = 30;
							pd.max_speed = 50;
							
							emission_count = 30;
						}break;
					}
		
					
					Emission_Cone cone = { 0, TAU32, (f32)get_asteroid_properties(asteroid->size).min_radius };
					
					particle_system_emit(entity->position, cone, &pd, emission_count);
					
					game.active_asteroid_count -= 1;
					mem.free(asteroid->_mesh);
					
					
				}break;
				
				case Entity_Type::enemy_ship:
				{
					game.active_enemy_ship_count -= 1;
					Enemy_Ship* e_ship = entity->retrive_external<Enemy_Ship>();
					Enemy_Ship::Type enemy_type = e_ship->type;
					mem.free(e_ship->ship.local_mesh);
					mem.free(e_ship);
					
					Particle_Defination pd;
			
					u32 emission_count = 0;
					pd.full_color = entity->color;
					pd.fade_start_time = game.game_time + 0.2f;
					pd.life_time = game.game_time + 0.75f;
					
					pd.min_speed = 50;
					pd.max_speed = 70;
					emission_count = 30;

					Emission_Cone cone = { 0, TAU32, Pickup::radius };
					
					particle_system_emit(entity->position, cone, &pd, emission_count);
				}break;
				
				case Entity_Type::player_ship:
				{
					Ship* ship = entity->retrive_external<Ship>();
					
					if(game.game_time < ship->itime)
					{
						set_entity_flag(entity, Entity_Flags::alive, true);
						break;
					}
					
					Player* player = find_ship_owner(ship);
					player->lives -= 1;
					player->ship = 0;
					player->ship_id = 0;
					
					Timed_Event event = {0};
					if(player->lives > 0)
					{
						event.trigger_time = game.game_time + 1.5;
						event.type = Event_Type::spawn_ship;
						*((u32*)&event.payload) = player->player_id;
					}
					else
					{
						event.trigger_time = game.game_time + 3.0;
						
                        u32 score_ranking = add_new_score(player->score);
                        save_settings_and_score(true);
                        
						if(score_ranking == 1)
                            event.type = Event_Type::game_over_highscore;
                        else                            
                            event.type = Event_Type::game_over;
					}
					
					add_timed_event(event);
					
					game.draw_ui = true;
					
					mem.free(ship->local_mesh);
					mem.free(ship);
					
					Particle_Defination pd;
			
					u32 emission_count = 0;
					pd.full_color = entity->color;
					pd.fade_start_time = game.game_time + 0.2f;
					pd.life_time = game.game_time + 0.75f;
					
					pd.min_speed = 50;
					pd.max_speed = 70;
					emission_count = 30;

					Emission_Cone cone = { 0, TAU32, Pickup::radius };
					
					particle_system_emit(entity->position, cone, &pd, emission_count);
					
				}break;
				
				case Entity_Type::pickup:
				{
					game.pickup_count -= 1;
					
				}break;
			}
		
			// Check if the entity resurected it self.
			if(!get_entity_flag(entity, Entity_Flags::alive))
			{
				game.active_entity_count -= 1;
				*entity = game.entities[game.active_entity_count];
				i -= 1;
			}
			
			game.zombie_entity_count -= 1;
		}
	}
	
	Assert(game.zombie_entity_count == 0);
}


static void process_timed_events()
{
	f64 time_stamp = game.game_time;
    Timed_Event* timed_events = game.timed_events;
    
    for(u32 i = 0; i < game.timed_event_count; ++i)
    {
		if(time_stamp >= timed_events[i].trigger_time)
		{
			//trigger!
			switch(timed_events[i].type)
			{
				case Event_Type::spawn_ship:
				{
					u32* player_id = (u32*)timed_events[i].payload;
					Player* player = 0;
					for(u32 p = 0; p < game.active_player_count; ++p)
						if(game.player_table[p].player_id == *player_id)
						{
							player = &game.player_table[p];
							break;
						}
					Assert(player);
					
					Entity* ship_entity = spawn_player_ship(canvas.get_middle(), player->color, 2, player->spawn_weapon);
					
					player->ship = ship_entity->retrive_external<Ship>();
					player->ship_id = ship_entity->id;
					
				}break;

				case Event_Type::game_over:
				{
                    remove_ui_canvas();
                    game.timed_event_count = 0;
					gui_create_death_menu();
				}return; // NOTE the return here!
                
                case Event_Type::game_over_highscore:
				{
                    remove_ui_canvas();
                    game.timed_event_count = 0;
                    gui_create_highscore_menu();
                    
				}return; // NOTE the return here!
				
				case Event_Type::spawn_wave:
				{
					f64 next_wave_time = generate_wave();
					Timed_Event event = { time_stamp + next_wave_time, Event_Type::spawn_wave};
					add_timed_event(event);
				}break;

				case Event_Type::spawn_pickups:
				{
					f32 t = 1.f + square(game.rm.random_f32() * 7.f); // 1 + 7 * 7 = 49 max = 50s
					Timed_Event event = { time_stamp + t, Event_Type::spawn_pickups};
					add_timed_event(event);

					if(!timed_events[i].payload[0])
					{
						if(game.rm.random_u32(10) <= 2)
						{
							spawn_pickup(generate_pickup());
							spawn_pickup(generate_pickup());
						}
						else
							spawn_pickup(generate_pickup());
					} 
					
				}break;
				
				case Event_Type::debug_respawn_enemy_ship:
				{
					Enemy_Ship::Type enemy_type = *((Enemy_Ship::Type*)&timed_events[i].payload);
					create_enemy_ship(random_point_in_rect(&game.rm, canvas.m_dimensions), 0, enemy_type);
				}break;
			}
			
			game.timed_event_count -= 1;
			timed_events[i] = timed_events[game.timed_event_count];
		}
    }
}


static void draw_game()
{
	// Draw Lasers.
	for(u32 i = 0; i < game.active_laser_count; ++i)
    {
		Laser* laser = &game.laser_table[i];
		v2f p1 = laser->pos;
		v2f p2 = laser->impact_p;
		if(p1 == p2)
			continue;
		
		canvas.draw_line(p1.As<i32>(), p2.As<i32>(), laser->color);
    }
	
	// Draw entites.
	for(u32 i = 0; i < game.active_entity_count; ++i)
	{
		Entity* entity = &game.entities[i];
		switch(entity->type)
		{
			case Entity_Type::enemy_ship:
			{
				Enemy_Ship* e_ship = entity->retrive_external<Enemy_Ship>();
				AIV_Call(canvas.draw_circle(e_ship->ai.target_position.As<i32>(), 10, 3, WHITE);)
		
			}
			case Entity_Type::player_ship:
			{
				Ship* ship = entity->retrive_external<Ship>();
				Assert(mem.address_inside_region((u8*)ship->local_mesh));
				Assert(mem.address_inside_region((u8*)ship->local_mesh + ship->mesh.p_count));
				u32 color = (game.game_time < ship->itime)? ship->icolor : entity->color;
				
				//canvas.draw_rect(0, create_rect_center_halfdim(entity->position, ship->width), WHITE);
				canvas.draw_mesh(entity->position, Mesh{ship->local_mesh, ship->mesh.p_count}, color);
				
			}break;

			case Entity_Type::bullet:
			{
				v2f p1 = entity->position;
				v2f p2 = p1 + normalize(entity->velocity) * -Bullet::trace_lenght;
					
				canvas.draw_line(p1.As<i32>(), p2.As<i32>(), entity->color);
			
			}break;

			case Entity_Type::asteroid:
			{
				Asteroid* asteroid = entity->retrive_internal<Asteroid>();
				Assert(mem.address_inside_region((u8*)asteroid->_mesh));
				Assert(mem.address_inside_region((u8*)asteroid->_mesh + (u32)asteroid->mesh_p_count));
				
				// canvas.draw_rect(entity->position, asteroid->bounding_box, WHITE);
				canvas.draw_mesh(entity->position, asteroid->mesh(), entity->color);
				
			}break;

			case Entity_Type::pickup:
			{
				Pickup* pickup = entity->retrive_internal<Pickup>();
				v2i pos = entity->position.As<i32>();
				canvas.draw_mesh(entity->position, transient.pickup_mesh, entity->color);
				
				const u8* font = &s_terminus_font[0];
				u32 char_width = s_terminus_font_char_width;
				u32 char_height = s_terminus_font_char_height;

				char character[2] = { 0, 0 };
				character[0] = pickup->display;
				
				
				v2i scale = {1,1};
				pos.x -= char_width * scale.x / 2;
				pos.y -= char_height * scale.y / 2;
				canvas.draw_text(&character[0], pos, entity->color, font, char_width, char_height, scale);
			
			}break;
		}
	}
	
	if(game.draw_ui && ui_canvas.m_pixels)
		draw_ui();
}


static void draw_ui()
{
    // TODO: Clean up and use the GUI_Font.
    
    if(!ui_canvas.m_pixels)
        return;
    
    game.draw_ui = false;
    
    ui_canvas.clear(put_color(20, 20, 20));
    ui_canvas.draw_border(WHITE);

    u32 ui_color = WHITE;

    if(game.active_player_count == 1)
    {
		u32 score = 0;
		u32 offset = 15;
		u32 inf_ammo = 0xFFFFFFFF;
		f32 ammo_percentile = 0;

		i32 player_lives = game.player_table[0].lives;
		score = game.player_table[0].score;

		Ship* ship = game.player_table[0].ship;
		if(ship && ship->weapon.ammo != inf_ammo)
		{
			ammo_percentile = ((f32)ship->weapon.ammo / (f32)ship->weapon.max_ammo);
		}
		for(i32 p = 0; p < player_lives; ++p)
		{
			ui_canvas.draw_mesh({(f32)(ui_canvas.m_dimensions.x - offset), 11.f}, transient.ship_mesh, ui_color);
			offset += 23;
		}

		const u8* font = &s_terminus_font[0];
		u32 char_width = s_terminus_font_char_width;
		u32 char_height = s_terminus_font_char_height;
		
		u8 buffer[11];
		// Draw current score
		{
			u8* score_text = u32_to_char_buffer(&buffer[0], sizeof(buffer), score);
			u32 text_offset = score_text - &buffer[0];
		
			//NOTE: This only works if the scale is set to 2 on the x axis.
			u32 char_count = (sizeof(buffer) - text_offset - 1); 
			i32 x_offset = char_count * s_terminus_font_char_width;
			if(char_count % 2 != 0)
				x_offset += s_terminus_font_char_width / 2;
			v2i p = { (i32)(ui_canvas.m_dimensions.x / 2) - x_offset, -3 };
		  
			ui_canvas.draw_text((char*)score_text, p, ui_color, font, char_width, char_height, {2, 2});
		}

		if(ammo_percentile > 0)
		{
			v2i p1, p2;
			p1 = {(i32)(ui_canvas.m_dimensions.x / 5), 4 };
			p2 = {p1.x + 100 ,(i32)ui_canvas.m_dimensions.y -4};
			buffer[0] = get_display_char_for_weapon(ship->weapon._type);
			buffer[1] = 0;
			u32 bg_color = put_color(20, 20, 20);
			u32 fill_color = put_color(200, 200, 200);
			ui_canvas.draw_percentile_bar( p1, p2, ammo_percentile, fill_color, bg_color, WHITE, 1);
			p1.x -= (i32)(char_width * 2) + 4;
			p1.y = -3;
			ui_canvas.draw_text((char*)&buffer[0], p1, ui_color, font, char_width, char_height, { 2 ,2 });
		}
		
		// Draw highscore
		{
			u8* score_text = u32_to_char_buffer(&buffer[0], sizeof(buffer), s_highscores[0]);
			ui_canvas.draw_text((char*)score_text, {2, -3}, ui_color, font, char_width, char_height, {2, 2});
		}
    }
}

static void draw_pause_menu()
{
	const u8* font = &s_terminus_font[0];
    u32 char_width = s_terminus_font_char_width;
    u32 char_height = s_terminus_font_char_height;

    char text[] = "PAUSED";
    v2i scale = {5,5};
    v2i p = (canvas.m_dimensions / 2).As<i32>();
    i32 offset = (sizeof(text) - 1) * scale.x * char_width / 2;
    p.x -= offset;
    
    canvas.draw_text(&text[0], p, WHITE, font, char_width, char_height, scale);
}


static inline void add_player(u32 color, v2f p, Weapon::type weapon)
{
    Assert(game.active_player_count < game.max_player_count);

    Player* player = &game.player_table[game.active_player_count];
    game.active_player_count += 1;
	
	player->spawn_weapon = weapon;
    player->color = color;
	Entity* ship_entity = spawn_player_ship(canvas.get_middle(), player->color, 0, player->spawn_weapon);
	
	player->ship = ship_entity->retrive_external<Ship>();
	player->ship_id = ship_entity->id;
    player->player_id = game.get_next_player_id();
    player->score = 0;
    player->lives = 3;
    
}


static Entity* spawn_player_ship(v2f pos, u32 color, f64 itime, Weapon::type weapon)
{
	Entity* entity = add_entity(Entity_Type::player_ship);

    entity->color = color;
	entity->position = pos;
	
	set_entity_flag(entity, Entity_Flags::immune_to_time_stop, true);
	
	Ship* ship = entity->alloc_external<Ship>(&mem);
	ship->width = transient.ship_mesh_width;
	
	ship->passed_screen = false;
    ship->icolor = WHITE;
    ship->itime = game.game_time + itime;
    ship->acceleration_speed = 200;
    ship->turn_speed = 5;
    
    ship->default_weapon_type = weapon;
	ship->weapon = create_weapon(ship->default_weapon_type);
    
    ship->mesh = transient.ship_mesh;
	ship->local_mesh = (v2f*)mem.push(sizeof(v2f) * ship->mesh.p_count);
        
    ship->orientation = 0;
    rotate_local_mesh(ship->local_mesh, ship->mesh, ship->orientation);
    
    game.draw_ui = true;
    
	return entity;
}


static inline void spawn_enemy_ship(Enemy_Ship::Type type)
{
	if(game.active_enemy_ship_count >= max_enemy_ship_count)
		return;
	
	v2f spawn_point = random_point_on_rim(&game.rm, game.asteroid_area_start, game.asteroid_area_end);
	
	f32 facing_direction = radian_wrap2(HALF_PI32 + angle_between_points(spawn_point, canvas.get_middle().As<f32>()));
	
	create_enemy_ship(spawn_point, facing_direction, type, AI::State::face_movement_target);
}


static void create_enemy_ship(v2f pos, f32 facing_direction, Enemy_Ship::Type type, AI::State start_state)
{
	Entity* entity = add_entity(Entity_Type::enemy_ship);
    entity->color = put_color(250, 80, 80);
    entity->position = pos;
	
	Enemy_Ship* enemy_ship = entity->alloc_external<Enemy_Ship>(&mem);
	enemy_ship->hp = 40;
    
    AI* ai = &enemy_ship->ai;
	*ai = AI();
	Ship* ship = &enemy_ship->ship;
    ship->itime = 0; //game.game_time + 3;
	ship->icolor = WHITE;
	ai->spook_range = 255;
    
	
    switch(type)
    {
		case Enemy_Ship::Type::slow:
		{
			ai->desired_velocity_magnitude = 80;
			
			ship->turn_speed = 2.5f;
			ship->acceleration_speed = 200;
			ship->default_weapon_type = Weapon::type::slow_scatter;

		}break;

		case Enemy_Ship::Type::fast:
		{
			ai->desired_velocity_magnitude = 300;
			
			ship->turn_speed = PI32;
			ship->acceleration_speed = 800;
			
			ship->default_weapon_type = Weapon::type::enemy_def;
			
		}break;
		
		case Enemy_Ship::Type::player_clone:
		{
			ai->desired_velocity_magnitude = 200;
			
			entity->color = MAGENTA;
			ship->acceleration_speed = 200;
			ship->turn_speed = 5;
			
			ship->default_weapon_type = Weapon::type::def;
			
		}break;
		
		default:
		{
			ai->desired_velocity_magnitude = 120;

			ship->turn_speed = PI32 * 1.2f;
			ship->acceleration_speed = 300;
			ship->default_weapon_type = Weapon::type::enemy_def;
			
		}
	    
    }
	ship->weapon = create_weapon(ship->default_weapon_type);
	
	
	if(type == Enemy_Ship::Type::player_clone)
	{
		ship->mesh = transient.ship_mesh;
		ship->width = 20;
	}
	else
	{
		u32 mesh_idx = get_enemy_mesh_idx_from_type(type);
		ship->mesh = transient.enemy_meshes[mesh_idx];
		ship->width = transient.enemy_mesh_widths[mesh_idx];	
	}
	
	ai->next_state = AI::State::none;
	ai->set_state(start_state, game.game_time);
    
    ai->target_position = pick_enemy_target_location(pos, ai->desired_velocity_magnitude);
    ship->local_mesh = (v2f*)mem.push(sizeof(v2f) * ship->mesh.p_count);
    
	ship->orientation = facing_direction;
    rotate_local_mesh(ship->local_mesh, ship->mesh, ship->orientation);

    game.active_enemy_ship_count += 1;
}


static void spawn_pickup(Pickup pickup)
{
	Entity* entity = add_entity(Entity_Type::pickup);
	entity->color = put_color(250, 220, 115);
	generate_vector_that_crosses_play_area(&entity->position, &entity->velocity);
    
	game.pickup_count += 1;
	
    Pickup* _pickup = entity->alloc_internal<Pickup>();
    *_pickup = pickup;
}


static void spawn_new_asteroid(Size size)
{
	if(game.active_asteroid_count >= max_asteroid_count)
		return;
	
    Entity* entity = create_asteroid(size);
    generate_vector_that_crosses_play_area(&entity->position, &entity->velocity);
}


static Entity* create_asteroid(Size size)
{
    Entity* entity = add_entity(Entity_Type::asteroid);
    entity->color = put_color(15, 255, 108);
    
    Asteroid* asteroid = entity->alloc_internal<Asteroid>();
	
	Asteroid_Properties prop = get_asteroid_properties(size);
    asteroid->mesh_p_count = prop.mesh_p_count;
    asteroid->_mesh = (v2f*)mem.push(sizeof(v2f) * asteroid->mesh_p_count);
    asteroid->hp = prop.hp;
    asteroid->size = size;
    
	
	f32 min_x = 0, min_y = 0;
	f32 max_x = 0, max_y = 0;
	
    // generate mesh
    {
		f32 a = TAU32 / ((u32)asteroid->mesh_p_count);
		f32 o = game.rm.random_f32() * TAU32;
		u32 diff = prop.max_radius - prop.min_radius;
		
		for(i32 i = 0; i < (i32)asteroid->mesh_p_count; ++i)
		{
			i32 r = game.rm.random_u32(diff) + prop.min_radius;
			v2f p = v2f{ cosf(a * i + o) , sinf(a * i + o) } * r;
			
			if(p.x < min_x)
				min_x = p.x;
			
			else if(p.x > max_x)
				max_x = p.x;
			
			if(p.y < min_y)
				min_y = p.y;
			
			else if(p.y > max_y)
				max_y = p.y;
			
			asteroid->_mesh[i] = p;
		}
    }

	asteroid->bounding_box = create_rect_min_max({ min_x, min_y }, { max_x, max_y });

    game.active_asteroid_count += 1;
    
    return entity;
}



static Pickup generate_pickup()
{
	// TODO: Consider pickup factory file, if this ends up clutterly.
    Pickup result;

    u32 r = game.rm.random_u32(100);
    if(0)
	{
		result._type = Pickup::type::life;
		result.display = '1';
    }
    else if(r < 10)
    {
		result._type = Pickup::type::time_stop;
		result.display = 'T';
    }
    else
    {
		result._type = Pickup::type::weapon;
		result.data = game.rm.random_u32((u32)Weapon::type::COUNT);
		result.display = get_display_char_for_weapon((Weapon::type)result.data);
    }
    
    result.passed_screen = false;
    return result;
}


static void pause_game()
{
    if(!game.is_paused)
    {
        game.is_paused = true;
        game.pause_time_start = platform.get_time_stamp();        
    }
}


static void unpause_game()
{
    if(game.is_paused)
    {
        game.is_paused = false;
        game.total_pause_time += platform.get_time_stamp() - game.pause_time_start;
        
        // Keeps poping menus till there are no more menus to pop.
        while(game.gui_handler.active_frame.widget_allocator.memory)
            gui_pop_frame(&game.gui_handler, &platform, &mem);
    }
}


static void unpause_game_without_destroy_frame()
{
    game.is_paused = false;
    game.total_pause_time += platform.get_time_stamp() - game.pause_time_start;   
}


static void record_ship_input()
{
    for(i32 i = 0; i < game.active_player_count; ++i)
    {    
		Ship* ship = game.player_table[i].ship;
        
		if(!ship)
			continue;

		if(!game.is_paused)
		{
			Ship::Input* input = &ship->input;

			if(get_action(Game_Actions::turn_right)->is_down())
                input->turn_dir = -1;

			if(get_action(Game_Actions::turn_left)->is_down())
                input->turn_dir = 1;

			if(get_action(Game_Actions::accelerate)->is_down())
                input->apply_thrust = true;

            // This here to make it so that after dying the player needs to release the shoot button inorder to shoot again.
			if(!get_action(Game_Actions::shoot)->is_down())
			   input->shoot_key_not_down = true;
			
			if(input->shoot_key_not_down && get_action(Game_Actions::shoot)->is_down())
                input->shoot = true;	   
		
        }
    }
}


static void clear_ship_input()
{
	for(i32 i = 0; i < game.active_player_count; ++i)
    {
		Ship* ship = game.player_table[i].ship;
		if(!ship)
			continue;

		ship->input.turn_dir = 0;
		ship->input.apply_thrust = 0;
		ship->input.shoot = 0;
    }
}


static void force_next_wave()
{
	// Super janky way of forcing next wave to happen if hostile count is 0.
    // The system is nice to use, even if not really fit for purpose, but
    // this kind of loop is still fast and rarely needed. (so maybe it's okey)
	
	for(u32 i = 0; i < game.timed_event_count; ++i)
		if(game.timed_events[i].type == Event_Type::spawn_wave)
		{
			game.timed_events[i].trigger_time = 0;
			break;
		}
}


static f64 generate_wave()
{
	game.wave_count += 1;	
	u32 r = game.rm.random_u32(1000);
	f64 next_wave_time = 10;
	u32 large_count = 0, medium_count = 0, small_count = 0;
	
	if(r < 10)
	{
		large_count = 3;
		small_count = 11;
		next_wave_time = 17;
	}
	else if(r < 80)
	{
		small_count = 22;
		next_wave_time = 25;
	}
	else if(r < 300)
	{
		medium_count = 2;
		large_count = 1;
		small_count = 4;
		next_wave_time = 20;
	}
	else if(r < 500)
	{
		large_count = 1;
		small_count = 2;
		next_wave_time = 7;
	}
	else if(r < 900)
	{
		medium_count = 2;
		next_wave_time = 5;
	}
	else
	{
		next_wave_time = 5;
	}


	for(u32 p = 0; p < large_count; ++p)
		spawn_new_asteroid(Size::large);	

	for(u32 p = 0; p < medium_count; ++p)
		spawn_new_asteroid(Size::medium);

	for(u32 p = 0; p < small_count; ++p)
		spawn_new_asteroid(Size::small);
	
	
	if(next_wave_time <= 10)
	{
		game.last_enemy_wave = game.wave_count;
		
		u32 default_enemy_count = 0, fast_enemy_count = 0, slow_enemy_count = 0;
		
		r = game.rm.random_u32(100);
		
		if(r < 50)
		{
			default_enemy_count = 1;
			next_wave_time += 5;
		}
		else if(r < 60)
		{
			fast_enemy_count = 1;
			next_wave_time += 5;
		}
		else if(r < 70)
		{
			slow_enemy_count = 1;
			next_wave_time += 5;
		}
		else if(r < 80)
		{
			default_enemy_count = 3;
			next_wave_time += 7;
		}
		else if (r < 95)
		{
			slow_enemy_count = 2;
			next_wave_time += 7;
		}
		else
		{
			default_enemy_count = 1;
			fast_enemy_count = 1;
			slow_enemy_count = 1;
			next_wave_time += 10;
		}
		
		for(u32 p = 0; p < default_enemy_count; ++p)
			spawn_enemy_ship(Enemy_Ship::Type::def);	

		for(u32 p = 0; p < fast_enemy_count; ++p)
			spawn_enemy_ship(Enemy_Ship::Type::fast);

		for(u32 p = 0; p < slow_enemy_count; ++p)
			spawn_enemy_ship(Enemy_Ship::Type::slow);

	}
	
	
	return next_wave_time;
}

//TODO: This should be moved in asteroid maths.
static inline void generate_vector_that_crosses_play_area(v2f* position, v2f* velocity)
{
	//TODO: Better way to create a random vector that crosses the play region.
	
	Assert(position && velocity);
	
	Random_Machine& rm = game.rm;

    *position = random_point_on_rim(&game.rm, game.asteroid_area_start, game.asteroid_area_end);
    
    // To avoid generating asteroids that move in a way that never crosses the play are, we check for that,
    // then reroll movement direction (v).
    v2f v;
    {
		v2f p = *position;
		f32 shrink_amount = 5;
		v2f shrink_region_start = { shrink_amount, shrink_amount };
		v2f shrink_region_end = canvas.m_dimensions.As<f32>() - shrink_amount;

		f32 min_y = shrink_region_start.y;
		f32 max_y = shrink_region_end.y;
		f32 min_x = shrink_region_start.x;
		f32 max_x = shrink_region_end.x;
	   
		while(true)
		{
			v = { game.rm.random_f32() - 0.5f, game.rm.random_f32() - 0.5f };	
			v = normalize(v);
		
			// test against vertical "walls".
			if(v.x != 0)
			{
			// test against left.
			if(vector_intersects_axis(p.x, p.y, v.x, v.y, min_x, min_y, max_y))
				break;

			// test against right.
			if(vector_intersects_axis(p.x, p.y, v.x, v.y, max_x, min_y, max_y))
				break;
			}

			// test against vertical "horizontal".
			if(v.y != 0)
			{
			// test against top.
			if(vector_intersects_axis(p.y, p.x, v.y, v.x, min_y, min_x, max_x))
				break;

			// test against bottom..
			if(vector_intersects_axis(p.y, p.x, v.y, v.x, max_y, min_x, max_y))
				break;
		
			}
		}	
    }

    v *= game.rm.random_f32() * 100 + 30;
    *velocity = v;

}


static void quit_to_desktop()
{
	platform.set_flag(App_Flags::is_running, false);
}


static inline void process_global_actions()
{
    update_actions(&platform, &s_global_actions[0], (u32)Global_Actions::COUNT);
        
    if(s_global_actions[(u32)Global_Actions::quit_game].is_pressed())
    {
        gui_create_quit_menu();
        return;
    }
    
    if(s_global_actions[(u32)Global_Actions::toggle_fullscreen].is_pressed())
    {
		u32 flags =  platform.get_flags();
		bool state = (flags & 1 << (u32)App_Flags::is_fullscreen) > 0;
		bool new_state = !state;
        platform.set_flag(App_Flags::is_fullscreen, new_state);

        // A really dumb way of toggling the checkbox a settings menu is up.
        GUI_Frame* frame = &game.gui_handler.active_frame;
        if(frame->widget_allocator.memory)
        {
            while(frame)
            {
                GUI_Widget_Header* header = (GUI_Widget_Header*)frame->widget_allocator.memory;
                for(u32 i = 0; i < frame->widget_count; ++i)
                {
                    u32 header_size = gui_get_widget_size(header);
                    if(header->type == GUI_Widget_Type::checkbox)
                    {
                        GUI_Checkbox* checkbox = (GUI_Checkbox*)header;
                        if(checkbox->spec.on_value_change == gui_set_fullscreen)
                        {
                            checkbox->spec.is_checked = new_state;
                        }                        
                        
                    }
                    
                    header = (GUI_Widget_Header*)((u8*)header + header_size);
                }
                
                frame = frame->prev_frame;
            }
            
        }
        
        save_settings_and_score(true);
    }
}

static inline void remove_ui_canvas()
{
    canvas = Pixel_Canvas(transient.pixel_buffer, transient.pixel_buffer_dimensions);
    ui_canvas = Pixel_Canvas(0, { 0, 0 });
}


static inline Action* get_action(Game_Actions action)
{
    return &s_game_actions[(u32)action];
}