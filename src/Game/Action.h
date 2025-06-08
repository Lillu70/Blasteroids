

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
		return state.Is_Pressed();
	}
	
	inline bool is_released()
	{
		return state.Is_Released();
	}
	
	inline bool is_down()
	{
		return state.Is_Down();
	}
	
	inline bool is_up()
	{
		return state.Is_Up();
	}
};


static void update_actions(Action* actions, u32 count)
{
	Assert(actions);
	Assert(count);	
	
	Controller_State controller = Platform_Get_Controller_State(0);
	
	u64 frame = Platform_Get_Frame_Count();
	
	for(u32 i = 0; i < count; ++i)
	{
		Action* action = actions + i;
		
		if(action->last_poll_frame != frame - 1 || action->disabled)
		{
			action->invalid = true;
			action->state = Button_State{false, false};
		}
		else
		{
			bool s1 = (action->keyboard_mapping == Key_Code::NONE)? false : 
			Platform_Get_Keyboard_Key_Down(action->keyboard_mapping);
			
			bool s2 = actions->controller_mapping == Button::NONE? false : 
			controller.Get_Button_State(action->controller_mapping).current;
			
			if(action->invalid)
			{
				if(s1 == false && s2 == false)
				{
					action->invalid = false;
				}
			}
			else
			{
				action->state = Button_State{s1 || s2, action->state.current};
			}
		}
		
		
		action->last_poll_frame = frame;
	}
}
