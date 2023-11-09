

#pragma once


struct Action
{
	Key_Code keyboard_mapping = Key_Code::NONE;
	Button controller_mapping = Button::NONE;
	
	Button_State state = { 0, 0 };
	u64 last_poll_frame = 0;
	bool invalid = false;
	bool disabled = false;
	
	inline bool is_pressed()
	{
		return state.is_pressed();
	}
	
	inline bool is_released()
	{
		return state.is_released();
	}
	
	inline bool is_down()
	{
		return state.is_down();
	}
	
	inline bool is_up()
	{
		return state.is_up();
	}
};


static void update_actions(Platform_Call_Table* platform, Action* actions, u32 count)
{
	Assert(platform);
	Assert(actions);
	Assert(count);
	
	Controller_State controller = platform->get_controller_state(0);
	
	u64 frame = platform->get_frame_count();
	
	for(u32 i = 0; i < count; ++i)
	{
		Action* action = actions + i;
		
		if(action->last_poll_frame != frame - 1 || action->disabled)
		{
			action->invalid = true;
			action->state = Button_State(false, false);
		}
		else
		{
			bool s1 = (action->keyboard_mapping == Key_Code::NONE)? false : 
			platform->get_keyboard_key_down(action->keyboard_mapping);
			
			bool s2 = actions->controller_mapping == Button::NONE? false : 
			controller.get_button_state(action->controller_mapping).m_curr;
			
			if(action->invalid)
			{
				if(s1 == false && s2 == false)
				{
					action->invalid = false;
				}
			}
			else
			{
				action->state = Button_State(s1 || s2, action->state.m_curr); 
			}
		}
		
		
		action->last_poll_frame = frame;
	}
}
