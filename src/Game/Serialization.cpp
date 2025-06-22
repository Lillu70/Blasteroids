
#pragma once


#pragma pack(push, 1)
struct Action_Data
{
    Key_Code keyboard_mapping;
    Button controller_mapping;
};


struct File_Data
{
    static inline char file_name[] = "Blasteroids.data";
    static constexpr u32 file_format_version = 1;
    
    u32 version;
    u32 highscores[s_max_highscore_count];
    Action_Data game_action_data[(u32)Game_Actions::COUNT];
    Action_Data menu_action_data[(u32)GUI_Menu_Actions::COUNT];
    Action_Data global_action_data[(u32)Global_Actions::COUNT];
    
    f32 sfx_volume;
    f32 music_volume;
    
    bool is_muted;
    bool is_fullscreen;
};
#pragma pack(pop)


static void save_settings_and_score(bool force_save = false)
{    
    if(!force_save && !s_settings.dirty)
        return;
    
    // Pool all the data that needs to be saved into a buffer.
    /*
    -file version
    -scores (10 * u32)
    -game actions (if changed?)
    -menu actions
    -global actions
    
    -settings struct, except the dirty flag.
    -fullscreen
    */
    
    File_Data file_data;
    file_data.version = File_Data::file_format_version;
    
    for(u32 i = 0; i < s_max_highscore_count; ++i)
        file_data.highscores[i] = s_highscores[i];
    
    
    for(u32 i = 0; i < (u32)Game_Actions::COUNT; ++i)
    {
        Action* action = s_game_actions + i;
        file_data.game_action_data[i] = {action->keyboard_mapping, action->controller_mapping};
    }
    
    for(u32 i = 0; i < (u32)GUI_Menu_Actions::COUNT; ++i)
    {
        Action* action = s_menu_actions + i;
        file_data.menu_action_data[i] = {action->keyboard_mapping, action->controller_mapping};
    }
    
    for(u32 i = 0; i < (u32)Global_Actions::COUNT; ++i)
    {
        Action* action = s_global_actions + i;
        file_data.global_action_data[i] = {action->keyboard_mapping, action->controller_mapping};
    }
    
    file_data.sfx_volume    = *s_settings.sfx_volume;
    file_data.music_volume  = *s_settings.music_volume;
    file_data.is_muted      = *s_settings.is_muted;
    file_data.is_fullscreen = (Platform_Get_Flags() & Platform_Flags::fullscreen);
    
    // NOTE: Likely not a sensible way of handling errors.
    u32 attemps = 3;
    while(!Platform_Write_File(File_Data::file_name, (char*)&file_data, sizeof(file_data)) && attemps-- > 0);
    
    s_settings.dirty = false;
}


static bool load_settings_and_score()
{
    File_Data file_data = {0};
    
    bool result = false;
    
    u32 file_size;
    if(Platform_Get_File_Size(File_Data::file_name, &file_size))
    {
        if(file_size == sizeof(file_data))
        {
            if(Platform_Read_File(File_Data::file_name, (u8*)&file_data, file_size))
            {
                if(file_data.version == File_Data::file_format_version)
                {
                    result = true;
                    
                    for(u32 i = 0; i < s_max_highscore_count; ++i)
                    {
                        s_highscores[i] = file_data.highscores[i];
                    }
                    
                    
                    for(u32 i = 0; i < (u32)Game_Actions::COUNT; ++i)
                    {
                        s_game_actions[i].keyboard_mapping = file_data.game_action_data[i].keyboard_mapping;
                        s_game_actions[i].controller_mapping = file_data.game_action_data[i].controller_mapping;
                    }
                    
                    for(u32 i = 0; i < (u32)GUI_Menu_Actions::COUNT; ++i)
                    {
                        s_menu_actions[i].keyboard_mapping = file_data.menu_action_data[i].keyboard_mapping;
                        s_menu_actions[i].controller_mapping = file_data.menu_action_data[i].controller_mapping;
                    }
                    
                    for(u32 i = 0; i < (u32)Global_Actions::COUNT; ++i)
                    {
                        s_global_actions[i].keyboard_mapping = file_data.global_action_data[i].keyboard_mapping;
                        s_global_actions[i].controller_mapping = file_data.global_action_data[i].controller_mapping;
                    }
                    
                    *s_settings.sfx_volume = file_data.sfx_volume;
                    *s_settings.music_volume = file_data.music_volume;
                    *s_settings.is_muted = file_data.is_muted;
                    
                    Platform_Set_Fullscreen(file_data.is_fullscreen);
                }
            }
        }
    }
    
    return result;
}
