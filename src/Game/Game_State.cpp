
#pragma once

static inline Entity* add_entity(Entity_Type type)
{
    Assert(game.active_entity_count < game.max_entity_count);
    
    Entity* entity = &game.entities[game.active_entity_count];
    *entity = Entity();
    
    entity->type = type;
    entity->id = game.get_next_entity_id();
    set_entity_flag(entity, Entity_Flags::alive, true);
    
    ++game.active_entity_count;
    
    return entity;
}


static inline void kill_entity(Entity* entity)
{
    Assert(entity);
    
    if(!get_entity_flag(entity, Entity_Flags::alive))
        return;
    
    game.zombie_entity_count += 1;
    set_entity_flag(entity, Entity_Flags::alive, false);
}   


static bool find_entity_by_id(u32 entity_id, u32* out_idx)
{
    Assert(out_idx);
    Assert(entity_id);
    
    for(u32 result_idx = 0; result_idx < game.active_entity_count; ++result_idx)
    {
        if(game.entities[result_idx].id == entity_id)
        {
            *out_idx = result_idx;
            return true;
        }
    }
    
    return false;
}


static bool find_entity_by_id(u32 entity_id, Entity** out_entity)
{
    Assert(entity_id);
    Assert(out_entity);
    
    for(u32 result_idx = 0; result_idx < game.active_entity_count; ++result_idx)
    {
        if(game.entities[result_idx].id == entity_id)
        {
            *out_entity = &game.entities[result_idx];
            return true;
        }
    }
    
    return false;
}


static inline void add_timed_event(Timed_Event event)
{
    Assert(game.timed_event_count < game.max_timed_event_count);
    
    game.timed_events[game.timed_event_count] = event;
    game.timed_event_count += 1;
}


static Player* find_ship_owner(Ship* ship)
{
    Assert(ship);
    
    for(u32 i = 0; i < game.active_player_count; ++i)
        if(game.player_table[i].ship == ship)
            return &game.player_table[i];
    
    Terminate;
    return 0;
}


static inline void murder_entity(Entity* entity, Entity* murderer)
{
    Assert(entity);
    
    if(!get_entity_flag(entity, Entity_Flags::alive))
        return;
    
    kill_entity(entity);
    
    if(murderer && entity != murderer && murderer->type == Entity_Type::player_ship)
    {
        u32 score = get_kill_score_for_entity(entity);
        if(score == 0)
            return;
                
        Player* player = find_ship_owner(murderer->retrive_external<Ship>());
        player->give_score(score);
        game.draw_ui = true;
    }
}


static inline void murder_entity(Entity* entity, u32 murderer_id)
{
    Entity* murderer = 0;
    if(find_entity_by_id(murderer_id, &murderer))
        murder_entity(entity, murderer);
    
    else
        kill_entity(entity);
}
