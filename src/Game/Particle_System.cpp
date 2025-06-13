
#pragma once


inline u32 multiply_accross_color_channels(u32 color, f32 mult)
{
    u8* p = (u8*)&color;
    for (u32 i = 0; i < 3; i++)
    {
        s32 mult_val = (s32)((*(p + i)) * mult);
        s32 val = Min(s32(255), Max(s32(0), mult_val));
        *(p + i) = (u8)val;
    }
    
    return color;
}


inline u32 multiply_accross_color_channels(u32 color, u32 min_values, f32 mult)
{
    u8* p = (u8*)&color;
    u8* m = (u8*)&min_values;
    for (u32 i = 0; i < 3; i++)
    {
        s32 mult_val = (s32)((*(p + i)) * mult);
        s32 val = Min(s32(255), Max((s32)(*(m+i)), mult_val));
        *(p + i) = (u8)val;
    }
    
    return color;
}


static inline void particle_system_clear()
{
    particle_system = Particle_System();
}


static void particle_system_emit(v2f source_position, Emission_Cone cone, Particle_Definition* pd, u32 count)
{
    Assert(pd);
    Particle* particles = particle_system.particles;

    for(u32 i = 0; i < count; ++i)
    {
        if(particle_system.active_particle_count >= particle_system.max_particle_count)
        {
            return;
        }
        
        Particle* particle = particles + particle_system.active_particle_count++;
        
        
        f32 angle = cone.direction_angle - cone.half_radius;
        angle += (cone.half_radius * 2) * particle_system.rm.random_f32();
        
        particle->velocity = v2f{cosf(angle), sinf(angle)};
        particle->position = source_position + particle->velocity * cone.offset;
        
        
        particle->velocity *= pd->min_speed + (pd->max_speed - pd->min_speed) * particle_system.rm.random_f32();
        
        particle->full_color = pd->full_color;
        particle->fade_start_time = pd->fade_start_time;
        particle->life_time = pd->life_time;
    }
}


static void particle_system_emit(v2f position, Mesh* mesh, Particle_Definition* pd, u32 count)
{
    Assert(pd);
    Assert(mesh);
    Particle* particles = particle_system.particles;
    
    // CONSIDER: Build a lookup table?
    
    f32 total_line_lenght = Distance(mesh->data[mesh->p_count - 1], mesh->data[0]);
    for(u32 i = 0; i < mesh->p_count - 1; ++i)
    {
        v2f a = mesh->data[i];
        v2f b = mesh->data[i + 1];
        
        total_line_lenght += Distance(a, b);
    }
    
    for(u32 i = 0; i < count; ++i)
    {
        if(particle_system.active_particle_count >= particle_system.max_particle_count)
        {
            return;
        }
        
        f32 d = particle_system.rm.random_f32() * total_line_lenght;
        f32 line_lenght = 0;
        
        for(u32 i = 0; i < mesh->p_count; ++i)
        {
            v2f a;
            v2f b;
            
            if(i == mesh->p_count - 1)
            {
                a = mesh->data[mesh->p_count - 1];
                b = mesh->data[0];
            }
            else
            {
                a = mesh->data[i];
                b = mesh->data[i + 1];
            }
            
            f32 pre_ld = line_lenght;
            line_lenght += Distance(a, b);
            if(d >= pre_ld && d < line_lenght)
            {
                Particle* particle = particles + particle_system.active_particle_count++;
                
                v2f segment_direction = Normalize(b - a);
                
                particle->velocity = Perp_CW(segment_direction);
                particle->position = position + a + segment_direction * (d - pre_ld);
                
                particle->velocity *= pd->min_speed + (pd->max_speed - pd->min_speed) * particle_system.rm.random_f32();
                
                particle->full_color = pd->full_color;
                particle->fade_start_time = pd->fade_start_time;
                particle->life_time = pd->life_time;
                
                break;
            }
        }
    }
}


static void particle_system_update_and_draw(
    Pixel_Canvas* canvas, 
    f32 delta_time, 
    f64 game_time, 
    u32 background_color)
{
    Assert(canvas);
    
    Particle* particles = particle_system.particles;
    
    for(u32 i = 0; i < particle_system.active_particle_count; ++i)
    {
        Particle* particle = particles + i;
        
        // Kill the particle.
        if(game.game_time > particle->life_time)
        {
            particle_system.active_particle_count -= 1;
            Particle* last_particle = particles + particle_system.active_particle_count; 
            *particle = *last_particle;
            i -= 1;
        }
        
        // Or update and draw the particle.
        else
        {
            if(delta_time > 0)
                update_position(&particle->position, &particle->velocity, delta_time);
            
            
            u32 color;
            if(game.game_time < particle->fade_start_time)
                color = particle->full_color;
    
            else
            {
                f32 f1 = f32(particle->life_time - particle->fade_start_time);
                f32 f2 = f32(game.game_time - particle->fade_start_time);
                f32 f3 = 1.0f - ( f2 / f1 );
                
                color = multiply_accross_color_channels(particle->full_color, background_color, f3);
            }
            
            canvas->set_pixel2(particle->position.As<s32>(), color);
        }   
    }
}
