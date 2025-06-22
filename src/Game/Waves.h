
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
        // NOTE: The fast enemy at this point is not usable, sooo just replace it with the default.
        spawn_enemy_ship(Enemy_Ship::Type::def);
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

static u32 add_and_return(u32* v, u32 inc)
{
    *v += inc;
    return *v;
}

static f64 spawn_increasingly_difficult_wave()
{
    u32 small_asteroid  = 0;
    u32 medium_asteroid = 0;
    u32 large_asteroid  = 0;
    
    u32 basic_enemy     = 0;
    u32 scatter_enemy   = 0;
    u32 fast_enemy      = 0;
    
    f64 next_wave_time  = 1;
    
    f64 x = f64(game.wave_count);
    f64 w = 50;
    f64 e = 100;
    f64 n = 1.5;
    f64 exp = -(x / (w / 4));
    f64 rp = -e + 2 * e * (1.0/(1.0 + Powf64(n, exp))); 
    
    Random_Machine* rm = &game.rm;
    u32 r = rm->random_u32(u32(rp));
    u32 c = 0;
    
#if 0       // TUTORIAL
    goto TUTORIAL;
#endif
    
#if 0       // MEDIUM
    goto MEDIUM;
#endif
    
#if 0       // HARD
    goto HARD;
#endif
    
    
    if(0);
    
    // "Tutorial" waves
    /*
        Only a couple of asteroids (1-4).
        No enemies before, a wave treshold. After that,
        there is small change for a wave that contains only one enemy.
        
        Tutorial waves have a very forgiving next wave spawn time.
        The player should have enough time to clear the entire wave,
        before the next one comes.
    */
    else if(r < add_and_return(&c, 10))
    {
        TUTORIAL:;
        
        if(game.active_asteroid_count < 15)
        {
            next_wave_time = 15;
            u32 rmax = 45;
            
            if(game.wave_count > 5)             
            {
                rmax += 5;
            }
            
            if(game.wave_count > 10)             
            {
                rmax += 5;
            }
            
            r = rm->random_u32(rmax);
            c = 0;
            if(0);
            else if(r < add_and_return(&c, 5))
            {
                small_asteroid = 8;
            }
            else if(r < add_and_return(&c, 20))
            {
                medium_asteroid = 1;
                small_asteroid = 3;
            }
            else if(r < add_and_return(&c, 20))
            {
                large_asteroid = 1;
            }
            else
            {
                Assert(game.wave_count > 5);
                basic_enemy = 1;
            }
        }
    }
    
    // Medium waves
    /*
        Can contain many asteroids, and multiple enemies.
        A wave with enemies can also have couple of asteroids.
        
        Reasonable next wave times.
    */
    else if(r < add_and_return(&c, 70))
    {
        MEDIUM:;
        
        if(game.active_asteroid_count < 15)
        {
            next_wave_time = 8;
            u32 rmax = 130;
            r = rm->random_u32(rmax);
            c = 0;
            if(0);
            else if(r < add_and_return(&c, 5))
            {
                small_asteroid = 14;
            }
            else if(r < add_and_return(&c, 15))
            {
                large_asteroid  = 1;
                medium_asteroid = 2;
                small_asteroid  = 3;
            }
            else if(r < add_and_return(&c, 15))
            {
                medium_asteroid = 2;
                small_asteroid  = 5;
            }
            else if(r < add_and_return(&c, 10))
            {
                large_asteroid = 3;
            }
            else if(r < add_and_return(&c, 15))
            {
                if(game.active_asteroid_count < 7)
                {
                    large_asteroid = 1;
                }
                scatter_enemy = 1;
            }
            else if(r < add_and_return(&c, 15))
            {
                if(game.active_asteroid_count < 5 && rm->random_u32(2))
                {
                    small_asteroid = 1;
                    medium_asteroid = 1;
                }
                basic_enemy = 1;
            }
            
            else if(r < add_and_return(&c, 15))
            {
                if(game.active_asteroid_count < 5 && rm->random_u32(2))
                {
                    medium_asteroid = 1;
                    small_asteroid = 1;
                }
                
                scatter_enemy = 1;
            }
            
            else
            {
                if(game.active_asteroid_count < 5 && rm->random_u32(2))
                {
                    small_asteroid = 2;
                }
                fast_enemy = 1;
            }
            
            Assert(c < rmax);
        }
    }
    
    // Hard waves
    /*
        Very high amount of asteroids and enemies.
        
        Unforgiving next wave times. 
        The player should fall behind and eventually die.
    */
    else
    {
        HARD:;
        
        if(game.active_asteroid_count < 20)
        {
            next_wave_time = 6;
            u32 rmax = 125;
            r = rm->random_u32(rmax);
            c = 0;
            if(0);
            else if(r < add_and_return(&c, 5))
            {
                small_asteroid = 25;
                next_wave_time = 14;
            }
            else if(r < add_and_return(&c, 30))
            {
                large_asteroid  = 1 + rm->random_u32(1);
                medium_asteroid = 2 + rm->random_u32(2);
                small_asteroid  = 3 + rm->random_u32(7);
                next_wave_time = 8;
            }
            else if(r < add_and_return(&c, 30))
            {
                if(game.active_asteroid_count < 7)
                {
                    large_asteroid = 1;
                }
                scatter_enemy = 1 + rm->random_u32(2);
                next_wave_time = 3;
            }
            else if(r < add_and_return(&c, 30))
            {
                if(game.active_asteroid_count < 7)
                {
                    small_asteroid = 3;
                    medium_asteroid = 2;
                }
                basic_enemy = 2 + rm->random_u32(3);
                next_wave_time = 3;
            }
            else if(r < add_and_return(&c, 20))
            {
                if(game.active_asteroid_count < 7)
                {
                    medium_asteroid = 1;
                    small_asteroid = 3;
                }
                scatter_enemy = 1;
                basic_enemy = 1;
                fast_enemy = 1;
                next_wave_time = 3;
            }
            else
            {
                if(game.active_asteroid_count < 7)
                {
                    small_asteroid = 4;
                }
                fast_enemy = rm->random_u32(2);
                next_wave_time = 3;
            }
            
            Assert(c < rmax);
        }
    }
    
    spawn_asteroids(small_asteroid, medium_asteroid, large_asteroid);
    
    /*
    Due to limits in the AI code, enemy ships become useless when there are too many
    obstacles for them, so just skip spawnin enemies if they would just instantly die,
    as that looks silly.
    */
    if(game.active_asteroid_count < 20)
    {
        spawn_enemy_ships(basic_enemy, scatter_enemy, fast_enemy);
    }
    
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