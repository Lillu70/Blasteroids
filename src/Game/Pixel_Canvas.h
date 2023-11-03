
#pragma once


struct Pixel_Canvas
{
	Pixel_Canvas() {};
	Pixel_Canvas(u32* pixels, v2u dimensions);
	
	void clear(u32 color);
	void draw_circle(v2i c, u32 radius, u32 thickness, u32 color);
	void draw_line(v2i p1, v2i p2, u32 color);
	void draw_line_with_wrapping(v2i p1, v2i p2, u32 color);
	void draw_border(u32 color);
	void draw_text(char* text, v2i p, u32 color, const u8* font, u32 char_width, u32 char_height);
	void draw_text(char* text, v2i p, u32 color, const u8* font, u32 char_width, u32 char_height, v2i scale);
	void draw_percentile_bar(v2i start, v2i end, f32 fill_amount, u32 fill_color, u32 bg_color, u32 border_color, u32 outline_thickness);
	void draw_mesh(v2f position, Mesh mesh, u32 color);
	void draw_rect(v2f offset, Rect rect, u32 color);
	void draw_filled_rect(v2f offset, Rect rect, u32 color);
	void draw_filled_rect_with_outline(Rect rect, u32 outline_thickness, u32 outline_color, u32 internal_color);
	
	inline void set_pixel(v2i coord, u32 color);
	inline void set_pixel2(v2i coord, u32 color);
	
	inline v2f get_middle() { return { (f32)m_dimensions.x * 0.5f, (f32)m_dimensions.y * 0.5f}; }
	
	u32* m_pixels = nullptr;
	v2u m_dimensions;
};
