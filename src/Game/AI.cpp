
#pragma once



inline void Threat_Pizza::clear_threat_pizza()
{
	for(u32 i = 0; i < slice_count; ++i)
		threat_pizza[i] = F32_MAX;
}


inline u32 Threat_Pizza::get_threat_pizza_idx_from_angle(f32 angle)
{
	f32 r = radian_wrap2(angle);
	u32 result = (u32)(r / (TAU32 / slice_count));
	if(result == slice_count)
		result = 0;
	
	Assert(result < slice_count);
	return result;
}


f32 Threat_Pizza::get_safest_direction(f32 start_angle)
{
	static constexpr f32 increment = TAU32 / slice_count;
	static constexpr u32 max_steps = slice_count / 2;
	
	f32 l_safest = 0;
	f32 l_result;
	i32 l_step_count;
	for(l_step_count = 0; l_step_count < max_steps; ++l_step_count)
	{
		f32 angle = start_angle + increment * l_step_count;
		u32 idx = get_threat_pizza_idx_from_angle(angle);
		f32 threat_distance = threat_pizza[idx];
		if(threat_distance == F32_MAX)
		{
			l_result = angle;
			l_safest = F32_MAX;
			break;
		}
		else if(threat_distance > l_safest)
		{
			l_result = angle;
			l_safest = threat_distance;
		}
	}
	
	f32 r_safest = 0;
	f32 r_result = 0;
	i32 r_step_count = 0;
	for(r_step_count = 0; r_step_count < max_steps; ++r_step_count)
	{
		f32 angle = start_angle - increment * r_step_count;
		u32 idx = get_threat_pizza_idx_from_angle(angle);
		f32 threat_distance = threat_pizza[idx];
		if(threat_distance == F32_MAX)
		{
			r_result = angle;
			r_safest = F32_MAX;
			break;
		}
		else if(threat_distance > l_safest)
		{
			r_result = angle;
			r_safest = threat_distance;
		}
	}
	
	f32 eval1 = square(l_step_count) + (F32_MAX - l_safest);
	f32 eval2 = square(r_step_count) + (F32_MAX - r_safest);
	
	if(eval1 < eval2)
	{
		//if(threat)
		// *threat = l_safest;
		return l_result;
	}
	else
	{
		//if(threat)
		// *threat = r_safest;
		return r_result;
	}
}
	

void Threat_Pizza::set_threat_level(f32 level, f32 start_angle, f32 end_angle, f32 middle_angle)
{
	Assert(start_angle != end_angle);
	
	static constexpr f32 increment = TAU32 / slice_count;
	static constexpr f32 epsilon = 0.0001f;
	
	start_angle = radian_wrap2(start_angle);
	end_angle = radian_wrap2(end_angle);
	middle_angle = radian_wrap2(middle_angle);
	
	
	f32 turn_distance_0;
	f32 turn_distance_1;
	f32 d1;
	f32 d2;
	
	d1 = get_turn_direction_to_face_target(start_angle, middle_angle, turn_distance_0);
	d2 = get_turn_direction_to_face_target(end_angle, middle_angle, turn_distance_1);
	
	u32 steps = (u32)((turn_distance_0 + turn_distance_1) / increment);
	for(u32 i = 0; i < steps; ++i)
	{
		f32 angle_offset = (increment * i + epsilon) * d1;
		f32 angle = start_angle + angle_offset;
		u32 idx = get_threat_pizza_idx_from_angle(angle);
		
		f32 d0;
		get_turn_direction_to_face_target(angle, middle_angle, d0);
		
		
		f32 threat_at_step = level + (PI32 - d0);
		if(threat_pizza[idx] > threat_at_step)
			threat_pizza[idx] = threat_at_step;
	}
}


static v2f pick_enemy_target_location(v2f position, f32 desired_velocity)
{
	//TODO: This is awfull, figure out something else. Maybe use the threat map instead?
	// Or just pick a random spot and let the threat detection code take care of this.
	
	v2f result;
	
	f32 ship_half_width = 20;
	static constexpr u32 max_attemps = 10;
	static constexpr f32 min_d = 300;
	static constexpr f32 max_d = 1200;
	
	for(u32 attempt = 0; attempt < max_attemps; ++attempt)
	{
		{
			f32 a = game.rm.random_f32() * TAU32;
			f32 d = 100 + game.rm.random_f32() * max_d;
			
			result = { cosf(a) * d, sinf(a) * d };
			clamp_position_to_rect(&result, canvas.m_dimensions.As<i32>());
		}
		
		v2f diff = result - position;
		v2f heading = normalize(diff);
		v2f left = { heading.y, -heading.x };
		v2f right = { -heading.y, heading.x };
		
		v2f lenght = heading * desired_velocity * 1;
		f32 md = magnitude(diff);
		f32 ml = magnitude(lenght);
		if(ml > md)
			lenght = heading * md;
		
		static constexpr u32 roi_mesh_p_count = 4;
		v2f roi_mesh[roi_mesh_p_count];
		build_rect_mesh(left, right, lenght, &roi_mesh[0]);
		Mesh roi_actual_mesh = {&roi_mesh[0], roi_mesh_p_count};
		
		
		Rect roi_rect = create_bounding_box_from_mesh(roi_actual_mesh);
		
		bool path_overlaps_asteroid = false;
		for(u32 i = 0; i < game.active_entity_count; ++i)
		{
			Entity* entity = &game.entities[i];
			
			if(entity->type != Entity_Type::asteroid)
				continue;
			
			Asteroid* asteroid = entity->retrive_internal<Asteroid>();
			
			bool bb_overlap = rects_overlap(position, roi_rect, entity->position, asteroid->bounding_box);
			
			if(bb_overlap && meshes_overlap(entity->position, asteroid->mesh(), position, roi_actual_mesh))
			{
				path_overlaps_asteroid = true;
				break;
			}
		}
		
		if(!path_overlaps_asteroid)
			break;
	}
	
	return result;
}


static void threat_map_add_asteroid(Entity* entity, Threat_Map_Element* map, u32* element_count, u32 array_size)
{
	Assert(entity);
	Assert(map);
	Assert(element_count);
	
	Threat_Map_Element new_element;
	{
		new_element.velocity = entity->velocity;
		new_element.nv = normalize(new_element.velocity);
		new_element.ro = { new_element.nv.y, -new_element.nv.x };
		new_element.lo = { -new_element.nv.y, new_element.nv.x };
		
		new_element.mv = magnitude(new_element.velocity);
		
		Asteroid* asteroid = entity->retrive_internal<Asteroid>();
		Asteroid_Properties properties = get_asteroid_properties(asteroid->size);
		new_element.size = properties.min_radius + (properties.max_radius - properties.min_radius) * 0.5f;
	}
	
	v2f map_dim = game.asteroid_area_end.As<f32>() - game.asteroid_area_start.As<f32>();
	
	//TODO: Only add the dublicates if the threat is close to the edge.
	
	//top, left 1
	new_element.position = entity->position + v2f{ -map_dim.x, map_dim.y };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//top, midle 2
	new_element.position = entity->position + v2f{ 0, map_dim.y };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//top, right 3
	new_element.position = entity->position + v2f{ map_dim.x, map_dim.y };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//mid, lef 4
	new_element.position = entity->position + v2f{ -map_dim.x, 0 };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//middle, midle 5
	new_element.position = entity->position;
	map[*element_count] = new_element;
	*element_count += 1;
	
	//mid, right 6
	new_element.position = entity->position + v2f{ map_dim.x, 0 };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//bottom, left 7
	new_element.position = entity->position + v2f{ -map_dim.x, -map_dim.y };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//bottom, middle 8
	new_element.position = entity->position + v2f{ 0, -map_dim.y };
	map[*element_count] = new_element;
	*element_count += 1;
	
	//bottom, right 9
	new_element.position = entity->position + v2f{ map_dim.x, -map_dim.y };
	map[*element_count] = new_element;
	*element_count += 1;
	
	
	
	Assert(sizeof(*map) * (*element_count) <= array_size);
}


static f32 add_threat_element_to_pizza(
	Threat_Pizza* pizza, 
	Threat_Map_Element* threat, 
	f32 ship_size, 
	v2f threat_mesh_origin,
	v2f* threat_mesh,
	v2f ship_position)
{
	
	f32 distance_to_threat = distance(ship_position, threat->position);
	f32 angle_towards_threat = angle_between_points(threat->position, ship_position);

	{		
		f32 start_angle =  (angle_towards_threat) - HALF_PI32 / 2;
		f32 end_angle = (angle_towards_threat) + HALF_PI32 / 2;
		
		pizza->set_threat_level(distance_to_threat, start_angle, end_angle, angle_towards_threat);	
	}
	
	v2f threat_points[4];
	threat_points[0] = threat_mesh_origin + threat_mesh[0];
	threat_points[1] = threat_mesh_origin + threat_mesh[1] + threat->lo * ship_size;
	threat_points[2] = threat_mesh_origin + threat_mesh[3];
	threat_points[3] = threat_mesh_origin + threat_mesh[2] + threat->ro * ship_size;
	
	f32 distances[4];
	for(u32 i = 0; i < 4; ++i)
	{
		f32 sqred_d = squared_distance(ship_position, threat_points[i]);
		distances[i] = sqred_d;
	}
	
	u32 nearest_idx = 0;
	u32 second_nearest_idx = 0;
	
	f32 best_value = F32_MAX;
	for(u32 i = 0; i < 4; ++i)
	{
		if(distances[i] < best_value)
		{
			nearest_idx = i;
			best_value = distances[i];
		}
	}
	
	distances[nearest_idx] = F32_MAX;
	
	best_value = F32_MAX;
	for(u32 i = 0; i < 4; ++i)
	{
		if(distances[i] < best_value)
		{
			second_nearest_idx = i;
			best_value = distances[i];
		}
	}
	
		
	Assert(nearest_idx != second_nearest_idx);
	
	v2f p1 = threat_points[nearest_idx];
	v2f p2 = threat_points[second_nearest_idx];
	
	AIV_Call(canvas.draw_circle(p1.As<i32>(), 10, 3, BLUE);)
	AIV_Call(canvas.draw_circle(p2.As<i32>(), 10, 3, BLUE);)
	
	
	f32 start_angle = angle_between_points(p1, ship_position);
	f32 end_angle = angle_between_points(p2, ship_position);
	
	Assert(start_angle != end_angle);
	
	pizza->set_threat_level(distance_to_threat, start_angle, end_angle, angle_towards_threat);
	
	return distance_to_threat;
	
}


static void check_for_enemy_AI_Interupt(Entity* enemy_ship, Threat_Map_Element* map, u32 map_element_count)
{
	// TODO: Cleanup this function.
	
	Assert(enemy_ship);
	Assert(map);

	static constexpr u32 threat_mesh_p_count = 4;
	struct Threat_Mesh
	{
		v2f points[threat_mesh_p_count];
		Rect threat_rect;
		Threat_Map_Element* threat;
	};
	
	u32 threat_mesh_count = 0;
	Threat_Mesh* threat_meshes = (Threat_Mesh*)mem.push(sizeof(Threat_Mesh) * map_element_count);
	
	Threat_Pizza pizza;
	pizza.clear_threat_pizza();
	
	Enemy_Ship* e_ship = enemy_ship->retrive_external<Enemy_Ship>();
	Ship* ship = &e_ship->ship;
			
	f32 ship_size = 20;
	f32 distance_to_threat = F32_MAX;
	Threat_Map_Element* closest_threat = 0;
	
	bool has_threat = 0;
	bool has_velocity = enemy_ship->velocity != 0;
	
	v2f snv = 0;
	v2f ship_lo = 0;
	v2f ship_ro = 0;
	
	static constexpr u32 velocity_mesh_p_count = 4;
	v2f velocity_mesh[velocity_mesh_p_count];
	
	Rect ship_mesh_rect = create_rect_center_halfdim(enemy_ship->position, ship->width);
	
	Mesh velocity_actual_mesh = { &velocity_mesh[0], velocity_mesh_p_count };
	Mesh ship_mesh = {ship->local_mesh, ship->mesh.p_count};
	
	Rect ship_velocity_rect;
	
	if(has_velocity)
	{
		snv = normalize(enemy_ship->velocity);
		ship_lo = v2f{ snv.y, -snv.x } * ship_size;
		ship_ro = v2f{ -snv.y, snv.x } * ship_size;
		build_rect_mesh(ship_lo, ship_ro, enemy_ship->velocity, &velocity_mesh[0]);
		AIV_Call(canvas.draw_mesh(enemy_ship->position, {&velocity_mesh[0], velocity_mesh_p_count}, YELLOW);)
		
		ship_velocity_rect = create_bounding_box_from_mesh(velocity_actual_mesh, enemy_ship->position);
	}
	
	for(Threat_Map_Element* threat = map; threat < map + map_element_count; ++threat)
	{		
		v2f r_offset = threat->ro * (threat->size + ship_size * 1.5f);
		v2f l_offset = threat->lo * (threat->size + ship_size * 1.5f);
		
		f32 facing_factor;
		get_turn_direction_to_face_target(atan2f(threat->nv.y, threat->nv.x), ship->orientation + HALF_PI32, facing_factor); 
		facing_factor = facing_factor / ship->turn_speed * 0.5f;
		
		v2f threat_lenght = (threat->nv * threat->size * 2.5) + (threat->nv * (facing_factor / game.update_tick));
		threat_lenght += threat->velocity * (threat->mv / ship->acceleration_speed);
		
		v2f effect_p = threat->position - threat->nv * threat->size;
		
		v2f threat_mesh[threat_mesh_p_count];
		build_rect_mesh(effect_p, l_offset, r_offset, threat_lenght, &threat_mesh[0]);
		Mesh threat_actual_mesh = {&threat_mesh[0], threat_mesh_p_count};
	
		Rect threat_rect = create_bounding_box_from_mesh(threat_actual_mesh, enemy_ship->position);

		bool ship_bb_overlap = true;
		bool velocity_bb_overlap = true;
	
		bool velocity_vs_asteroid = false;
		if(has_velocity)
		{
			velocity_bb_overlap = rects_overlap(ship_velocity_rect, threat_rect);
			velocity_vs_asteroid = velocity_bb_overlap && 
			meshes_overlap2(enemy_ship->position, velocity_actual_mesh, 0, threat_actual_mesh);
		}
		
		bool ship_vs_asteroid = false;
		if(!velocity_vs_asteroid)
		{
			ship_bb_overlap = rects_overlap(ship_mesh_rect, threat_rect);
			ship_vs_asteroid = ship_bb_overlap && meshes_overlap2(enemy_ship->position, ship_mesh, 0, threat_actual_mesh);
		}
		
		bool in_threat_area = ship_vs_asteroid || velocity_vs_asteroid;
		
		AIV_Call(canvas.draw_mesh(0, {&threat_mesh[0], threat_mesh_p_count}, in_threat_area? RED : WHITE);)
		
		if(in_threat_area)
		{
			has_threat = true;
			f32 _distance = add_threat_element_to_pizza(
				&pizza, 
				threat, 
				ship_size, 
				0,
				&threat_mesh[0], 
				enemy_ship->position);
			
			if(_distance < distance_to_threat)
			{
				distance_to_threat = _distance;
				closest_threat = threat;
			}
		}
		else
		{
			threat_meshes[threat_mesh_count].threat = threat;
			threat_meshes[threat_mesh_count].threat_rect = threat_rect;
			for(u32 i = 0; i < threat_mesh_p_count; ++i)
			{
				threat_meshes[threat_mesh_count].points[i] = threat_mesh[i];
			}
			threat_mesh_count += 1;
		}
	}
	
	if(has_threat)
	{
		Assert(closest_threat);

		v2f start_direction;
		f32 velocity_threat = 0;
		f32 velocity_angle;
		f32 start_angle = e_ship->ship.orientation + HALF_PI32;
		
		if(has_velocity)
		{
			velocity_angle = atan2f(enemy_ship->velocity.y, enemy_ship->velocity.x);
			velocity_threat = pizza.threat_pizza[pizza.get_threat_pizza_idx_from_angle(velocity_angle)];
			if(velocity_threat < F32_MAX)
				start_angle = velocity_angle + PI32;
		}
		
		f32 current_angle_threat = pizza.threat_pizza[pizza.get_threat_pizza_idx_from_angle(e_ship->ai.evade_direction)];
		
		bool direction_is_dangerous = false;
		f32 evade_direction;
		if(e_ship->ai.state == AI::State::evasive_manuvers && current_angle_threat == F32_MAX)
			evade_direction = e_ship->ai.evade_direction;
		
		else if(velocity_threat == F32_MAX)
			evade_direction = velocity_angle;
		
		else
		{	
			static constexpr u32 max_attempts = 3;
			for(u32 i = 0; i < max_attempts; ++i)
			{
				evade_direction = pizza.get_safest_direction(start_angle);
				
				snv = {cosf(evade_direction), sinf(evade_direction)};
				
				ship_lo = v2f{ snv.y, -snv.x } * ship_size;
				ship_ro = v2f{ -snv.y, snv.x } * ship_size;
				
				build_rect_mesh(ship_lo, ship_ro, snv * e_ship->ai.desired_velocity_magnitude, &velocity_mesh[0]);
				AIV_Call(canvas.draw_mesh(enemy_ship->position, {&velocity_mesh[0], velocity_mesh_p_count}, YELLOW);)
				
				Rect new_velocity_rect = create_bounding_box_from_mesh(velocity_actual_mesh, enemy_ship->position);
				
				for(Threat_Mesh* threat_mesh = threat_meshes; threat_mesh < threat_meshes + threat_mesh_count; ++threat_mesh)
				{
					if(threat_mesh->threat == 0)
						continue;
					
					bool bb_overlap = rects_overlap(new_velocity_rect, threat_mesh->threat_rect);
					
					bool evade_vs_asteroid = bb_overlap && meshes_overlap2(
						enemy_ship->position,
						{&velocity_mesh[0], velocity_mesh_p_count},
						0,
						{&threat_mesh->points[0], threat_mesh_p_count});
										  
					
					if(evade_vs_asteroid)
					{	
						direction_is_dangerous = true;			
						f32 _distance = add_threat_element_to_pizza(
							&pizza, 
							threat_mesh->threat, 
							ship_size, 
							0,
							&threat_mesh->points[0], 
							enemy_ship->position);
						
						if(_distance < distance_to_threat)
						{
							distance_to_threat = _distance;
							closest_threat = threat_mesh->threat;
						}
						
						threat_mesh->threat = 0;
					}
				}
				
				if(!direction_is_dangerous)
					break;
			}
			
			if(direction_is_dangerous)
				evade_direction = pizza.get_safest_direction(start_angle);
		}
		
		// Checks for nan
		Assert(evade_direction == evade_direction);
		
		e_ship->ai.evade_direction = evade_direction;
		
		f32 orientation_threat = pizza.threat_pizza[pizza.get_threat_pizza_idx_from_angle(ship->orientation + HALF_PI32)];
		
		e_ship->ai.threat_towards_orientation = orientation_threat;
		e_ship->ai.threat_towards_velocity = velocity_threat;
		e_ship->ai.target_position = closest_threat->position;
			
		e_ship->ai.next_state = AI::State::none;
		e_ship->ai.set_state(AI::State::evasive_manuvers, game.game_time);
		
		AIV_Call
		(
			//Draw the pizza
			for(u32 i = 0; i < Threat_Pizza::slice_count; ++i)
			{
				f32 increment = TAU32 / Threat_Pizza::slice_count;
				f32 threat = pizza.threat_pizza[i];
				u32 color = put_color(255 - (u8)min(threat, 255.f), 0, 0);
				f32 lenght = threat;
				
				if(threat == F32_MAX)
				{
					lenght = 10;
					color = GREEN;
				}
				
				v2f p2 = enemy_ship->position;
				p2 += v2f( cosf(i * increment) * lenght, sinf(i * increment) * lenght );
				
				canvas.draw_line(enemy_ship->position.As<i32>(), p2.As<i32>(), color);
			}
		)
	}
	
	if(!has_threat && e_ship->ai.state == AI::State::evasive_manuvers)
	{
		e_ship->ai.set_state(AI::State::slow_down, game.game_time);
	}
	
	
	mem.free(threat_meshes);
}


static inline void handle_enemy_AI(Entity* e_ship_entity)
{
	Enemy_Ship* e_ship = e_ship_entity->retrive_external<Enemy_Ship>();
	Ship* ship = &e_ship->ship;
	AI* ai = &e_ship->ai;
	Ship* target_ship = 0;

	Entity* target_entity = 0;
	u32 target_id = 0;

	ship->input = {0};
		
	if(game.active_player_count > 0)
	{
		// Find new target.
		for(u32 p = 0; p < game.active_player_count; ++p)
		{
			if(game.player_table[p].ship)
			{
				target_ship = game.player_table[p].ship;
				target_id = game.player_table[p].ship_id;
				find_entity_by_id(target_id, &target_entity);
				Assert(target_entity);
			}
		}
	}
	
	//There are no players, so shoot at asteroids I quess?
	else
	{
		Entity* best_target = 0;
		f32 best_angle = F32_MAX;
		
		
		Rect screen_rect = create_rect_min_max(v2f{0,0}, canvas.m_dimensions.As<f32>());
		
		for(u32 i = 0; i < game.active_entity_count; ++i)
		{
			Entity* entity = game.entities + i;
			if(entity->type != Entity_Type::asteroid)
				continue;
			
			Asteroid* asteroid = entity->retrive_internal<Asteroid>();
			
			//Can't shoot asteroids that are offscreen.
			if(!rects_overlap(v2f{0,0}, screen_rect, entity->position, asteroid->bounding_box))
				continue;
			
			
			v2f target_position = entity->position;
			f32 target_angle = angle_between_points(e_ship_entity->position, target_position);
			
			f32 diff = radian_wrap(abs(ship->orientation - (target_angle + HALF_PI32)));
			if(diff < best_angle)
			{
				best_angle = diff;
				best_target = entity;
			}
		}
		
		target_entity = best_target;
	}
	
	
	
	static constexpr f32 still_threshold = 4.f;
	static constexpr u32 max_iter = 3;
	AI::State _state = ai->state;
	for(u32 iter = 0; iter < max_iter; ++iter)
	{
		switch(ai->state)
		{
			case AI::State::evasive_manuvers:
			{
				f32 mv = 0;
				if(e_ship_entity->velocity != 0)
				{	
					mv = magnitude(e_ship_entity->velocity); 
					if(mv > ai->desired_velocity_magnitude * 3)
					{
						ai->set_state(AI::State::slow_down, game.game_time);
						break;
					}
				}
				
				v2f p2 = e_ship_entity->position + v2f( cosf(ai->evade_direction) * 300, sinf(ai->evade_direction) *  300 );
				AIV_Call(canvas.draw_line(e_ship_entity->position.As<i32>(), p2.As<i32>(), YELLOW);)
				
				f32 d0 = turn_ship_towards_angle(ai->evade_direction, ship, game.update_tick);
				
				f32 d1;
				f32 d2;
				
				{
					v2f ship_acceleration;
					ship_acceleration.x += -sinf(ship->orientation);
					ship_acceleration.y += cosf(ship->orientation);
					ship_acceleration *= ship->acceleration_speed;
					
					v2f next_p_with_thrust = e_ship_entity->position;
					v2f next_v_with_thrust = e_ship_entity->velocity;
					update_position(&next_p_with_thrust, &next_v_with_thrust, ship_acceleration, game.update_tick);
					
					v2f next_p_without_thrust = e_ship_entity->position;
					v2f next_v_without_thrust = e_ship_entity->velocity;
					update_position(&next_p_without_thrust, &next_v_without_thrust, game.update_tick);
					
					d1 = squared_distance(next_p_without_thrust, ai->target_position);
					d2 = squared_distance(next_p_with_thrust, ai->target_position);					
				}
				
				bool can_thrust = !(mv >= ai->desired_velocity_magnitude && ai->threat_towards_velocity == F32_MAX);
				if((d2 > d1 || d0 < 1.f) && can_thrust)
					ship->input.apply_thrust = true;
			}break;
			
			case AI::State::fire_and_stay:
			{
				if(magnitude(e_ship_entity->velocity) >= still_threshold)
				{
					ai->set_state(AI::State::slow_down, game.game_time);
					ai->next_state = AI::State::fire_and_stay;
					break;
				}
				
				if(target_entity)
				{
					v2f position = e_ship_entity->position;
					v2f target_position = target_entity->position;
					f32 target_angle = angle_between_points(e_ship_entity->position, target_position);
					
					// The spook code
					if(target_ship)
					{
						f32 eval1 = radian_wrap(abs((target_ship->orientation + HALF_PI32) - target_angle));
						if(eval1 < PI32 / 12 && distance(target_position, position) < ai->spook_range)
						{
							ai->set_state(AI::State::face_movement_target, game.game_time);
							ai->target_position = pick_enemy_target_location(position, ai->desired_velocity_magnitude);
							break;
						}
					}
					
					turn_ship_towards_angle(target_angle + PI32, ship, game.update_tick);
					
					f32 eval1 = radian_wrap(abs(ship->orientation - (target_angle + HALF_PI32)));
					if(eval1 < PI32 / 12)
						ship->input.shoot = true;
				}
				
				f64 time_in_state = game.game_time - ai->state_start_time;
				if(time_in_state > 3.f)
					ai->set_state(AI::State::none, game.game_time);
				
			}break;
			
			case AI::State::face_movement_target:
			{
				f32 angle = angle_between_points(ai->target_position, e_ship_entity->position);
				if(turn_ship_towards_angle(angle, ship, game.update_tick) < 1)
				{
					AI::State _state = ai->next_state;
					ai->next_state = AI::State::none;
					ai->set_state(AI::State::accelerate_towards_target, game.game_time);
					ai->next_state = _state;
				}
			}break;
			
			case AI::State::accelerate_towards_target:
			{
				f32 mv = magnitude(e_ship_entity->velocity);
				if(mv < ai->desired_velocity_magnitude)
					ship->input.apply_thrust = true;
				
				else if(target_entity)
				{
					f32 target_angle = angle_between_points(e_ship_entity->position, target_entity->position);
					turn_ship_towards_angle(target_angle + PI32, ship, game.update_tick);
					f32 angle_to_target = radian_wrap(abs(ship->orientation - (target_angle + PI32 * 0.5f)));
					
					if(angle_to_target < PI32 / 12)
					ship->input.shoot = true;
				}
				
				f32 turn_distance;
				f32 heading_angle = ship->orientation + HALF_PI32;
				f32 inverterd_velocity_angle = atan2f(e_ship_entity->velocity.y * -1, e_ship_entity->velocity.x * -1);
				get_turn_direction_to_face_target(heading_angle, inverterd_velocity_angle, turn_distance);
				
				f32 eval1 = distance(e_ship_entity->position, ai->target_position) / mv;
				f32 eval2 = (mv * 0.5f / ship->acceleration_speed) + (turn_distance / ship->turn_speed);
				if(eval1 < eval2)
				{
					ship->input.turn_dir = 0;
					ai->set_state(AI::State::slow_down, game.game_time);
				}
			}break;
			
			case AI::State::slow_down:
			{
				f32 mv = magnitude(e_ship_entity->velocity); 
				if(mv < still_threshold)
				{
					e_ship_entity->velocity = { 0, 0 };
					ai->set_state(AI::State::none, game.game_time);
				}
				else
				{
					f32 target_angle = atan2f((e_ship_entity->velocity.y * -1),(e_ship_entity->velocity.x * -1));
					if(turn_ship_towards_angle(target_angle, ship, game.update_tick) < 0.0001)
						ship->input.apply_thrust = true;			
				}	
			}break;
			
			case AI::State::do_nothing:
			break;
			
			default:
			{
				u32 r = game.rm.random_u32(100);
				
				if(!point_inside_rect(e_ship_entity->position, canvas.m_dimensions) || r < 30)
				{
					ai->set_state(AI::State::face_movement_target, game.game_time);
					ai->target_position = pick_enemy_target_location(e_ship_entity->position, ai->desired_velocity_magnitude);
				}
				else if(ai->last_state != AI::State::fire_and_stay)
				{
					ai->set_state(AI::State::fire_and_stay, game.game_time);
				}
			}
			
		} 
		
		if(ai->state != _state)
		{
			_state = ai->state;
		}
		else
		{
			break;
		}
	}
}

