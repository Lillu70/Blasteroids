
#pragma once

/*
Adding a gui widget check list

What to do in the .h file
{	
	- Add an entry in the GUI_Widget_Type enum.

	- Create a GUI_Widget struct
		It shold be composed of the 2 internal structs FIRST MUST COME the GUI_Widget_Header struct.
		Second should be widget spesific data inside "spec" struct, though it can be called anything,
		and is optional.
	
}

What to do in the .cpp file
{
	- Create an "gui_add_X" function, like for example the "gui_add_button" function.
	
	- If the widget has internal state, add an entry inte "gui_clear_widget_state" function.

	- If the widget needs special mouse behavior, 
    		add an entry in the "gui_handle_mouse_input" function.
	
	- Add an entry in the "gui_draw_widgets" function.

	- Add an entry in the select trigger switch in the "gui_handle_input".
	
	- Add an entty in the select hold trigger switch in the "gui_handle_input".

- Add an entry in the "gui_get_widget_size" function.
}
*/

enum class GUI_Menu_Actions
{
	back = 0,
	enter,
	up,
	down,
	left,
	right,
	mouse,
	COUNT
};


enum class GUI_Link_Direction : u32
{
	none = 0,
	down,
	up,
	skip
};


enum class GUI_Widget_Type : u32
{
	none = 0,
	button,
	slider,
	text,
	checkbox,
	key_listener,
};


struct GUI_Font
{
	u32 char_width = 0;
	u32 char_height = 0;
	u8* data_buffer = 0;
};


struct GUI_Theme
{
	u32 selected_color = put_color(250, 220, 115);
	u32 background_color = put_color(20, 20, 20);
	u32 down_color = put_color(50, 50, 50);
	u32 outline_thickness = 3;
	u32 outline_color = put_color(110, 110, 130);
	u32 text_color = WHITE;
	u32 title_color = put_color(210, 210, 230);
	
	GUI_Font font;
};


struct GUI_Widget_Header;
struct GUI_Additional_Highlights
{
	GUI_Widget_Header** highlights = 0;
	u32 capacity = 0;
	u32 count = 0;
};


struct GUI_Widget_Header
{
	GUI_Widget_Type type = GUI_Widget_Type::none;
	
	v2f position = 0;
	v2f dimensions = 0;
	
	GUI_Widget_Header* up_widget = 0;
	GUI_Widget_Header* down_widget = 0;
	GUI_Theme* theme = 0;
	GUI_Additional_Highlights* additional_highlights = 0;
};


struct GUI_Button_Spec
{
	u8* text = 0;
	i32 text_scale = 1;
	
	void(*on_click)() = 0;
};


struct GUI_Button
{
	// header must come first.
	
	GUI_Widget_Header header = GUI_Widget_Header(); 
	GUI_Button_Spec spec = GUI_Button_Spec();
	
	bool is_pressed = false;
};


struct GUI_Text_Spec
{
	u8* text = 0;
	i32 text_scale = 1;
	bool is_selectable = 0;
	bool is_title = 0;
};


struct GUI_Text
{
	// header must come first.
	
	GUI_Widget_Header header = GUI_Widget_Header();
	GUI_Text_Spec spec = GUI_Text_Spec();
};


struct GUI_Slider_Spec
{	
	f32 value = 0; // 0-1.f
	f32 step = 0.05f;
	f32 min = 0;
	f32 max = 1;
	
	void(*on_value_change)(GUI_Slider_Spec*);
};


struct GUI_Slider
{
	// header must come first.
	
	GUI_Widget_Header header = GUI_Widget_Header();
	GUI_Slider_Spec spec = GUI_Slider_Spec();
	
	bool is_clicked = false;
};


struct GUI_Checkbox_Spec
{
	bool is_checked = 0;
	void(*on_value_change)(GUI_Checkbox_Spec*);
};


struct GUI_Checkbox
{
	// header must come first.
	
	GUI_Widget_Header header = GUI_Widget_Header();
	GUI_Checkbox_Spec spec = GUI_Checkbox_Spec();
	
	bool is_pressed = 0;
};


struct GUI_Key_Listener_Spec
{
	Action* action_array = 0;   //ptr
	u32 action_idx = 0;         //offset
	void(*on_trigger)(Key_Code, Button, Action*, u32) = 0;
};


struct GUI_Key_Listener
{
	// header must come first.
	
	GUI_Widget_Header header = GUI_Widget_Header();
	GUI_Key_Listener_Spec spec = GUI_Key_Listener_Spec();
};


struct GUI_Frame
{
	u32 widget_count = 0;
	GUI_Widget_Header* selected_header = 0;
	Linear_Allocator widget_allocator = Linear_Allocator();
	
	void(*on_frame_close)() = 0;
	void(*on_back_action)() = 0;
	GUI_Frame* prev_frame = 0;
};


struct GUI_Handler
{
	GUI_Frame active_frame = GUI_Frame();
	
	f32 action_cd = 0.15f;
	f32 action_cd_time = 0;
	f32 hold_action_cd = 0.05f;
	f32 hold_delay = 0.3f;
	
	f32 action_start_time = 0;
	
	f32 default_padding = 10;
	v2i last_cursor_position = v2i(0,0);
	v2f last_element_pos = 0;
	v2f last_element_dim = 0;
	u32 addhighlight_mem_offset = 0;
	
	GUI_Theme* active_theme = 0;
	
	GUI_Widget_Header* first_header = 0;
	GUI_Widget_Header* last_header = 0;
	
	bool cursor_on_selection = false;
	
	static inline bool select_has_been_in_up_state = false;
};


static bool gui_menu_is_up(GUI_Handler* handler);


static void gui_push_frame(
	GUI_Handler* handler, 
	Platform_Call_Table* platform, 
	General_Allocator* mem_arena,
	u32 menu_memory_size,
	void(*on_back_action)(),
	void(*on_frame_close)() = 0);
    
    
static void gui_pop_frame(GUI_Handler* handler, Platform_Call_Table* platform, General_Allocator* mem_arena);


static inline Action* gui_get_action(Action* actions, GUI_Menu_Actions action);


static void gui_draw_widgets(GUI_Handler* handler, Pixel_Canvas* canvas);


static bool gui_handle_input(GUI_Handler* handler, Platform_Call_Table* platform, Action* actions);


static inline void gui_link_up_down(GUI_Widget_Header* up, GUI_Widget_Header* down);


static inline void gui_link_up_first_and_last(GUI_Handler* handler);


static inline void gui_link_down_first_and_last(GUI_Handler* handler);


static inline u32 gui_get_widget_size(GUI_Widget_Header* header); 


static inline void gui_select_first_element(GUI_Handler* handler);


static inline void gui_select_last_element(GUI_Handler* handler);


static inline u32 gui_get_offset_for_additional_highlights(u32 capacity);


static inline GUI_Additional_Highlights* gui_setup_additional_highlighting(
	GUI_Handler* handler, 
	GUI_Widget_Header* widget, 
	u32 capacity);


static inline void gui_add_additional_highlight(
	GUI_Additional_Highlights* addhigh, 
	GUI_Widget_Header* widget);


static void gui_create_header(
	GUI_Handler* handler, 
	GUI_Widget_Header* target, 
	v2f* position, 
	v2f* dimensions,
	GUI_Link_Direction ld);


static GUI_Widget_Header* gui_add_button(
	GUI_Handler* handler, 
	GUI_Button_Spec* spec, 
	v2f* position, 
	v2f* dimensions, 
	GUI_Link_Direction ld = GUI_Link_Direction::up);


static GUI_Widget_Header* gui_add_text(
	GUI_Handler* handler, 
	GUI_Text_Spec* spec, 
	v2f* position,
	GUI_Link_Direction ld = GUI_Link_Direction::up);

    
 static GUI_Widget_Header* gui_add_slider(
	GUI_Handler* handler, 
	GUI_Slider_Spec* spec,
	v2f* position, 
	v2f* dimensions, 
	GUI_Link_Direction ld= GUI_Link_Direction::up);


 static GUI_Widget_Header* gui_add_checkbox(
	GUI_Handler* handler, 
	GUI_Checkbox_Spec* spec,
	v2f* position, 
	v2f* dimensions, 
	GUI_Link_Direction ld= GUI_Link_Direction::up);
 
 
static GUI_Widget_Header* gui_add_key_listener(GUI_Handler* handler, GUI_Key_Listener_Spec* spec);
