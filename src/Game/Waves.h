
# pragma once

static void spawn_new_asteroid(Size size);
static inline void spawn_enemy_ship(Enemy_Ship::Type type);


static void force_next_wave()
{
    // Super janky way of forcing next wave to happen if hostile count is 0.
    // The system is nice to use, even if not really fit for purpose, but
    // this kind of loop is still fast and rarely needed. (so maybe it's okey)
    
    for(u32 i = 0; i < game.timed_event_count; ++i)
    {
        if(game.timed_events[i].type == Event_Type::spawn_wave)
        {
            game.timed_events[i].trigger_time = 0;
            break;
        }
    }
}


static inline void spawn_asteroids(u32 small, u32 medium, u32 large)
{
    for(u32 p = 0; p < large; ++p)
    {
        spawn_new_asteroid(Size::large);    
    }
    
    for(u32 p = 0; p < medium; ++p)
    {
        spawn_new_asteroid(Size::medium);
    }
    
    for(u32 p = 0; p < small; ++p)
    {
        spawn_new_asteroid(Size::small);
    }
}


static inline void spawn_enemy_ships(u32 basic, u32 scatter, u32 fast)
{
    for(u32 p = 0; p < basic; ++p)
    {
        spawn_enemy_ship(Enemy_Ship::Type::def);    
        
    }
    
    for(u32 p = 0; p < scatter; ++p)
    {
        spawn_enemy_ship(Enemy_Ship::Type::slow);
    }
    
    for(u32 p = 0; p < fast; ++p)
    {
        spawn_enemy_ship(Enemy_Ship::Type::fast);
    }
}


static f64 spawn_easy_wave()
{
    u32 r = game.rm.random_u32(10);
    f64 next_wave_time = 6 + f64(game.rm.random_u32(5));
    u32 large = 0, medium = 0, small = 0;
    
    if(r < 4)
    {
        medium = 1;
        small = 2;
    }
    else if (r < 7)
    {
        large = 1;
    }
    else
    {
        small = 4;
    }
    
    spawn_asteroids(small, medium, large);
    
    return next_wave_time;
}

static u32 return_and_add(u32* v, u32 inc)
{
    u32 temp = *v;
    *v += inc;
    return temp;
}

static f64 spawn_increasingly_difficult_wave()
{
    u32 small_asteroid  = 0;
    u32 medium_asteroid = 0;
    u32 large_asteroid  = 0;
    
    u32 basic_enemy     = 0;
    u32 scatter_enemy   = 0;
    u32 fast_enemy      = 0;

    f64 next_wave_time  = 10;
    
    u32 wave_cap = 50;
    
    f64 x = Powf64(game.wave_count, 1.8);
    u32 rp = Min((u32)Round(Root(f32(x))), wave_cap);
    u32 r = game.rm.random_u32(6 + rp);
    
    u32 c = 2;
    
    if(r < return_and_add(&c, 1))
    {
        next_wave_time = 0;
        small_asteroid = 1;
    }
    else if(r < return_and_add(&c, 2))
    {
        medium_asteroid = 2;
        small_asteroid = 2;
        next_wave_time = 10;
        
    }
    else if(r < return_and_add(&c, 2))
    {
        large_asteroid = 1;
        next_wave_time = 10;
    }
    else if(r < return_and_add(&c, 2))
    {
        medium_asteroid = 1;
        small_asteroid = 1;
        large_asteroid = 1;
        next_wave_time = 10;
    }
    else if(r < return_and_add(&c, 1))
    {
        basic_enemy = 1;
        next_wave_time = 30;
    }
    else if(r < return_and_add(&c, 3))
    {
        medium_asteroid = game.rm.random_u32(2);
        small_asteroid = 2;
        next_wave_time = 5;
        
    }
    else if(r < return_and_add(&c, 3))
    {
        large_asteroid = 1;
        next_wave_time = 4;
    }
    else if(r < return_and_add(&c, 3))
    {
        medium_asteroid = 1;
        small_asteroid = game.rm.random_u32(2);
        large_asteroid = 1;
        next_wave_time = 6;
    }
    else if(r < return_and_add(&c, 2))
    {
        basic_enemy = 1;
        next_wave_time = 5;
    }
    else if(r < return_and_add(&c, 1))
    {
        small_asteroid = 10;
        next_wave_time = 15;
    }
    else if(r < return_and_add(&c, 3))
    {
        large_asteroid = 1;
        medium_asteroid = 2;
        small_asteroid = 3 + game.rm.random_u32(10);
        next_wave_time = 20;
    }
    else if(r < return_and_add(&c, 3))
    {
        large_asteroid  = game.rm.random_u32(5);
        small_asteroid  = game.rm.random_u32(5);
        medium_asteroid = game.rm.random_u32(5);
        next_wave_time = 3;
    }
    else if(r < return_and_add(&c, 2))
    {
        medium_asteroid = 4;
        small_asteroid = 2;
        next_wave_time = 5;
    }
    else if(r < return_and_add(&c, 2))
    {
        medium_asteroid = 4;
        small_asteroid = 2;
        next_wave_time = 5;
    }
    else if(r < return_and_add(&c, 2))
    {
        basic_enemy = 2;
        medium_asteroid = 2;
        next_wave_time = 5;
    }
    else if(r < return_and_add(&c, 3))
    {
        small_asteroid = 15;
        next_wave_time = 15;
    }
    
    
    
    spawn_asteroids(small_asteroid, medium_asteroid, large_asteroid);
    spawn_enemy_ships(basic_enemy, scatter_enemy, fast_enemy);
    
    return next_wave_time;
}


static f64 generate_wave()
{
    game.wave_count += 1;
    f64 next_wave_time = spawn_increasingly_difficult_wave();
    #if 0
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
    
    
    spawn_asteroids(small_count, medium_count, large_count);
    
    
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
        {
            spawn_enemy_ship(Enemy_Ship::Type::def);    
            
        }
        
        for(u32 p = 0; p < fast_enemy_count; ++p)
        {
            spawn_enemy_ship(Enemy_Ship::Type::fast);
        }
        
        for(u32 p = 0; p < slow_enemy_count; ++p)
        {
            spawn_enemy_ship(Enemy_Ship::Type::slow);
        }
    }
    #endif
    return next_wave_time;
}