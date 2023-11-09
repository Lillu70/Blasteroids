
#pragma once

#define AUTO_POS 0
#define AUTO_DIM 0
#define NO_BACK_ACTION 0

static void gui_create_settings_menu_from_main_menu();
static void gui_create_settings_menu_from_pause_menu();
static void gui_create_quit_menu();
static void gui_create_game_controls_menu();
static void gui_create_controls_menu();
static void gui_create_menu_controls_menu();
static void gui_create_global_controls_menu();
static void gui_create_leaderboard_menu();
static void gui_pop_frame_and_create_pause_menu();

// NOTE: sure what to do with thise yet, will have see when an actual system is made.
static void gui_set_fullscreen(GUI_Checkbox_Spec* spec)
{
	platform.set_flag(App_Flags::is_fullscreen, spec->is_checked);
	s_settings.dirty = true;
}


static void gui_set_is_muted(GUI_Checkbox_Spec* spec)
{
	s_settings.is_muted = spec->is_checked;
	s_settings.dirty = true;
}


static void set_sfx_volume(GUI_Slider_Spec* spec)
{
	s_settings.sfx_volume = spec->value;
	s_settings.dirty = true;
}


static void set_music_volume(GUI_Slider_Spec* spec)
{
	s_settings.music_volume = spec->value;
	s_settings.dirty = true;
}


static void gui_prev_menu()
{
	gui_pop_frame(&game.gui_handler, &platform, &mem);
}


static void gui_save_and_pop_menu()
{
	save_settings_and_score();
	gui_pop_frame(&game.gui_handler, &platform, &mem);
}


static void gui_rebind_action(Key_Code key, Button button, Action* action_array, u32 offset)
{
	gui_prev_menu();
	
	// We can assume that the triggering button is now selected?
	GUI_Widget_Header* selected_header = game.gui_handler.active_frame.selected_header;
	
	//NOTE: if things change, this likely will not be cought here at compile time. Ugly.
	//CONSIDER: Some better way of identifying gui elements?
	
	Assert(selected_header->type == GUI_Widget_Type::button);
	
	GUI_Additional_Highlights* addhigh = selected_header->additional_highlights;
	
	Assert(addhigh);
	Assert(addhigh->count == 2);
	
	if(key != Key_Code::NONE && action_array[offset].keyboard_mapping != key)
	{
		action_array[offset].keyboard_mapping = key;
		
		GUI_Text_Spec* text = &(((GUI_Text*)addhigh->highlights[0])->spec);
		text->text = (u8*)s_key_names[(u32)key];
		s_settings.dirty = true;
	}
	
	if(button != Button::NONE && action_array[offset].controller_mapping != button)
	{
		action_array[offset].controller_mapping = button;
		
		GUI_Text_Spec* text = &(((GUI_Text*)addhigh->highlights[1])->spec);
		text->text = (u8*)s_button_names[(u32)button];
		s_settings.dirty = true;
	}
	
	action_array[offset].invalid = true;
	action_array[offset].disabled = false;
}


static void gui_create_rebind_menu(Action* action_array, u32 action_idx)
{
	u32 menu_memory_size = 1 * sizeof(GUI_Key_Listener) + 1 * sizeof(GUI_Text);
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, NO_BACK_ACTION);
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"Press any key or controller button to rebind action.",
			.text_scale = 1,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, 170};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	GUI_Key_Listener_Spec listener = 
	{
		.action_array = action_array,
		.action_idx = action_idx,
		.on_trigger = gui_rebind_action
	};
	
	gui_add_key_listener(&game.gui_handler, &listener);
}

// Z prefix here just for my code editor to sort these out of sight in the function list.
static void zgui_rebind_game_shoot()
{
	gui_create_rebind_menu(&s_game_actions[0], (u32)Game_Actions::shoot);
}


static void zgui_rebind_game_accelerate()
{
	gui_create_rebind_menu(&s_game_actions[0], (u32)Game_Actions::accelerate);
}


static void zgui_rebind_game_turn_left()
{
	gui_create_rebind_menu(&s_game_actions[0], (u32)Game_Actions::turn_left);
}


static void zgui_rebind_game_turn_right()
{
	gui_create_rebind_menu(&s_game_actions[0], (u32)Game_Actions::turn_right);
}


static void zgui_rebind_game_pause()
{
	gui_create_rebind_menu(&s_game_actions[0], (u32)Game_Actions::pause);
}


static void zgui_rebind_menu_back()
{
	gui_create_rebind_menu(&s_menu_actions[0], (u32)GUI_Menu_Actions::back);
}


static void zgui_rebind_menu_select()
{
	gui_create_rebind_menu(&s_menu_actions[0], (u32)GUI_Menu_Actions::enter);
}


static void zgui_rebind_menu_up()
{
	gui_create_rebind_menu(&s_menu_actions[0], (u32)GUI_Menu_Actions::up);
}


static void zgui_rebind_menu_down()
{
	gui_create_rebind_menu(&s_menu_actions[0], (u32)GUI_Menu_Actions::down);
}


static void zgui_create_menu_left()
{
	gui_create_rebind_menu(&s_menu_actions[0], (u32)GUI_Menu_Actions::left);
}


static void zgui_rebind_menu_right()
{
	gui_create_rebind_menu(&s_menu_actions[0], (u32)GUI_Menu_Actions::right);
}


static void zgui_rebind_global_fullscreen()
{
	u32 idx = (u32)Global_Actions::toggle_fullscreen;
	
	s_global_actions[idx].disabled = true;
	gui_create_rebind_menu(&s_global_actions[0], idx);
}


static void zgui_rebind_global_quit()
{
	u32 idx = (u32)Global_Actions::quit_game;
	
	s_global_actions[idx].disabled = true;
	gui_create_rebind_menu(&s_global_actions[0], idx);
}


static void gui_create_main_menu()
{
	u32 menu_memory_size = 4 * sizeof(GUI_Button) + sizeof(GUI_Text);
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, NO_BACK_ACTION);
	
	i32 text_scale = 2;
	
	// Main Menu text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"MENU",
			.text_scale = 5,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, 70};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Start game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Start Game",
			.text_scale = text_scale,
			.on_click = set_mode_asteroids_sp
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// Leaderboard button.
	{
		GUI_Button_Spec button =
		{
			.text = (u8*)"Leaderboard",
			.text_scale = text_scale,
			.on_click = gui_create_leaderboard_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Settings menu button.
	{
		GUI_Button_Spec button =
		{
			.text = (u8*)"Settings",
			.text_scale = text_scale,
			.on_click = gui_create_settings_menu_from_main_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Quit game button.
	{
		GUI_Button_Spec button =
		{
			.text = (u8*)"Quit Game",
			.text_scale = text_scale,
			.on_click = quit_to_desktop
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_create_pause_menu()
{
	u32 menu_memory_size = 5 * sizeof(GUI_Button) + sizeof(GUI_Text);
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, unpause_game);
	
	pause_game();
	
	i32 text_scale = 2;
	
	// Main Menu text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"PAUSED",
			.text_scale = 5,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, 170};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Continue button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Continue",
			.text_scale = text_scale,
			.on_click = unpause_game,
		};
		
		v2f dimensions = v2f{ 200, 50 };
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// Restart button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Restart",
			.text_scale = text_scale,
			.on_click = restart_game
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Settings button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Settings",
			.text_scale = text_scale,
			.on_click = gui_create_settings_menu_from_pause_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Main menu.
	{
		GUI_Button_Spec button =
		{
			.text = (u8*)"Main Menu",
			.text_scale = text_scale,
			.on_click = set_mode_main_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Quit game 
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Quit Game",
			.text_scale = text_scale,
			.on_click = quit_to_desktop
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_create_quit_menu()
{
	pause_game();
	
	void(*on_frame_close)() = 0;
	void(*no_action)() = gui_pop_frame_and_create_pause_menu;
	if(game.gui_handler.active_frame.widget_allocator.memory)
	{
		no_action = gui_prev_menu;
		on_frame_close = unpause_game_without_destroy_frame;
	}
	
	u32 menu_memory_size = 2 * sizeof(GUI_Button) + sizeof(GUI_Text);
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, NO_BACK_ACTION, on_frame_close);  
	
	i32 text_scale = 2;
	
	// Quit Menu text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"QUIT TO DESKTOP?",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, 120};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// YES button.
	{
		GUI_Button_Spec button =
		{
			.text = (u8*)"YES",
			.text_scale = text_scale,
			.on_click = quit_to_desktop
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// NO button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"NO",
			.text_scale = text_scale,
			.on_click = no_action
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_pop_frame_and_create_pause_menu()
{
	gui_prev_menu();
	gui_create_pause_menu();
}


static void gui_create_settings_menu(u8* back_button_name, void(*create_function)(), void(*on_esc_fuck)(), f32 vertical_offset)
{
	u32 menu_memory_size = 
		2 * sizeof(GUI_Button) + 
		5 * sizeof(GUI_Text) + 
		2 * sizeof(GUI_Slider) +
		2 * sizeof(GUI_Checkbox);
	
	
	u32 sfx_slider_addhighlight_memory = gui_get_offset_for_additional_highlights(1); 
	u32 music_slider_addhighlight_memory = gui_get_offset_for_additional_highlights(1);
	u32 fullscreen_checkbox_addhighlight_memory = gui_get_offset_for_additional_highlights(1);
	u32 mute_checkbox_addhighlight_memory = gui_get_offset_for_additional_highlights(1);
	
	menu_memory_size += sfx_slider_addhighlight_memory;
	menu_memory_size += music_slider_addhighlight_memory;
	menu_memory_size += fullscreen_checkbox_addhighlight_memory;
	menu_memory_size += mute_checkbox_addhighlight_memory;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, gui_save_and_pop_menu);
	
	i32 text_scale = 2;
	
	// Settings text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"SETTINGS",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, vertical_offset};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Main Menu button.
	{
		GUI_Button_Spec button = 
		{
			.text = back_button_name,
			.text_scale = text_scale,
			.on_click = gui_save_and_pop_menu
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// Keybinds button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Controls",
			.text_scale = text_scale,
			.on_click = gui_create_controls_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// mute checkbox
	v2f mute_checkbox_position;
	v2f mute_text_position;
	GUI_Additional_Highlights* mute_checkbox_highlights;
	{
		GUI_Checkbox_Spec checkbox = 
		{
			.is_checked = s_settings.is_muted,
			.on_value_change = gui_set_is_muted
		};
		
		f32 side = f32(s_gui_theme.font.char_height * text_scale);
		
		v2f dimensions = v2f{ side, side };
		
		GUI_Widget_Header* widget = gui_add_checkbox(&game.gui_handler, &checkbox, AUTO_POS, &dimensions);
		mute_checkbox_position = widget->position;
		mute_text_position = mute_checkbox_position;
		mute_text_position.x +=  side / 2 + game.gui_handler.default_padding / 2;
		widget->position.x -= (100 - side / 2);
		
		mute_checkbox_highlights = gui_setup_additional_highlighting(&game.gui_handler, widget, 1);
	}
	
	// mute text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"Mute Sound",
			.text_scale = text_scale
		};
		
		GUI_Widget_Header* widget = gui_add_text(&game.gui_handler, &text, &mute_text_position, GUI_Link_Direction::skip);
		gui_add_additional_highlight(mute_checkbox_highlights, widget);
		
		game.gui_handler.last_element_pos = mute_checkbox_position;
	}
	
	GUI_Widget_Header* sfx_text_widget;
	// text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"SFX Volume",
			.text_scale = text_scale
		};
		
		sfx_text_widget = gui_add_text(&game.gui_handler, &text, AUTO_POS, GUI_Link_Direction::skip);
	}
	
	// Slider.
	{
		GUI_Slider_Spec slider = 
		{
			.value = s_settings.sfx_volume,
			.on_value_change = set_sfx_volume
		};
		
		v2f dimensions = v2f{ 200, 28 };
		
		GUI_Widget_Header* widget;
		widget = gui_add_slider(&game.gui_handler, &slider, AUTO_POS, &dimensions);
		
		GUI_Additional_Highlights* highlights;
		highlights = gui_setup_additional_highlighting(&game.gui_handler, widget, 1);
		gui_add_additional_highlight(highlights, sfx_text_widget);
	}
	
	GUI_Widget_Header* music_text_widget;
	// text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"Music Volume",
			.text_scale = text_scale
		};
		
		music_text_widget = gui_add_text(&game.gui_handler, &text, AUTO_POS, GUI_Link_Direction::skip);
	}
	
	// Slider.
	{
		GUI_Slider_Spec slider = 
		{
			.value = s_settings.music_volume,
			.on_value_change = set_music_volume
		};
		
		v2f dimensions = v2f{ 200, 28 };
		
		GUI_Widget_Header* widget;
		widget = gui_add_slider(&game.gui_handler, &slider, AUTO_POS, &dimensions);
		
		GUI_Additional_Highlights* highlights;
		highlights = gui_setup_additional_highlighting(&game.gui_handler, widget, 1);
		gui_add_additional_highlight(highlights, music_text_widget);
	}
	
	
	// checkbox
	v2f fullscreen_checkbox_position;
	GUI_Additional_Highlights* fullscreen_checkbox_highlights;
	{
		GUI_Checkbox_Spec checkbox = 
		{
			.is_checked = (platform.get_flags() & 1 << (u32)App_Flags::is_fullscreen) > 0,
			.on_value_change = gui_set_fullscreen
		};
		
		f32 side = f32(s_gui_theme.font.char_height * text_scale);
		
		v2f dimensions = v2f{ side, side };
		
		GUI_Widget_Header* widget = gui_add_checkbox(&game.gui_handler, &checkbox, AUTO_POS, &dimensions);
		fullscreen_checkbox_position = widget->position;
		fullscreen_checkbox_position.x += side / 2 + game.gui_handler.default_padding / 2;
		widget->position.x -= (100 - side / 2);
		
		fullscreen_checkbox_highlights = gui_setup_additional_highlighting(&game.gui_handler, widget, 1);
	}
	
	// text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"Fullscreen",
			.text_scale = text_scale
		};
		
		GUI_Widget_Header* widget = gui_add_text(&game.gui_handler, &text, &fullscreen_checkbox_position, GUI_Link_Direction::skip);
		gui_add_additional_highlight(fullscreen_checkbox_highlights, widget);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_create_settings_menu_from_main_menu()
{
	gui_create_settings_menu((u8*)"Main Menu", gui_create_settings_menu_from_main_menu, gui_create_main_menu, 160);
}


static void gui_create_settings_menu_from_pause_menu()
{
	gui_create_settings_menu((u8*)"Pause Menu", gui_create_settings_menu_from_pause_menu, gui_create_pause_menu, 190);
}


static void gui_create_highscore_menu()
{
	static constexpr u32 score_text_buffer_lenght = 11;
	
	u32 menu_memory_size = 3 * sizeof(GUI_Button) + 2 * sizeof(GUI_Text) + score_text_buffer_lenght;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, NO_BACK_ACTION);
	
	i32 text_scale = 2;
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"HIGHSCORE!",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, 120};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Score text.
	{
		u8* widget_allocator_memory = game.gui_handler.active_frame.widget_allocator.memory;
		u8* buffer = widget_allocator_memory + menu_memory_size - score_text_buffer_lenght; 
		u8* offset_buffer = u32_to_char_buffer(buffer, score_text_buffer_lenght, s_highscores[0]);
		
		GUI_Text_Spec text =
		{
			.text = offset_buffer,
			.text_scale = 5,
			.is_title = true
		};
		
		gui_add_text(&game.gui_handler, &text, AUTO_POS, GUI_Link_Direction::skip);
	}
	
	// New Game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Restart",
			.text_scale = text_scale,
			.on_click = restart_game
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// New Game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Main Menu",
			.text_scale = text_scale,
			.on_click = set_mode_main_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// New Game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Quit Game",
			.text_scale = text_scale,
			.on_click = quit_to_desktop
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_create_death_menu()
{
	static constexpr u32 score_text_buffer_lenght = 11;
	static constexpr u32 score_string_memory = 15;
	
	u32 menu_memory_size = 3 * sizeof(GUI_Button) + 2 * sizeof(GUI_Text) + score_text_buffer_lenght;
	menu_memory_size += score_string_memory;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, NO_BACK_ACTION);
	
	i32 text_scale = 2;
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"SCORE",
			.text_scale = 3,
			.is_title = true
		};
		
		if(s_last_earned_score.ranking > 0)
		{
			u8* widget_allocator_memory = game.gui_handler.active_frame.widget_allocator.memory;
			u8* buffer = widget_allocator_memory + menu_memory_size - score_text_buffer_lenght;
			buffer -= score_string_memory;
			u32 i = 0;
			if(s_last_earned_score.ranking == 10)
			{
				buffer[i++] = '1';
				buffer[i++] = '0';
				buffer[i++] = '.';
				buffer[i++] = ' ';
				buffer[i++] = 'B';
				buffer[i++] = 'E';
				buffer[i++] = 'S';
				buffer[i++] = 'T';
				buffer[i++] = ' ';
				buffer[i++] = 'S';
				buffer[i++] = 'C';
				buffer[i++] = 'O';
				buffer[i++] = 'R';
				buffer[i++] = 'E';
				buffer[i++] = 0;    
			}
			else
			{
				buffer[i++] = 48 + s_last_earned_score.ranking;
				buffer[i++] = '.';
				buffer[i++] = ' ';
				buffer[i++] = 'B';
				buffer[i++] = 'E';
				buffer[i++] = 'S';
				buffer[i++] = 'T';
				buffer[i++] = ' ';
				buffer[i++] = 'S';
				buffer[i++] = 'C';
				buffer[i++] = 'O';
				buffer[i++] = 'R';
				buffer[i++] = 'E';
				buffer[i++] = 0;    
			}
			
			
			text.text = buffer;
		}
		
		// TODO: str copy function :D
		
		v2f position = screen_canvas.get_middle() + v2f{0, 120};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Score text.
	{
		u8* widget_allocator_memory = game.gui_handler.active_frame.widget_allocator.memory;
		u8* buffer = widget_allocator_memory + menu_memory_size - score_text_buffer_lenght; 
		u8* offset_buffer = u32_to_char_buffer(buffer, score_text_buffer_lenght, s_last_earned_score.score);
		
		GUI_Text_Spec text =
		{
			.text = offset_buffer,
			.text_scale = 3,
			.is_title = true
		};
		
		gui_add_text(&game.gui_handler, &text, AUTO_POS, GUI_Link_Direction::skip);
	}
	
	// New Game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Restart",
			.text_scale = text_scale,
			.on_click = restart_game
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// New Game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Main Menu",
			.text_scale = text_scale,
			.on_click = set_mode_main_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// New Game button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Quit Game",
			.text_scale = text_scale,
			.on_click = quit_to_desktop
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


struct Add_Rebind_Row_Common_Data
{
	i32 text_scale = 0;
	f32 keyboard_text_offset = 0;
	f32 control_text_offset = 0;
	v2f dimensions = 0;
};


static void gui_add_rebind_row(
	Add_Rebind_Row_Common_Data* common_data, 
	u8* button_text, 
	void(*create_rebind_menu)(), 
	Action* action)
{
	GUI_Button_Spec button = 
	{
		.text = button_text,
		.text_scale = common_data->text_scale,
		.on_click = create_rebind_menu
	};
	
	GUI_Widget_Header* widget = gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	
	GUI_Additional_Highlights* highlights;
	highlights = gui_setup_additional_highlighting(&game.gui_handler, widget, 2);
	
	v2f button_pos = game.gui_handler.last_element_pos;
	
	{
		Key_Code key = action->keyboard_mapping;
		u8* key_name = (u8*)(key == Key_Code::NONE? "NONE" : s_key_names[(u32)key]);
		
		GUI_Text_Spec text = 
		{
			.text = key_name,
			.text_scale = common_data->text_scale
		};
		
		v2f text_pos = button_pos + v2f(common_data->keyboard_text_offset, 0);
		GUI_Widget_Header* text_1 = gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		gui_add_additional_highlight(highlights, text_1);            
	}
	
	{
		Button button = action->controller_mapping;
		u8* button_name = (u8*)(button == Button::NONE? "NONE" : s_button_names[(u32)button]);
		
		GUI_Text_Spec text = 
		{
			.text = button_name,
			.text_scale = common_data->text_scale
		};
		
		v2f text_pos = button_pos + v2f(common_data->control_text_offset, 0);
		
		GUI_Widget_Header* text_2 = gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		gui_add_additional_highlight(highlights, text_2);            
	}
	
	game.gui_handler.last_element_pos = button_pos;
	game.gui_handler.last_element_dim = common_data->dimensions;
}

static void gui_create_controls_menu()
{
	u32 menu_memory_size = 4 * sizeof(GUI_Button) + 1 * sizeof(GUI_Text);
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, gui_prev_menu);
	
	i32 text_scale = 2;
	
	// Title text.
	{
		GUI_Text_Spec text = 
		{
			.text = (u8*)"CONTROLS",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_canvas.get_middle() + v2f{0, 160};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Settings",
			.text_scale = text_scale,
			.on_click = gui_prev_menu
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
	}
	
	// Game controls button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Game",
			.text_scale = text_scale,
			.on_click = gui_create_game_controls_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Menu controls button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Menu",
			.text_scale = text_scale,
			.on_click = gui_create_menu_controls_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	// Global controls button.
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Global",
			.text_scale = text_scale,
			.on_click = gui_create_global_controls_menu
		};
		
		gui_add_button(&game.gui_handler, &button, AUTO_POS, AUTO_DIM);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}

static void gui_create_game_controls_menu()
{
	u32 menu_memory_size = 6 * sizeof(GUI_Button) + 13 * sizeof(GUI_Text);
	u32 add_highlight_mem_per_row = gui_get_offset_for_additional_highlights(2);
	
	menu_memory_size += add_highlight_mem_per_row * 5;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, gui_prev_menu);
	
	i32 text_scale = 2;
	
	v2f screen_middle = screen_canvas.get_middle();
	
	v2f dimensions = v2f{ 170, 40 };
	
	f32 keyboard_text_offset = 170;
	f32 control_text_offset = 340;
	
	
	Add_Rebind_Row_Common_Data common_data = 
	{
		.text_scale = text_scale,
		.keyboard_text_offset = keyboard_text_offset,
		.control_text_offset = control_text_offset,
		.dimensions = dimensions
	};
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"GAME CONTROLS",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_middle + v2f{0, 195};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Back to settings button.
	{
		f32 right_padd = game.gui_handler.default_padding * 3;
		
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Back",
			.text_scale = text_scale,
			.on_click = gui_prev_menu
		};
		
		GUI_Widget_Header* widget = gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
		widget->position.x -= keyboard_text_offset;
		
		game.gui_handler.last_element_pos = widget->position;
		v2f button_pos = game.gui_handler.last_element_pos;
		
		GUI_Text_Spec text = 
		{
			.text = (u8*)"Keyboard",
			.text_scale = text_scale
		};
		
		v2f text_pos = button_pos + v2f(keyboard_text_offset + right_padd, 0);
		gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		
		text_pos = button_pos + v2f(control_text_offset + right_padd, 0);
		text.text = (u8*)"Controller";
		gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		
		game.gui_handler.last_element_pos = button_pos + v2f(right_padd, 0);
		game.gui_handler.last_element_dim = dimensions;
	}
	
	// Rebind shoot.
	{
		void(*rebind_func)() = zgui_rebind_game_shoot;
		Action* action = &s_game_actions[(u32)Game_Actions::shoot];
		gui_add_rebind_row(&common_data, (u8*)"Shoot", rebind_func, action);
	}
	
	// Rebind accelerate.
	{
		void(*rebind_func)() = zgui_rebind_game_accelerate;
		Action* action = &s_game_actions[(u32)Game_Actions::accelerate];
		gui_add_rebind_row(&common_data, (u8*)"Accelerate", rebind_func, action);
	}
	
	// Rebind turn left.
	{
		void(*rebind_func)() = zgui_rebind_game_turn_left;
		Action* action = &s_game_actions[(u32)Game_Actions::turn_left];
		gui_add_rebind_row(&common_data, (u8*)"Turn Left", rebind_func, action);
	}
	
	// Rebind turn right.
	{
		void(*rebind_func)() = zgui_rebind_game_turn_right;
		Action* action = &s_game_actions[(u32)Game_Actions::turn_right];
		gui_add_rebind_row(&common_data, (u8*)"Turn Right", rebind_func, action);
	}
	
	// Rebind turn pause.
	{
		void(*rebind_func)() = zgui_rebind_game_pause;
		Action* action = &s_game_actions[(u32)Game_Actions::pause];
		gui_add_rebind_row(&common_data, (u8*)"Pause", rebind_func, action);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_create_menu_controls_menu()
{
	u32 menu_memory_size = 7 * sizeof(GUI_Button) + 15 * sizeof(GUI_Text);
	u32 add_highlight_mem_per_row = gui_get_offset_for_additional_highlights(2);
	
	menu_memory_size += add_highlight_mem_per_row * 6;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, gui_prev_menu);
	
	i32 text_scale = 2;
	
	v2f screen_middle = screen_canvas.get_middle();
	
	v2f dimensions = v2f{ 170, 40 };
	
	f32 keyboard_text_offset = 170;
	f32 control_text_offset = 340;
	
	Add_Rebind_Row_Common_Data common_data = 
	{
		.text_scale = text_scale,
		.keyboard_text_offset = keyboard_text_offset,
		.control_text_offset = control_text_offset,
		.dimensions = dimensions
	};
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"MENU CONTROLS",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_middle + v2f{0, 195};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Back to settings button.
	{
		f32 right_padd = game.gui_handler.default_padding * 3;
		
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Back",
			.text_scale = text_scale,
			.on_click = gui_prev_menu
		};
		
		GUI_Widget_Header* widget = gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
		widget->position.x -= keyboard_text_offset;
		
		game.gui_handler.last_element_pos = widget->position;
		v2f button_pos = game.gui_handler.last_element_pos;
		
		GUI_Text_Spec text = 
		{
			.text = (u8*)"Keyboard",
			.text_scale = text_scale
		};
		
		v2f text_pos = button_pos + v2f(keyboard_text_offset + right_padd, 0);
		gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		
		text_pos = button_pos + v2f(control_text_offset + right_padd, 0);
		text.text = (u8*)"Controller";
		gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		
		game.gui_handler.last_element_pos = button_pos + v2f(right_padd, 0);
		game.gui_handler.last_element_dim = dimensions;
	}
	
	// Rebind back.
	{
		void(*rebind_func)() = zgui_rebind_menu_back;
		Action* action = &s_menu_actions[(u32)GUI_Menu_Actions::back];
		gui_add_rebind_row(&common_data, (u8*)"Back", rebind_func, action);
	}
	
	// Rebind select.
	{
		void(*rebind_func)() = zgui_rebind_menu_select;
		Action* action = &s_menu_actions[(u32)GUI_Menu_Actions::enter];
		gui_add_rebind_row(&common_data, (u8*)"Select", rebind_func, action);
	}
	
	// Rebind up.
	{
		void(*rebind_func)() = zgui_rebind_menu_up;
		Action* action = &s_menu_actions[(u32)GUI_Menu_Actions::up];
		gui_add_rebind_row(&common_data, (u8*)"Up", rebind_func, action);
	}
	
	// Rebind down.
	{
		void(*rebind_func)() = zgui_rebind_menu_down;
		Action* action = &s_menu_actions[(u32)GUI_Menu_Actions::down];
		gui_add_rebind_row(&common_data, (u8*)"Down", rebind_func, action);
	}
	
	// Rebind left.
	{
		void(*rebind_func)() = zgui_create_menu_left;
		Action* action = &s_menu_actions[(u32)GUI_Menu_Actions::left];
		gui_add_rebind_row(&common_data, (u8*)"Left", rebind_func, action);
	}
	
	// Rebind right.
	{
		void(*rebind_func)() = zgui_rebind_menu_right;
		Action* action = &s_menu_actions[(u32)GUI_Menu_Actions::right];
		gui_add_rebind_row(&common_data, (u8*)"Right", rebind_func, action);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}


static void gui_create_global_controls_menu()
{
	u32 menu_memory_size = 3 * sizeof(GUI_Button) + 7 * sizeof(GUI_Text);
	u32 add_highlight_mem_per_row = gui_get_offset_for_additional_highlights(2);
	
	menu_memory_size += add_highlight_mem_per_row * 2;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, gui_prev_menu);
	
	i32 text_scale = 2;
	
	v2f screen_middle = screen_canvas.get_middle();
	
	v2f dimensions = v2f{ 170, 40 };
	
	f32 keyboard_text_offset = 170;
	f32 control_text_offset = 340;
	
	
	Add_Rebind_Row_Common_Data common_data = 
	{
		.text_scale = text_scale,
		.keyboard_text_offset = keyboard_text_offset,
		.control_text_offset = control_text_offset,
		.dimensions = dimensions
	};
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"GLOBAL CONTROLS",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f position = screen_middle + v2f{0, 195};
		
		gui_add_text(&game.gui_handler, &text, &position, GUI_Link_Direction::skip);
	}
	
	// Back to settings button.
	{
		f32 right_padd = game.gui_handler.default_padding * 3;
		
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Back",
			.text_scale = text_scale,
			.on_click = gui_prev_menu
		};
		
		GUI_Widget_Header* widget = gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions);
		widget->position.x -= keyboard_text_offset;
		
		game.gui_handler.last_element_pos = widget->position;
		v2f button_pos = game.gui_handler.last_element_pos;
		
		GUI_Text_Spec text = 
		{
			.text = (u8*)"Keyboard",
			.text_scale = text_scale
		};
		
		v2f text_pos = button_pos + v2f(keyboard_text_offset + right_padd, 0);
		gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		
		text_pos = button_pos + v2f(control_text_offset + right_padd, 0);
		text.text = (u8*)"Controller";
		gui_add_text(&game.gui_handler, &text, &text_pos, GUI_Link_Direction::skip);
		
		game.gui_handler.last_element_pos = button_pos + v2f(right_padd, 0);
		game.gui_handler.last_element_dim = dimensions;
	}
	
	// Rebind back.
	{
		void(*rebind_func)() = zgui_rebind_global_fullscreen;
		Action* action = &s_global_actions[(u32)Global_Actions::toggle_fullscreen];
		gui_add_rebind_row(&common_data, (u8*)"Fullscreen", rebind_func, action);
	}
	
	// Rebind select.
	{
		void(*rebind_func)() = zgui_rebind_global_quit;
		Action* action = &s_global_actions[(u32)Global_Actions::quit_game];
		gui_add_rebind_row(&common_data, (u8*)"Quit", rebind_func, action);
	}
	
	gui_link_up_first_and_last(&game.gui_handler);
}

static void gui_create_leaderboard_menu()
{
	//TODO: This way of doing this takes a ton of memory and is slower than needs be.
	// Fix with combining prefx and score into the same text element.
	
	static constexpr u32 score_text_buffer_lenght = 11;
	
	static const char* listin_prefixes[] = 
	{
		"1st",
		"2nd",
		"3rd",
		"4th",
		"5th",
		"6th",
		"7th",
		"8th",
		"9th",
		"10th"
	};
	
	u32 menu_memory_size = 1 * sizeof(GUI_Button) + 21 * sizeof(GUI_Text);
	menu_memory_size += score_text_buffer_lenght * s_max_highscore_count;
	
	gui_push_frame(&game.gui_handler, &platform, &mem, menu_memory_size, gui_prev_menu);
	
	i32 text_scale = 2;
	GUI_Link_Direction ld = GUI_Link_Direction::skip;
	
	f32 screen_middle_x;
	
	// Header text.
	{
		GUI_Text_Spec text =
		{
			.text = (u8*)"LEADERBOARD",
			.text_scale = 3,
			.is_title = true
		};
		
		v2f screen_middle = screen_canvas.get_middle();
		screen_middle_x = screen_middle.x;
		
		v2f position = screen_middle + v2f{0, 195};
		
		gui_add_text(&game.gui_handler, &text, &position, ld);
	}
	
	{
		GUI_Button_Spec button = 
		{
			.text = (u8*)"Main Menu",
			.text_scale = text_scale,
			.on_click = gui_prev_menu
		};
		
		v2f dimensions = v2f{ 200, 50 };
		
		GUI_Widget_Header* widget = gui_add_button(&game.gui_handler, &button, AUTO_POS, &dimensions, ld);
		game.gui_handler.active_frame.selected_header = widget;
	}
	
	i32 scale = 4;
	
	if(s_highscores[0] == 0)
	{
		GUI_Text_Spec prefix_text =
		{
			.text = (u8*)"You haven't scored anythin yet. Go play the game!",
			.text_scale = 1,
			.is_title = true
		};
		
		gui_add_text(&game.gui_handler, &prefix_text, AUTO_POS, ld);
	}
	else
	{
		for(u32 i = 0; i < s_max_highscore_count; ++i)
		{
			if(!s_highscores[i])
				break;
			
			i32 used_scale = scale < 1? 1 : scale;
			scale -= 1;
			
			u8* widget_allocator_memory = game.gui_handler.active_frame.widget_allocator.memory;
			u8* buffer = widget_allocator_memory + menu_memory_size - score_text_buffer_lenght * (i + 1);
			u8* offset_buffer = u32_to_char_buffer(buffer, score_text_buffer_lenght, s_highscores[i]);
			
			u32 char_width = game.gui_handler.active_theme->font.char_width;
			
			u32 score_text_lenght = null_terminated_buffer_lenght(offset_buffer)* char_width * used_scale;
			u32 prefix_text_lenght = null_terminated_buffer_lenght((u8*)listin_prefixes[i])* char_width * used_scale;
			u32 spacing = 1* char_width * used_scale;
			
			u32 total_text_lenght = score_text_lenght + prefix_text_lenght + spacing;
			
			GUI_Text_Spec prefix_text =
			{
				.text = (u8*)listin_prefixes[i],
				.text_scale = used_scale,
				.is_title = true
			};
			
			GUI_Widget_Header* widget;
			widget = gui_add_text(&game.gui_handler, &prefix_text, AUTO_POS, ld);
			
			v2f p = v2f(screen_middle_x - total_text_lenght / 2 + prefix_text_lenght / 2, widget->position.y);
			widget->position = p;
			
			if(i == 9)
				widget->position.x -= char_width / 2;
			
			GUI_Text_Spec text =
			{
				.text = offset_buffer,
				.text_scale = used_scale,
				.is_title = true
			};
			
			p = v2f(p.x + prefix_text_lenght / 2 + spacing + score_text_lenght / 2, widget->position.y);
			widget = gui_add_text(&game.gui_handler, &text, &p, GUI_Link_Direction::skip);
		}
	}
}
