
#define QUICKSTART 0
#define SPAWN_WAVE 1

/*

TODOS:
    - More pickups/weapons
    - Visual asteroid damage effects (hp representation with cracks)
    - Make it so that enemies can't shoot for a bit after entering the screen.
    - Redesign fast enemy.

*/


static void pause_game()
{
    if(!game.is_paused)
    {
        game.is_paused = true;
        game.pause_time_start = Platform_Get_Time_Stamp();
        
        Pause_All_Sounds_Of_Type(Sound_Types::effect);
    }
}


static void unpause_game_without_destroy_frame()
{
    game.is_paused = false;
    s_sound_player.paused = game.is_paused;
    game.total_pause_time += Platform_Get_Time_Stamp() - game.pause_time_start;   
}


static void unpause_game()
{
    if(game.is_paused)
    {
        game.is_paused = false;
        game.total_pause_time += Platform_Get_Time_Stamp() - game.pause_time_start;
        
        Continue_All_Sounds_Of_Type(Sound_Types::effect);
        
        // Keeps poping menus till there are no more menus to pop.
        while(game.gui_handler.active_frame.widget_allocator.memory)
        {
            gui_pop_frame(&game.gui_handler, &mem);
        }
    }
}


static void quit_to_desktop()
{
    game.running = false;
}


static void reset_game()
{
    Stop_All_Sounds_Of_Type(Sound_Types::effect);
    
    GUI_Theme* temp_theme = game.gui_handler.active_theme;
    
    mem.clear();
    
    game = Game();
    particle_system_clear();
    
    s_sound_player.paused = game.is_paused;
    game.gui_handler.active_theme = temp_theme;
}


static Entity* spawn_player_ship(v2f pos, u32 color, f64 itime, Weapon::type weapon)
{
    Entity* entity = add_entity(Entity_Type::player_ship);
    
    game.sound_listener_entity_id = entity->id;
    
    entity->color = color;
    entity->position = pos;
    
    set_entity_flag(entity, Entity_Flags::immune_to_time_stop, true);
    
    Ship* ship = entity->alloc_external<Ship>(&mem);
    ship->width = transient.ship_mesh_width;
    
    ship->passed_screen = false;
    ship->icolor = color_to_u32(WHITE);
    ship->itime = f32(game.game_time + itime);
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


static inline void add_player(u32 color, v2f p, Weapon::type weapon = Weapon::type::def)
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


static void start_game()
{
    // add ui canvas
    {
        v2u pixel_buffer_dimensions = transient.pixel_buffer_dimensions;
        
        u32 score_area_height = 28;
        pixel_buffer_dimensions.y -= score_area_height;
        canvas = Pixel_Canvas(transient.pixel_buffer + score_area_height * pixel_buffer_dimensions.x, pixel_buffer_dimensions);
        ui_canvas = Pixel_Canvas(transient.pixel_buffer, { pixel_buffer_dimensions.x, score_area_height });
        
        s32 asteroid_buffer_area = 100;
        game.asteroid_area_start = {-asteroid_buffer_area, -asteroid_buffer_area};
        game.asteroid_area_end = canvas.m_dimensions.As<s32>() + asteroid_buffer_area + 1;
        
    }
    
    game.game_time = Platform_Get_Time_Stamp();
    
    Random_Machine::seed = (u32)game.game_time;
    
    // Set pickup mesh
    {
        f32 a = TAU32 / transient.pickup_mesh.p_count;
        for(u32 i = 0; i < transient.pickup_mesh.p_count; ++i)
        {
            transient.pickup_mesh.data[i].x = cosf(a * i - HALF_PI32) * Pickup::radius;
            transient.pickup_mesh.data[i].y = sinf(a * i - HALF_PI32) * Pickup::radius;
        }
    }
    
    add_player(color_to_u32(Make_Color(235,00,235)), canvas.get_middle());
    
    #if SPAWN_WAVE
    add_timed_event(Timed_Event{ game.game_time + 0.5f, Event_Type::spawn_wave});
    add_timed_event(Timed_Event{ 0, Event_Type::spawn_pickups, 0 });    
    #endif
}


static void restart_game()
{
    reset_game();
    start_game();
}


static void set_mode_asteroids_sp()
{
    s_game_mode = Game_Mode::asteroids_sp;
    restart_game();
}


static inline void remove_ui_canvas()
{
    canvas = Pixel_Canvas(transient.pixel_buffer, transient.pixel_buffer_dimensions);
    ui_canvas = Pixel_Canvas(0, { 0, 0 });
}


static void create_enemy_ship(v2f pos, f32 facing_direction, Enemy_Ship::Type type,  AI::State start_state = AI::State::face_movement_target)
{
    Entity* entity = add_entity(Entity_Type::enemy_ship);
    entity->color = color_to_u32(Make_Color(250, 80, 80));
    entity->position = pos;
    
    Enemy_Ship* enemy_ship = entity->alloc_external<Enemy_Ship>(&mem);
    enemy_ship->hp = 40;
    
    AI* ai = &enemy_ship->ai;
    *ai = AI();
    Ship* ship = &enemy_ship->ship;
    ship->itime = 0; //game.game_time + 3;
    ship->icolor = color_to_u32(WHITE);
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
            game.sound_listener_entity_id = entity->id;
            
            ai->desired_velocity_magnitude = 200;
            
            entity->color = color_to_u32(MAGENTA);
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


static inline void spawn_enemy_ship(Enemy_Ship::Type type)
{
    if(game.active_enemy_ship_count >= max_enemy_ship_count)
        return;
    
    v2f spawn_point = random_point_on_rim(&game.rm, game.asteroid_area_start, game.asteroid_area_end);
    
    f32 facing_direction = radian_wrap2(HALF_PI32 + angle_between_points(spawn_point, canvas.get_middle().As<f32>()));
    
    create_enemy_ship(spawn_point, facing_direction, type, AI::State::face_movement_target);
}


static Entity* create_asteroid(Size size)
{
    Entity* entity = add_entity(Entity_Type::asteroid);
    entity->color = color_to_u32(Make_Color(15, 255, 108));
    
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
        
        for(s32 i = 0; i < (s32)asteroid->mesh_p_count; ++i)
        {
            s32 r = game.rm.random_u32(diff) + prop.min_radius;
            v2f p = v2f{cosf(a * i + o), sinf(a * i + o)} * (f32)r;
            
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
    
    asteroid->bounding_box = Create_Rect_Min_Max({ min_x, min_y }, { max_x, max_y });
    
    game.active_asteroid_count += 1;
    
    return entity;
}


static inline void generate_vector_that_crosses_play_area(v2f* position, v2f* velocity)
{
    // TODO: Better way to create a random vector that crosses the play region.
    
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
            v = Normalize(v);
            
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


static void spawn_new_asteroid(Size size)
{
    if(game.active_asteroid_count >= max_asteroid_count)
        return;
    
    Entity* entity = create_asteroid(size);
    generate_vector_that_crosses_play_area(&entity->position, &entity->velocity);
}


static void set_mode_main_menu()
{
    #if QUICKSTART
    
    set_mode_asteroids_sp();
    return;
    
    #endif
    
    Random_Machine::seed = (u32)game.game_time;
    
    s_game_mode = Game_Mode::main_menu;
    
    reset_game();
    
    remove_ui_canvas();
    
    s32 asteroid_buffer_area = 100;
    game.asteroid_area_start = {-asteroid_buffer_area, -asteroid_buffer_area};
    game.asteroid_area_end = canvas.m_dimensions.As<s32>() + asteroid_buffer_area + 1;
    
    gui_create_main_menu();
    
    //Spawn things into the background.
    {
        spawn_enemy_ship(Enemy_Ship::Type::player_clone);
        
        static constexpr u32 menu_asteroid_count = 6;
        
        for(u32 i = 0; i < menu_asteroid_count; ++i)
        {
            spawn_new_asteroid((Size)game.rm.random_u32(2));
        }
    }
}

static void init_asteroids_game()
{
    v2s screen_dim = {620, 480};
    Platform_Init("Blasteroids", {}, screen_dim, false, false);
    
    u32 game_state_memory_size = 50*MiB;
    void* game_state_memory = Platform_Allocate_Memory(game_state_memory_size, &game_state_memory_size);
    
    
    Color* pixel_buffer = Platform_Resize_Software_Render_Target_Pixel_Buffer(screen_dim);
    
    // Set up the font for text rendering.
    {
        GUI_Font* font = &s_gui_theme.font;
        
        font->char_width    = s_terminus_font_char_width;
        font->char_height   = s_terminus_font_char_height;
        font->data_buffer   = (u8*)s_terminus_font;
    }
    
    // Push transient memory in, and give the rest for the general allocator.
    {   
        Linear_Allocator transient_mem;
        transient_mem.init(game_state_memory, game_state_memory_size);
        generate_mesh_data(&transient_mem, &transient);
        
        {
            Allocator_Shell allocator = {};
            Init_Shell_From_Linear_Allocator(&allocator, &transient_mem);
            
            constexpr u32 kilobyte = 1024 * 1024;
            
            Linear_Allocator sound_loading_memory;
            u32 sound_loading_memory_size = 10000 * kilobyte;
            u8* memory = (u8*)Platform_Allocate_Memory(sound_loading_memory_size, &sound_loading_memory_size);
            sound_loading_memory.init(memory, sound_loading_memory_size);
            
            for(u32 i = 0; i < Sounds::COUNT; ++i)
            {
                transient.sounds[i] = Load_Wave(s_sound_names[i], &sound_loading_memory, &allocator);
            }
            
            Platform_Free_Memory(sound_loading_memory.memory);
            sound_loading_memory = {};
        }
        
        game.timed_events   = (Timed_Event*)transient_mem.push(sizeof(Timed_Event) * game.max_timed_event_count);
        game.entities       = (Entity*)transient_mem.push(sizeof(Entity) * game.max_entity_count);
        game.laser_table    = (Laser*)transient_mem.push(sizeof(Laser) * game.max_laser_count);
        game.player_table   = (Player*)transient_mem.push(sizeof(Player) * game.max_player_count);
        particle_system.particles = (Particle*)transient_mem.push(sizeof(Particle) * particle_system.max_particle_count);
        
        
        mem.init(transient_mem.next_free, transient_mem.get_free_capacity());
    }
    
    transient.pixel_buffer_dimensions = screen_dim.As<u32>();
    
    transient.pixel_buffer = (u32*)pixel_buffer;
    
    if(!load_settings_and_score())
    {
        load_default_menu_action();
        load_default_game_action();
        load_default_global_actions();
    }
    
    screen_canvas = Pixel_Canvas(transient.pixel_buffer, transient.pixel_buffer_dimensions);
    
    game.gui_handler.active_theme = &s_gui_theme;
    
    Sound_ID music_id = Play_Sound(get(Sounds::music), 0, Play_Mode::loop, Sound_Types::music);
    Fade_Sound(music_id, 3.0, Fade_Direction::in);
    
    set_mode_main_menu();
}

static inline void process_global_actions()
{
    update_actions(&s_global_actions[0], (u32)Global_Actions::COUNT);
    
    if(s_global_actions[(u32)Global_Actions::quit_game].is_pressed())
    {
        gui_create_quit_menu();
        return;
    }
    
    if(s_global_actions[(u32)Global_Actions::toggle_fullscreen].is_pressed())
    {
        u32 flags =  Platform_Get_Flags();
        bool state = (flags & Platform_Flags::fullscreen) > 0;
        bool new_state = !state;
        Platform_Set_Fullscreen(new_state);
        
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


static inline Action* get_action(Game_Actions action)
{
    return &s_game_actions[(u32)action];
}


static void record_ship_input()
{
    for(u32 i = 0; i < game.active_player_count; ++i)
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
    for(u32 i = 0; i < game.active_player_count; ++i)
    {
        Ship* ship = game.player_table[i].ship;
        if(!ship)
            continue;
        
        ship->input.turn_dir = 0;
        ship->input.apply_thrust = 0;
        ship->input.shoot = 0;
    }
}


static Pickup generate_pickup()
{
    // CONSIDER: pickup factory file, if this ends up clutterly.
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


static void spawn_pickup(Pickup pickup)
{
    Entity* entity = add_entity(Entity_Type::pickup);
    entity->color = color_to_u32(Make_Color(250, 220, 115));
    generate_vector_that_crosses_play_area(&entity->position, &entity->velocity);
    
    game.pickup_count += 1;
    
    Pickup* _pickup = entity->alloc_internal<Pickup>();
    *_pickup = pickup;
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
                    f32 t = 1.f + Square(game.rm.random_f32() * 7.f); // 1 + 7 * 7 = 49 max = 50s
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
    v2f a2 = a1 - (av * game.update_tick) - (Normalize(av) * Bullet::trace_lenght);
    
    v2f b1 = bullet2->position;
    v2f bv = bullet2->velocity;
    v2f b2 = b1 - (bv * game.update_tick) - (Normalize(bv) * Bullet::trace_lenght);
    
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
    
    Rect ship_bounding_box = create_rect_center_halfdim({0,0}, V2F(ship->width));
    bool bb_overlap = Rects_Overlap(
        player_ship_entity->position, 
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
    
    Rect ship_bounding_box = create_rect_center_halfdim(player_ship->position, V2F(ship->width));
    Rect pickup_bounding_box = create_rect_center_halfdim(pickup_entity->position, V2F(Pickup::radius * 2));
    
    if(!Rects_Overlap(ship_bounding_box, pickup_bounding_box))
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
        
        Particle_Definition pd;
        
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
    
    Rect ship_bounding_box = create_rect_center_halfdim({0,0}, V2F(ship->width));
    
    if(!Rects_Overlap(ship_bounding_box, Create_Rect_Min_Max({0,0}, canvas.m_dimensions.As<f32>())))
        return;
    
    Asteroid* asteroid = asteroid_entity->retrive_internal<Asteroid>();
    
    bool bb_overlap = Rects_Overlap(
        enemy_ship_entity->position, 
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
    b[1] = b[0] - (bv * game.update_tick) - (Normalize(bv) * Bullet::trace_lenght);
    
    Mesh ship_mesh =  { ship->local_mesh, ship->mesh.p_count};
    
    Rect ship_bounding_box = create_rect_center_halfdim(ship_entity->position, V2F(ship->width));
    Rect bullet_bounding_box = create_rect_center_halfdim(bullet_entity->position, V2F(Bullet::trace_lenght));
    bool bb_overlap = Rects_Overlap(ship_bounding_box, bullet_bounding_box);
    
    if(bb_overlap && meshes_overlap2({}, { &b[0], 2}, ship_entity->position, ship_mesh))
    {
        murder_entity(ship_entity, bullet->source_id);
    }
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


static void check_lasers_againt_targets()
{
    for(u32 i = 0; i < game.active_laser_count; ++i)
    {
        if(game.laser_table[i].alive == false)
        {
            Stop_Sound(game.laser_table[i].sound);
            game.active_laser_count -= 1;
            game.laser_table[i] = game.laser_table[game.active_laser_count];
            i -= 1;
            continue;
        }
        else
        {
            Laser* laser = game.laser_table + i;
            
            // NOTE: Laser weapons have to; on every frame keep set their alive flag to be true,
            // they are killed.
            laser->alive = false;
            
            v2f laser_pos = laser->pos;
            v2f laser_dir = { sinf(laser->dir) * -1, cosf(laser->dir) };
            //Assert(laser_dir != 0);
            
            Update_Sound_Position(laser->sound, laser_pos);
            
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
                    tx = vector_intersects_axis(laser_pos.x, laser_dir.x, (f32)canvas.m_dimensions.x);
                
                t = tx;
            }
            
            if(laser_dir.y != 0)
            {
                f32 ty = vector_intersects_axis(laser_pos.y, laser_dir.y, 0);
                if(ty < 0)
                    ty = vector_intersects_axis(laser_pos.y, laser_dir.y, (f32)canvas.m_dimensions.y);
                
                t = Min(t, ty);
            }
            
            v2f edge = laser->pos + laser_dir * t;
            v2f closest_hit = edge;
            f32 short_Distance = Distance(laser_pos, edge);
            u32 hit_idx = 0;
            Entity_Type hit_type = Entity_Type::none;
            
            for(u32 i2 = 0; i2 < game.active_entity_count; ++i2)
            {
                Entity_Type type = game.entities[i2].type;
                switch(type)
                {
                    case Entity_Type::asteroid:
                    {
                        // TODO: an opitimazation pass for this. First check againt a bounding box.
                        // Cross bb line check?
                        // use sqr Distance instead, just relative comparison, so no need for root.
                        Asteroid* asteroid = game.entities[i2].retrive_internal<Asteroid>();
                        v2f asteroid_pos = game.entities[i2].position;
                        v2f p1 = asteroid_pos + asteroid->_mesh[(u32)asteroid->mesh_p_count - 1];
                        
                        for(u32 j = 0; j < asteroid->mesh_p_count; ++j)
                        {
                            v2f hit;
                            v2f p2 = asteroid_pos + asteroid->_mesh[j];
                            if(line_intersection(laser_pos, closest_hit, p2, p1, hit))
                            {
                                f32 _Distance = Distance(laser_pos, hit);
                                if(_Distance < short_Distance)
                                {
                                    closest_hit = hit;
                                    short_Distance = _Distance;
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
                        // use sqr Distance instead, just relative comparison, so no need for root.
                        Ship* ship = game.entities[i2].retrive_external<Ship>();
                        v2f ship_pos = game.entities[i2].position;
                        v2f p1 = ship_pos + ship->local_mesh[ship->mesh.p_count - 1];
                        
                        for(u32 j = 0; j < ship->mesh.p_count; ++j)
                        {
                            v2f hit;
                            v2f p2 = ship_pos + ship->local_mesh[j];
                            if(line_intersection(laser_pos, closest_hit, p2, p1, hit))
                            {
                                f32 _Distance = Distance(laser_pos, hit);
                                if(_Distance < short_Distance)
                                {
                                    closest_hit = hit;
                                    short_Distance = _Distance;
                                    hit_idx = i2;
                                    hit_type = type;
                                }
                            }
                            p1 = p2;
                        }
                        
                    }break;
                    
                    case Entity_Type::bullet:
                    {
                        v2f hit;
                        v2f v = game.entities[i2].velocity;
                        
                        v2f bullet[2];
                        
                        bullet[0] = game.entities[i2].position;
                        bullet[1] = bullet[0] + (Normalize(v) * (-Bullet::trace_lenght * 2));
                        
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
                            
                            if(meshes_overlap2({}, bullet_mesh, {}, laser_mesh))
                            {
                                f32 _Distance = Distance(laser_pos, bullet[0]);
                                if(_Distance < short_Distance)
                                {
                                    closest_hit = laser_pos + laser_dir * _Distance;
                                    short_Distance = _Distance;
                                    hit_idx = i2;
                                    hit_type = type;
                                    impact = true;
                                }
                            }
                        }
                        
                        if(!impact && line_intersection(laser_pos, closest_hit, bullet[0], bullet[1], hit))
                        {
                            f32 _Distance = Distance(laser_pos, hit);
                            if(_Distance < short_Distance)
                            {
                                closest_hit = hit;
                                short_Distance = _Distance;
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
    // NOTE: For future projects. This kind of instant remove after death is not really a good idea.
    // If I had corpses, or death animations, this would make that much more difficult than need be.
    // ...but that said, this project is simple enough that I don't feel the need to change it here.
    
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
                    
                    Particle_Definition pd;
                    
                    u32 emission_count = 0;
                    pd.full_color = entity->color;
                    pd.fade_start_time = game.game_time + 1;
                    pd.life_time = game.game_time + 2;
                    Range explosion_volume = {};
                    
                    switch(asteroid->size)
                    {
                        
                        case Size::small:
                        {
                            pd.min_speed = 10;
                            pd.max_speed = 30;
                            
                            emission_count = 10;
                            
                            explosion_volume = {0.4f, 0.9};

                        }break;
                        
                        case Size::medium:
                        {
                            pd.min_speed = 20;
                            pd.max_speed = 40;
                            
                            emission_count = 20;
                            
                            explosion_volume = {0.7f, 1.1};
                        }break;
                        
                        case Size::large:
                        {
                            pd.min_speed = 30;
                            pd.max_speed = 50;
                            
                            emission_count = 30;
                            
                            explosion_volume = {0.9f, 1.5};
                        }break;
                    }
                    
                    Mesh mesh = asteroid->mesh();
                    particle_system_emit(entity->position, &mesh, &pd, emission_count);
                    
                    game.active_asteroid_count -= 1;
                    mem.free(asteroid->_mesh);
                    
                    Sound* s = get(Sounds::asteroid_explosion);
                    v2f* p = &entity->position;
                    Play_Sound(s, p, Play_Mode::once, Sound_Types::effect, explosion_volume, pitch_range);
                    
                }break;
                
                case Entity_Type::enemy_ship:
                {
                    game.active_enemy_ship_count -= 1;
                    Enemy_Ship* e_ship = entity->retrive_external<Enemy_Ship>();
                    Stop_Sound(e_ship->ship.thrust_sound);
                    Enemy_Ship::Type enemy_type = e_ship->type;
                    mem.free(e_ship->ship.local_mesh);
                    mem.free(e_ship);
                    
                    Particle_Definition pd;
                    
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
                    Stop_Sound(ship->thrust_sound);
                    
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
                    
                    Particle_Definition pd;
                    
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


static void physics_update()
{
    game.game_time = Platform_Get_Time_Stamp() - game.total_pause_time;
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
                    v2f ship_acceleration = {};
                    ship_acceleration.x += -orien_sin;
                    ship_acceleration.y += orien_cos;
                    ship_acceleration *= ship->acceleration_speed;
                    
                    update_position(&entity->position, &entity->velocity, ship_acceleration, game.update_tick);
                    
                    f32 inverse_ship_facing_angle = ship->orientation - HALF_PI32;
                    v2f inverse_ship_facing_vector = 
                        { cosf(inverse_ship_facing_angle), sinf(inverse_ship_facing_angle) };
                    
                    v2f emit_pos = inverse_ship_facing_vector * 5 + entity->position;
                    
                    if(!ship->accelerate_hold)
                    {
                        ship->accelerate_hold = true;
                        
                        while(!Fade_Sound(ship->thrust_sound, 1.f, Fade_Direction::in))
                        {
                            ship->thrust_sound = Play_Sound(get(Sounds::thruster), &emit_pos, Play_Mode::loop);
                        }
                    }
                    
                    Update_Sound_Position(ship->thrust_sound, emit_pos);
                    
                    if(game.game_time >= ship->next_thrust_emit_time)
                    {
                        ship->next_thrust_emit_time = game.game_time + 0.05f;
                        
                        Emission_Cone cone = { ship->orientation - HALF_PI32, 0.2f };
                        
                        Particle_Definition pd;
                        pd.min_speed = Length(entity->velocity) + 10;
                        pd.max_speed = pd.min_speed + 20;
                        pd.full_color = entity->color; 
                        pd.fade_start_time = game.game_time + 0.1f;
                        pd.life_time = game.game_time + 0.3f;
                        
                        particle_system_emit(emit_pos, cone, &pd, 10);
                    }
                }
                else
                {
                    Mixer_Slot* thrust_sound = Find_Sound_By_ID(ship->thrust_sound);
                    if(thrust_sound)
                    {
                        bool playing = (thrust_sound->flags & Sound_Flags::playing);
                        if(Bit_Not_Set(thrust_sound->flags, Sound_Flags::fading_out) && playing)
                        {
                            Fade_Sound(ship->thrust_sound, 1.f, Fade_Direction::out);
                            ship->accelerate_hold = false;
                        }
                        
                    }
                    update_position(&entity->position, &entity->velocity, game.update_tick);
                }
                
                if(ship->passed_screen && entity->type == Entity_Type::player_ship)
                {
                    clamp_position_to_rect(&entity->position, canvas.m_dimensions.As<s32>());
                }
                
                else
                {
                    if(point_inside_rect(entity->position, canvas.m_dimensions))
                        ship->passed_screen = true;
                    
                    clamp_position_to_rect(&entity->position, game.asteroid_area_start, game.asteroid_area_end);
                }
                
                
                if(ship->input.shoot)
                {
                    constexpr f32 spawn_Distance = 20;
                    f32 bx = entity->position.x + (orien_sin * spawn_Distance * -1);
                    f32 by = entity->position.y + (orien_cos * spawn_Distance);
                    v2f gun_mount_p = { bx, by };
                    
                    ship->weapon.fire(now, entity, gun_mount_p, ship->orientation);
                    if(ship->weapon.ammo == 0)
                    {
                        ship->weapon = create_weapon(ship->default_weapon_type);
                        ship->weapon.next_fire_time = now + ship->weapon.fire_rate;
                    }
                }
            }break;
            
            
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
                {
                    pickup->passed_screen = true;
                }
                else if(pickup->passed_screen)
                {
                    kill_entity(entity);
                }
                else
                {
                    clamp_position_to_rect(&entity->position, game.asteroid_area_start, game.asteroid_area_end);
                }
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
    
    if(game.sound_listener_entity_id)
    {
        Entity* entity;
        if(find_entity_by_id(game.sound_listener_entity_id, &entity))
        {
            Set_Listener_Location(entity->position);
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


static void draw_ui()
{
    // TODO: Clean up and use the GUI_Font.
    
    if(!ui_canvas.m_pixels)
        return;
    
    game.draw_ui = false;
    
    ui_canvas.clear(color_to_u32(Make_Color(20, 20, 20)));
    ui_canvas.draw_border(color_to_u32(WHITE));
    
    u32 ui_color = color_to_u32(WHITE);
    
    if(game.active_player_count == 1)
    {
        u32 score = 0;
        u32 offset = 15;
        u32 inf_ammo = color_to_u32(WHITE);
        f32 ammo_percentile = 0;
        
        s32 player_lives = game.player_table[0].lives;
        score = game.player_table[0].score;
        
        Ship* ship = game.player_table[0].ship;
        if(ship && ship->weapon.ammo != inf_ammo)
        {
            ammo_percentile = ((f32)ship->weapon.ammo / (f32)ship->weapon.max_ammo);
        }
        for(s32 p = 0; p < player_lives; ++p)
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
            u32 text_offset = u32(score_text - &buffer[0]);
            
            //NOTE: This only works if the scale is set to 2 on the x axis.
            u32 char_count = (sizeof(buffer) - text_offset - 1); 
            s32 x_offset = char_count * s_terminus_font_char_width;
            if(char_count % 2 != 0)
            x_offset += s_terminus_font_char_width / 2;
            v2s p = { (s32)(ui_canvas.m_dimensions.x / 2) - x_offset, -3 };
            
            ui_canvas.draw_text((char*)score_text, p, ui_color, font, char_width, char_height, {2, 2});
        }
        
        if(ammo_percentile > 0)
        {
            v2s p1, p2;
            p1 = {(s32)(ui_canvas.m_dimensions.x / 5), 4 };
            p2 = {p1.x + 100 ,(s32)ui_canvas.m_dimensions.y -4};
            buffer[0] = get_display_char_for_weapon(ship->weapon._type);
            buffer[1] = 0;
            u32 bg_color = color_to_u32(Make_Color(20, 20, 20));
            u32 fill_color = color_to_u32(Make_Color(200, 200, 200));
            ui_canvas.draw_percentile_bar( p1, p2, ammo_percentile, fill_color, bg_color, color_to_u32(WHITE), 1);
            p1.x -= (s32)(char_width * 2) + 4;
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
        
        canvas.draw_line(p1.As<s32>(), p2.As<s32>(), laser->color);
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
                AIV_Call(canvas.draw_circle(e_ship->ai.target_position.As<s32>(), 10, 3, WHITE);)
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
                v2f p2 = p1 + Normalize(entity->velocity) * -Bullet::trace_lenght;
                
                canvas.draw_line(p1.As<s32>(), p2.As<s32>(), entity->color);
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
                v2s pos = entity->position.As<s32>();
                canvas.draw_mesh(entity->position, transient.pickup_mesh, entity->color);
                
                const u8* font = &s_terminus_font[0];
                u32 char_width = s_terminus_font_char_width;
                u32 char_height = s_terminus_font_char_height;
                
                char character[2] = { 0, 0 };
                character[0] = pickup->display;
                
                
                v2s scale = {1,1};
                pos.x -= char_width * scale.x / 2;
                pos.y -= char_height * scale.y / 2;
                canvas.draw_text(&character[0], pos, entity->color, font, char_width, char_height, scale);
            }break;
        }
    }
    
    if(game.draw_ui && ui_canvas.m_pixels)
        draw_ui();
}


static bool update_asteroids_game(f64 delta_time, bool& update_surface)
{
    u64 flags = Platform_Get_Flags();
    
    if(flags & Platform_Flags::wants_to_exit)
    {
        gui_create_quit_menu();
        return game.running;
    }
    
    if(flags & Platform_Flags::focused)
    {
        process_global_actions();
    }
    
    else if(!gui_menu_is_up(&game.gui_handler) && !game.is_paused)
    {
        gui_create_pause_menu();
    }
    
    gui_handle_input(&game.gui_handler, &s_menu_actions[0]);
    
    if(!game.is_paused)
    {
        if(!gui_menu_is_up(&game.gui_handler))
        {
            update_actions(&s_game_actions[0], (u32)Game_Actions::COUNT);
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
        if(Platform_Get_Keyboard_Key_Down(Key_Code::T))
        {
            f32 speed_up = 5.f;
            
            f32 real_delta = (f32)delta_time;
            delta_time *= speed_up;
            
            s_sound_player.time_scale = speed_up;
            game.total_pause_time -= (delta_time - real_delta);
        }
        else
        {
            s_sound_player.time_scale = 1.f;
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
    
    
    f64 time_stamp = Platform_Get_Time_Stamp();
    if(time_stamp >= game.next_draw_time)
    {
        game.next_draw_time = time_stamp + game.draw_frequency;
        u8 c = 25;
        u32 clear_color = color_to_u32(Make_Color(c, c, c));
        
        canvas.clear(clear_color);
        
        f32 particle_delta = game.is_paused? 0 : (f32)game.draw_frequency;
        
        particle_system_update_and_draw(&canvas, particle_delta, game.game_time, clear_color);
        
        draw_game();
        
        gui_draw_widgets(&game.gui_handler, &screen_canvas);
        
        update_surface = true;
    }
    
    return game.running;
}


static void draw_pause_menu()
{
    const u8* font = &s_terminus_font[0];
    u32 char_width = s_terminus_font_char_width;
    u32 char_height = s_terminus_font_char_height;
    
    char text[] = "PAUSED";
    v2s scale = {5,5};
    v2s p = (canvas.m_dimensions).As<s32>() / 2;
    s32 offset = (sizeof(text) - 1) * scale.x * char_width / 2;
    p.x -= offset;
    
    canvas.draw_text(&text[0], p, color_to_u32(WHITE), font, char_width, char_height, scale);
}
