
#pragma once


static void gui_push_frame(
	GUI_Handler* handler, 
	Platform_Call_Table* platform, 
	General_Allocator* mem_arena,
	u32 menu_memory_size,
	void(*on_back_action)(),
	void(*on_frame_close)())
{
	Assert(handler);
	Assert(platform);
	Assert(mem_arena);
	
	GUI_Frame* active_frame = &handler->active_frame;
	
	GUI_Frame* prev_frame = 0;
	
	// There is a menu, it needs to be pushed forwards in the chain.
	if(active_frame->widget_allocator.memory)
	{
		prev_frame = mem_arena->push<GUI_Frame>();
		*prev_frame = *active_frame;
	}
	
	// there is no previous menu, show cursor.
	else
	{
		platform->set_flag(App_Flags::cursor_is_visible, true);
	}
	
	active_frame->widget_count = 0;
	active_frame->selected_header = 0;
	active_frame->widget_allocator.init(mem_arena->push(menu_memory_size), menu_memory_size);
	active_frame->on_back_action = on_back_action;
	active_frame->on_frame_close = on_frame_close;
	active_frame->prev_frame = prev_frame;
	
	
	handler->last_element_pos = 0;
	handler->last_element_dim = 0;
	handler->first_header = 0;
	handler->last_header = 0;
	
	// NOTE: Scary stuff here, absolutely must remember to clear any memory related state here.
	handler->addhighlight_mem_offset = 0;
}

    
static void gui_pop_frame(GUI_Handler* handler, Platform_Call_Table* platform, General_Allocator* mem_arena)
{
	Assert(handler);
	GUI_Frame* active_frame = &handler->active_frame;
	
	Assert(active_frame->widget_allocator.memory);
	
	if(active_frame->on_frame_close)
		active_frame->on_frame_close();
	
	mem_arena->free(active_frame->widget_allocator.memory);
	
	if(active_frame->prev_frame)
	{    
		GUI_Frame* prev_frame = active_frame->prev_frame;
		*active_frame = *prev_frame;
		mem_arena->free(prev_frame);
	}
	
	// This is the only frame.
	else
	{
		*active_frame = GUI_Frame();
		platform->set_flag(App_Flags::cursor_is_visible, false);
	}
}


static inline u32 gui_get_widget_size(GUI_Widget_Header* header)
{
	Assert(header);
	
	switch(header->type)
	{
		case GUI_Widget_Type::button:
			return sizeof(GUI_Button);
		
		case GUI_Widget_Type::text:
			return sizeof(GUI_Text);
		
		case GUI_Widget_Type::slider:
			return sizeof(GUI_Slider);
		
		case GUI_Widget_Type::checkbox:
			return sizeof(GUI_Checkbox);
		
		case GUI_Widget_Type::key_listener:
			return sizeof(GUI_Key_Listener);
	}
	
	return Terminate;
}


static void gui_create_header(
	GUI_Handler* handler, 
	GUI_Widget_Header* target, 
	v2f* position, 
	v2f* dimensions,
	GUI_Link_Direction ld)
{
	Assert(handler);
	Assert(target);
	
	(*target) = {};
	
	GUI_Frame* active_frame = &handler->active_frame;
	
	active_frame->widget_count += 1;
	
	// Auto positioning.
	// NOTE: Dimension effects positioning, so it's handeled first.
	if(!dimensions)
		target->dimensions = handler->last_element_dim;
	
	else
		target->dimensions = *dimensions;
	
	
	if(!position)
	{
		f32 vertical_offset = 
		handler->last_element_dim.y / 2 + target->dimensions.y / 2 + handler->default_padding;
		
		if(ld != GUI_Link_Direction::down)
			vertical_offset *= -1;
		
		target->position = handler->last_element_pos + v2f(0, vertical_offset);
	}
	else
		target->position = *position;
	
	handler->last_element_pos = target->position;
	handler->last_element_dim = target->dimensions;
	
	target->theme = handler->active_theme;
	
	// Auto linking.
	if((b32)ld && handler->last_header)
	{
		GUI_Widget_Header* prev_header = handler->last_header;
		
		switch(ld)
		{
			case GUI_Link_Direction::up:
			{	
				handler->last_header->down_widget = target;
				target->up_widget = handler->last_header;
			}break;
			
			case GUI_Link_Direction::down:
			{
				handler->last_header->up_widget = target;
				target->down_widget = handler->last_header;
			}break;
			
			case GUI_Link_Direction::skip:
				return; // NOTE the return here. Point of the case is to skip out of the function.
		}
	}
	
	if(!handler->first_header && ld != GUI_Link_Direction::skip)
	{
		handler->first_header = target;
		if(!active_frame->selected_header)
			active_frame->selected_header = handler->first_header;
	}
	
	handler->last_header = target;
}


static GUI_Widget_Header* gui_add_button(
	GUI_Handler* handler, 
	GUI_Button_Spec* spec, 
	v2f* position, 
	v2f* dimensions, 
	GUI_Link_Direction ld)
{
	Assert(handler);
	Assert(spec);
	
	GUI_Button* widget = handler->active_frame.widget_allocator.push<GUI_Button>();
	GUI_Widget_Header* header = &widget->header;
	
	gui_create_header(handler, header, position, dimensions, ld);
	header->type = GUI_Widget_Type::button;
	
	widget->spec = *spec;
	
	widget->is_pressed = false;
	
	return header;
}


static GUI_Widget_Header* gui_add_text(
	GUI_Handler* handler, 
	GUI_Text_Spec* spec, 
	v2f* position,
	GUI_Link_Direction ld)
{
	Assert(handler);
	Assert(spec);
	Assert(spec->text);
	
	GUI_Text* widget = handler->active_frame.widget_allocator.push<GUI_Text>();
	
	// Text doens't have dimensions in the traditional sense, but it's bounding box is calculated here.
	// It's used for auto position of elements and mouse selection of text elements.
	
	GUI_Font* font = &handler->active_theme->font;
	
	u32 text_lenght = null_terminated_buffer_lenght(spec->text);
	v2f dimensions = v2f{(f32)(text_lenght * spec->text_scale * font->char_width), 
	(f32)(spec->text_scale * font->char_height)};
	
	GUI_Widget_Header* header = &widget->header;
	gui_create_header(handler, header, position, &dimensions, ld);
	
	header->type = GUI_Widget_Type::text;
	widget->spec = *spec;
	
	return header;
}


static GUI_Widget_Header* gui_add_slider(
	GUI_Handler* handler, 
	GUI_Slider_Spec* spec,
	v2f* position, 
	v2f* dimensions, 
	GUI_Link_Direction ld)
{
	Assert(handler);
	Assert(spec);
	
	GUI_Slider* widget = handler->active_frame.widget_allocator.push<GUI_Slider>();
	GUI_Widget_Header* header = &widget->header;
	
	gui_create_header(handler, header, position, dimensions, ld);
	header->type = GUI_Widget_Type::slider;
	widget->spec = *spec;
	
	widget->is_clicked = false;
	
	return header;
}


static GUI_Widget_Header* gui_add_checkbox(
	GUI_Handler* handler, 
	GUI_Checkbox_Spec* spec,
	v2f* position, 
	v2f* dimensions, 
	GUI_Link_Direction ld)
{
	Assert(handler);
	Assert(spec);
	
	GUI_Checkbox* widget = handler->active_frame.widget_allocator.push<GUI_Checkbox>();
	GUI_Widget_Header* header = &widget->header;
	
	gui_create_header(handler, header, position, dimensions, ld);
	header->type = GUI_Widget_Type::checkbox;
	widget->spec = *spec;
	
	widget->is_pressed = false;
	
	return header;
}


static GUI_Widget_Header* gui_add_key_listener(GUI_Handler* handler, GUI_Key_Listener_Spec* spec)
{
	Assert(handler);
	Assert(spec);
	
	GUI_Key_Listener* widget = handler->active_frame.widget_allocator.push<GUI_Key_Listener>();
	GUI_Widget_Header* header = &widget->header;
	
	gui_create_header(handler, header, 0, 0, GUI_Link_Direction::skip);
	header->type = GUI_Widget_Type::key_listener;
	widget->spec = *spec;
	
	// Auto select these.
	handler->active_frame.selected_header = header;
	
	return header;
}


static void gui_clear_widget_state(GUI_Widget_Header* header)
{
	switch(header->type)
	{
		case GUI_Widget_Type::button:
		{
			((GUI_Button*)header)->is_pressed = false;
		}break;
		
		case GUI_Widget_Type::slider:
		{
			((GUI_Slider*)header)->is_clicked = false;
		}break;
		
		case GUI_Widget_Type::checkbox:
		{
			((GUI_Checkbox*)header)->is_pressed = false;
		}break;
	}
}


static inline void gui_select_widget_with_mouse(GUI_Handler* handler, GUI_Widget_Header* header)
{
	if(header != handler->active_frame.selected_header)
	{
		gui_clear_widget_state(handler->active_frame.selected_header);
		handler->active_frame.selected_header = header;
	}
	
	handler->cursor_on_selection = true;
}


static void gui_handle_mouse_input(GUI_Handler* handler, Platform_Call_Table* platform, Action* actions)
{
	v2i cursor_position = platform->get_cursor_position();
	
	GUI_Frame* active_frame = &handler->active_frame;
	
	bool mouse_is_pressed = gui_get_action(actions, GUI_Menu_Actions::mouse)->is_pressed();
	bool mouse_down = gui_get_action(actions, GUI_Menu_Actions::mouse)->is_down();
	
	if(handler->last_cursor_position != cursor_position || mouse_is_pressed)
	{
		handler->cursor_on_selection = false;
		
		GUI_Widget_Header* header = (GUI_Widget_Header*)active_frame->widget_allocator.memory;
		
		for(u32 i = 0; i < active_frame->widget_count; ++i)
		{
			u32 widget_size = gui_get_widget_size(header);
			
			Rect element_rect = create_rect_center(header->position, header->dimensions);
			if(point_inside_rect(cursor_position.As<f32>(), element_rect))
			{
				// Select element with the cursor.
				
				switch(header->type)
				{
					case GUI_Widget_Type::text:
					{
						GUI_Text_Spec* spec = &((GUI_Text*)header)->spec;
						if(spec->is_selectable)
						gui_select_widget_with_mouse(handler, header);
					}break;
					
					case GUI_Widget_Type::slider:
					{
						gui_select_widget_with_mouse(handler, header);
						
						GUI_Slider* slider = (GUI_Slider*)header;
						
						bool slider_is_held = slider->is_clicked && mouse_down;
						
						if(mouse_is_pressed || slider_is_held)
						{
							slider->is_clicked = true;
							
							GUI_Slider_Spec* spec = &((GUI_Slider*)header)->spec;
							v2f half_dim = header->dimensions / 2;
							f32 rel_cursor_x = (cursor_position.As<f32>() - (header->position - half_dim)).x;
							f32 fill_percent = rel_cursor_x / header->dimensions.x;
							f32 fill = (spec->max - spec->min) * fill_percent + spec->min;
							i32 steps = round(fill / spec->step);
							
							spec->value = steps * spec->step;
							
							if(spec->on_value_change)
								spec->on_value_change(spec);
						}
						
					}break;
					
					default:
					{
						gui_select_widget_with_mouse(handler, header);
					}
				}
				
				break;
			}
			
			header = (GUI_Widget_Header*)((u8*)header+widget_size);
		}
		
		
		// This assumes that widget state is mouse behaviar related.
		if(!handler->cursor_on_selection && active_frame->selected_header)
		{
			// Ugly hack to be able continue draging sliders outside their bounds.
			if(active_frame->selected_header->type == GUI_Widget_Type::slider)
			{
				if(!mouse_down)
					((GUI_Slider*)active_frame->selected_header)->is_clicked = false;
			}
			else
				gui_clear_widget_state(active_frame->selected_header);
		}
	}	
	
	handler->last_cursor_position = cursor_position;
}


static bool gui_handle_input(GUI_Handler* handler, Platform_Call_Table* platform, Action* actions)
{
	Assert(handler);
	Assert(actions);
	
	GUI_Frame* active_frame = &handler->active_frame;
	
	// Note: This is here to fix starting the application by pressing enter.
	// Todo: Fix this in the platform layer instead of here.
	if(!platform->get_keyboard_key_down(Key_Code::ENTER))
		handler->select_has_been_in_up_state = true;
	
	if(!handler->select_has_been_in_up_state)
		return true;
	
	if(!(platform->get_flags() & (1 << (u32)App_Flags::is_focused)))
		return false;
	
	// Exit if no menu is up.
	if(active_frame->widget_allocator.memory == 0)
		return true;
	
	update_actions(platform, actions, (u32)GUI_Menu_Actions::COUNT);
	
	
	if(active_frame->on_back_action)
	{
		if(gui_get_action(actions, GUI_Menu_Actions::back)->is_pressed())
		{
			active_frame->on_back_action();
			return false;
		}
	}
	
	gui_handle_mouse_input(handler, platform, actions);
	
	GUI_Widget_Header* header = active_frame->selected_header;
	
	if(!header)
		return true;
	
	// Keyboard/Controller element selection.
	{  
		if(gui_get_action(actions, GUI_Menu_Actions::up)->is_pressed() && header->up_widget)
		{
			gui_clear_widget_state(header);
			handler->cursor_on_selection = false;
			active_frame->selected_header = header->up_widget;
			return false;
		}		
		
		if(gui_get_action(actions, GUI_Menu_Actions::down)->is_pressed() && header->down_widget)
		{
			gui_clear_widget_state(header);
			handler->cursor_on_selection = false;
			active_frame->selected_header = header->down_widget;
			return false;
		}  
	}
	
	// Mouse stuff just uses the mouse buttons.
	bool mouse_is_pressed = gui_get_action(actions, GUI_Menu_Actions::mouse)->is_pressed();
	bool mouse_is_released = gui_get_action(actions, GUI_Menu_Actions::mouse)->is_released();
	
	
	bool select_action_pressed = 
	gui_get_action(actions, GUI_Menu_Actions::enter)->is_pressed() ||
		( handler->cursor_on_selection && mouse_is_pressed );
	
	
	bool select_action_released = 
	gui_get_action(actions, GUI_Menu_Actions::enter)->is_released() ||
		( handler->cursor_on_selection && mouse_is_released );
	
	// Keyboard/Controller selection interaction,
	switch(header->type)
	{
		case GUI_Widget_Type::button:
		{
			GUI_Button* button = (GUI_Button*)header;
			
			if(select_action_released)
			{
				if(button->is_pressed)
				{
					button->is_pressed = false;
					if(button->spec.on_click)
						button->spec.on_click();			
				
				}
			}
			else if(select_action_pressed)
				button->is_pressed = true;
			
			
		}break;
		
		case GUI_Widget_Type::slider:
		{
			GUI_Slider* slider = (GUI_Slider*)header;
			
			bool left_press = gui_get_action(actions, GUI_Menu_Actions::left)->is_pressed();
			bool right_press = gui_get_action(actions, GUI_Menu_Actions::right)->is_pressed();
			if(left_press || right_press)
			{
				handler->action_start_time = platform->get_time_stamp();
			}
			
			f32 time = (f32)platform->get_time_stamp();
			if(time < handler->action_cd_time)
				break;
			
			
			if(gui_get_action(actions, GUI_Menu_Actions::left)->is_down())
			{   
				f32 cd = (time > handler->action_start_time + handler->hold_delay)? 
					handler->hold_action_cd : handler->action_cd;
				
				handler->action_cd_time = time + cd;
				
				handler->action_cd_time = time + cd;
				
				GUI_Slider_Spec* spec = &slider->spec;
				
				spec->value -= spec->step;
				if(spec->value < spec->min)
					spec->value = spec->min;
				
				if(spec->on_value_change)
					spec->on_value_change(spec);
			}
			
			if(gui_get_action(actions, GUI_Menu_Actions::right)->is_down())
			{
				f32 cd = (time > handler->action_start_time + handler->hold_delay)? 
					handler->hold_action_cd : handler->action_cd;
				
				handler->action_cd_time = time + cd;
				
				GUI_Slider_Spec* spec = &slider->spec;
				
				spec->value += spec->step;
				if(spec->value > spec->max)
					spec->value = spec->max;
				
				if(spec->on_value_change)
					spec->on_value_change(spec);
			}
		
		}break;
		
		case GUI_Widget_Type::checkbox:
		{
			GUI_Checkbox* checkbox = (GUI_Checkbox*)header;
			
			if(select_action_released)
			{
				if(checkbox->is_pressed)
				{
					checkbox->is_pressed = false;
					checkbox->spec.is_checked = !checkbox->spec.is_checked;
					if(checkbox->spec.on_value_change)
						checkbox->spec.on_value_change(&checkbox->spec);					
				}
			}
			else if(select_action_pressed)
				checkbox->is_pressed = true;
			
		}break;
		
		case GUI_Widget_Type::key_listener:
		{
			GUI_Key_Listener_Spec* listener = &(((GUI_Key_Listener*)header)->spec);
			
			if(listener->on_trigger)
			{
				Key_Code key = Key_Code::NONE;
				Button button = Button::NONE;
				
				for(u32 i = 0; i < (u32)Key_Code::COUNT; ++i)
				{
					Key_Code k = (Key_Code)i;
					
					if(platform->get_keyboard_key_down(k))
					{
						key = k;
						break;
					}
				}
				
				Controller_State controller = platform->get_controller_state(0);
				
				for(u32 i = 0; i < (u32)Button::BUTTON_COUNT; ++i)
				{
					Button b = (Button)i;
					
					if(controller.get_button_down(b))
					{
						button = b;
						break;
					}
				}
				
				if(key != Key_Code::NONE || button != Button::NONE)
					listener->on_trigger(key, button, listener->action_array,listener->action_idx);
			}
		
		}break;
	}
	
	return true;
}


static void gui_draw_widgets(GUI_Handler* handler, Pixel_Canvas* canvas)
{
	GUI_Frame* active_frame = &handler->active_frame;
	
	GUI_Widget_Header* header = (GUI_Widget_Header*)active_frame->widget_allocator.memory;
	
	GUI_Font* font = &handler->active_theme->font;
	
	for(u32 i = 0; i < active_frame->widget_count; ++i)
	{
		u32 widget_size = gui_get_widget_size(header);
		
		bool is_selected = header == active_frame->selected_header;
		
		// Additional highlight code.
		if(active_frame->selected_header)
		{
			GUI_Additional_Highlights* addhigh = active_frame->selected_header->additional_highlights;
			if(addhigh)
				for(u32 i = 0; i < addhigh->count; ++i)
					if(header == addhigh->highlights[i])
					{
						is_selected = true;
						break;
					}
		
		}
		
		GUI_Theme* theme = header->theme;
		
		switch(header->type)
		{
			case GUI_Widget_Type::button:
			{
				GUI_Button* widget = (GUI_Button*)header;
				GUI_Button_Spec* spec = &widget->spec;
				
				u32 outline_color = is_selected? theme->selected_color : theme->outline_color;
				if(widget->is_pressed) 
					outline_color = theme->down_color;
				
				Rect button_rect = create_rect_center(header->position, header->dimensions);
				
				canvas->draw_filled_rect_with_outline(button_rect, 
				theme->outline_thickness,
				outline_color,
				theme->background_color);
				
				if(spec->text)
				{
					u32 text_lenght = null_terminated_buffer_lenght(spec->text);
					v2i scale = { spec->text_scale, spec->text_scale };
					
					v2i text_p = header->position.As<i32>();
					text_p.x -= text_lenght * scale.x * font->char_width * 0.5f;
					text_p.y -= scale.y * font->char_height * 0.5f;
					
					canvas->draw_text(
						(char*)spec->text, 
						text_p, 
						outline_color, 
						(const u8*)font->data_buffer,
						font->char_width, 
						font->char_height, 
						scale);
				}
			}break;
			
			case GUI_Widget_Type::text:
			{
				GUI_Text* widget = (GUI_Text*)header;
				GUI_Text_Spec* spec = &widget->spec;
				
				u32 text_lenght = null_terminated_buffer_lenght(spec->text);
				v2i scale = { spec->text_scale, spec->text_scale };
				
				v2i text_p = header->position.As<i32>();
				text_p.x -= text_lenght * scale.x * font->char_width * 0.5f;
				text_p.y -= scale.y * font->char_height * 0.5f;
				
				u32 color;
				if(spec->is_title)
					color = theme->title_color;
				else
					color = is_selected? theme->selected_color : theme->outline_color;
				
				canvas->draw_text(
					(char*)spec->text, 
					text_p, 
					color, 
					(const u8*)font->data_buffer,
					font->char_width, 
					font->char_height, 
					scale);
			}break;
			
			
			case GUI_Widget_Type::slider:
			{
				GUI_Slider* widget = (GUI_Slider*)header;
				GUI_Slider_Spec* spec = &widget->spec;
				v2f half_dim = header->dimensions * 0.5f;
				
				v2i bar_start = (header->position - half_dim).As<i32>();
				v2i bar_end = (header->position + half_dim).As<i32>();
				
				f32 fill = (spec->value - spec->min) / (spec->max - spec->min);
				
				u32 fill_color = is_selected? theme->selected_color : theme->outline_color;
				
				canvas->draw_percentile_bar(
					bar_start, 
					bar_end, 
					fill, 
					fill_color, 
					theme->background_color, 
					fill_color,
					theme->outline_thickness);
			
			}break;
			
			case GUI_Widget_Type::checkbox:
			{
				GUI_Checkbox* widget = (GUI_Checkbox*)header;
				GUI_Checkbox_Spec* spec = &widget->spec;
				u32 outline_color = is_selected? theme->selected_color : theme->outline_color;
				
				if(widget->is_pressed) 
					outline_color = theme->down_color;
				
				Rect rect = create_rect_center(header->position, header->dimensions);
				
				canvas->draw_filled_rect_with_outline(
					rect, 
					theme->outline_thickness,
					outline_color,
					theme->background_color);
				
				if(spec->is_checked)
				{
					rect = create_rect_center(header->position, header->dimensions * 0.5f);
					canvas->draw_filled_rect(0, rect, outline_color);
				}
			
			}break;
		}
		
		Assert(widget_size);
		header = (GUI_Widget_Header*)((u8*)header+widget_size);
	}
}


static inline void gui_link_up_down(GUI_Widget_Header* up, GUI_Widget_Header* down)
{
	Assert(up);
	Assert(down);
	
	up->down_widget = down;
	down->up_widget = up;
}


static inline void gui_link_up_first_and_last(GUI_Handler* handler)
{
	Assert(handler);
	Assert(handler->first_header);
	Assert(handler->last_header);
	
	handler->first_header->up_widget = handler->last_header;
	handler->last_header->down_widget = handler->first_header;
}


static inline void gui_link_down_first_and_last(GUI_Handler* handler)
{
	Assert(handler);
	Assert(handler->first_header);
	Assert(handler->last_header);
	
	handler->first_header->down_widget = handler->last_header;
	handler->last_header->up_widget = handler->first_header;
}


static inline void gui_select_first_element(GUI_Handler* handler)
{
	Assert(handler);
	Assert(handler->first_header);
	
	handler->active_frame.selected_header = handler->first_header;
}



static inline void gui_select_last_element(GUI_Handler* handler)
{
	Assert(handler);
	Assert(handler->last_header);
	
	handler->active_frame.selected_header = handler->last_header;
}


static inline u32 gui_get_offset_for_additional_highlights(u32 capacity)
{
	return sizeof(GUI_Additional_Highlights) + sizeof(GUI_Widget_Header*) * capacity;
}


static inline GUI_Additional_Highlights* gui_setup_additional_highlighting(
	GUI_Handler* handler, 
	GUI_Widget_Header* widget, 
	u32 capacity)
{   
	Linear_Allocator* widget_allocator = &handler->active_frame.widget_allocator;
	u8* memory = widget_allocator->memory + widget_allocator->capacity;
	memory -= handler->addhighlight_mem_offset;
	
	u32 offset = gui_get_offset_for_additional_highlights(capacity);
	widget->additional_highlights = (GUI_Additional_Highlights*)(memory - offset);
	handler->addhighlight_mem_offset += offset;
	
	
	widget->additional_highlights->capacity = capacity;
	widget->additional_highlights->count = 0;
	
	//Note: Why even have a pointer for where the memory is if it's always just right after the struct?
	widget->additional_highlights->highlights = (GUI_Widget_Header**)(widget->additional_highlights + 1);
	
	return widget->additional_highlights;
}


static inline void gui_add_additional_highlight(
	GUI_Additional_Highlights* addhigh, 
	GUI_Widget_Header* widget)
{
	Assert(addhigh->count < addhigh->capacity);
	
	GUI_Widget_Header** p_array = addhigh->highlights;
	p_array[addhigh->count++] = widget;
}


static inline Action* gui_get_action(Action* actions, GUI_Menu_Actions action)
{
	return (actions + (u32)action);
}


static bool gui_menu_is_up(GUI_Handler* handler)
{
	return handler->active_frame.widget_allocator.memory;
}
