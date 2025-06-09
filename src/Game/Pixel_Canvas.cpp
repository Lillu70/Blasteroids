
#pragma once


Pixel_Canvas::Pixel_Canvas(u32* pixels, v2u dimensions) : m_pixels(pixels), m_dimensions(dimensions){}


void Pixel_Canvas::clear(u32 color)
{
    for (u32 i = 0; i < m_dimensions.x * m_dimensions.y; i++)
        m_pixels[i] = color;
}


void Pixel_Canvas::set_pixel(v2s p, u32 color)
{
    while(p.x > (s32)m_dimensions.x - 1)
        p.x -= m_dimensions.x;
    while(p.x < 0)
        p.x += m_dimensions.x;
    
    while(p.y > (s32)m_dimensions.y - 1)
        p.y -= m_dimensions.y;
    while(p.y < 0)
        p.y += m_dimensions.y;
    
    m_pixels[p.y * m_dimensions.x + p.x] = color;
}


void Pixel_Canvas::set_pixel2(v2s p, u32 color)
{
    if(p.x >= 0 && p.x < (s32)m_dimensions.x && p.y >= 0 && p.y < (s32)m_dimensions.y)
        m_pixels[p.y * m_dimensions.x + p.x] = color;
}


template<typename T>
inline f32 slope(v2<T> a, v2<T> b)
{                     
    return  (f32)(b.y - a.y) / (f32)(b.x - a.x);  
}

void Pixel_Canvas::draw_line_with_wrapping(v2s p1, v2s p2, u32 color)
{
    f64 angle = atan2((f64)p2.y - p1.y, (f64)p2.x - p1.x);
    if(angle < (PI / -4.0) || angle >= PI - (PI/4.0))
    {
        v2s temp_p = p1;
        p1 = p2;
        p2 = temp_p;
    }
    
    s32 xlen = abs((p2.x - p1.x));
    s32 ylen = abs((p2.y - p1.y));
    f32 m = slope(p1.As<f32>(), p2.As<f32>());
    
    if (xlen >= ylen)
    {
        for (s32 x = 0; x <= xlen; x++)
        {
            set_pixel(v2s{ p1.x + x, p1.y + (s32)(m * x) }, color);
        }
    }
    else
    {
        for (s32 y = 0; y <= ylen; y++)
        {
            set_pixel(v2s{ p1.x + (s32)(y / m), p1.y + y }, color);
        }
    }
}
    
    
void Pixel_Canvas::draw_line(v2s p1, v2s p2, u32 color)
{
    f64 angle = atan2((f64)p2.y - p1.y, (f64)p2.x - p1.x);
    if(angle < (PI / -4.0) || angle >= PI - (PI/4.0))
    {
        v2s temp_p = p1;
        p1 = p2;
        p2 = temp_p;
    }
    
    s32 xlen = abs((p2.x - p1.x));
    s32 ylen = abs((p2.y - p1.y));
    f32 m = slope(p1.As<f32>(), p2.As<f32>());
    
    if (xlen >= ylen)
        for (s32 x = 0; x <= xlen; ++x)
        {
            v2s p = { p1.x + x, p1.y + (s32)(m * x)};
            if(p.x < 0 || p.y < 0 || p.y >= (s32)m_dimensions.y || p.x >= (s32)m_dimensions.x)
                continue;
            
            m_pixels[p.y * m_dimensions.x + p.x] = color;
        }    
    else
        for (s32 y = 0; y <= ylen; ++y)
        {
            v2s p = {p1.x + (s32)(y / m), p1.y + y};
            
            if(p.x < 0 || p.y < 0 || p.x >= (s32)m_dimensions.x || p.y >= (s32)m_dimensions.y)
                continue;
            
            m_pixels[p.y * m_dimensions.x + p.x] = color;
        }

}


void Pixel_Canvas::draw_circle(v2s c, u32 radius, u32 thickness, u32 color)
{
    v2s start;
    start.x = c.x - radius;
    start.y = c.y - radius;
    
    
    v2s end;
    end.x = c.x + radius;
    end.y = c.y + radius;
    
    
    u32 half_thickness = thickness / 2;
    
    for(s32 y = start.y; y <= end.y; y++)
    {
        for(s32 x = start.x; x <= end.x; x++)
        {
            f32 d = Distance(c, { x, y });
            if(d >= (f32)(radius - half_thickness) && d <= (f32)(radius + half_thickness))
            {
                set_pixel2({x,y}, color);
            }
        }
    }
}


void Pixel_Canvas::draw_border(u32 color)
{
    for(u32 i = 0; i < m_dimensions.x; ++i)
        m_pixels[i] = color;
    
    u32 offset = (m_dimensions.y - 1) * m_dimensions.x;
    for(u32 i = 0; i < m_dimensions.x; ++i)
        m_pixels[offset + i] = color;
    
    for(u32 i = 1; i < m_dimensions.y - 1; ++i)
    {
        m_pixels[m_dimensions.x * i] = color;
        m_pixels[m_dimensions.x * (i + 1) - 1] = color;
    }
}


void Pixel_Canvas::draw_text(char* text, v2s p, u32 color, const u8* font, u32 char_width, u32 char_height)
{
    s32 y = p.y;
    s32 x = p.x;
    
    for(u32 i = 0; text[i]; ++i)
    {
        if (text[i] == '\n')
        {
            y += char_height;
            x = p.x;
            continue;
        }
        
        s32 character_idx = text[i] - 33;
        if (character_idx < 0 || character_idx > 93)
        {
            x += char_width;
            continue;
        }
        
        character_idx *= char_height;
        
        for (s32 row = 0; row < (s32)char_height; ++row)
        {
            for (s32 bit = 0; bit < (s32)char_width; ++bit)
            {
                if (font[character_idx + row] & (1 << bit))
                    set_pixel2({ x + bit, y + (s32)char_height - row}, color);
            }
        }
        x += char_width;
    }

}


void Pixel_Canvas::draw_text(char* text, v2s p, u32 color, const u8* font, u32 char_width, u32 char_height, v2s scale)
{
    s32 y = p.y;
    s32 x = p.x;
    
    for(u32 i = 0; text[i]; ++i)
    {
        if (text[i] == '\n')
        {
            y += char_height;
            x = p.x;
            continue;
        }
        
        s32 character_idx = text[i] - 33;
        if (character_idx < 0 || character_idx > 93)
        {
            x += char_width;
            continue;
        }
        
        character_idx *= char_height;
        
        for (s32 row = 0; row < (s32)char_height; ++row)
        {
            for (s32 bit = 0; bit < (s32)char_width; ++bit)
            {
                if (font[character_idx + row] & (1 << bit))
                {
                    s32 px = p.x+((x-p.x) * scale.x) + bit * scale.x;
                    s32 py = p.y+((y-p.y) * scale.y) + ((s32)char_height * scale.y) - (row * scale.y);    
                    for(s32 w = 0; w < scale.x; ++w)
                    {
                        for(s32 h = 0; h < scale.y; ++h)
                        {
                            set_pixel2({px + w, py + h}, color);
                        }
                    }
                
                }
            
            }
        }
        x += char_width;
    }
}


void Pixel_Canvas::draw_percentile_bar(
    v2s start,
    v2s end,
    f32 fill_amount,
    u32 fill_color,
    u32 bg_color,
    u32 border_color,
    u32 outline_thickness)
{
    v2s dim = end - start;
    
    s32 fill_width = (s32)(dim.x * fill_amount) + start.x;
    Rect rect = {start.As<f32>(), end.As<f32>()};
    
    s32 height = s32(rect.max.y - rect.min.y);
    s32 width = s32(rect.max.x - rect.min.x);
    
    for(s32 y = (s32)rect.min.y; y < (s32)rect.min.y + (s32)outline_thickness; ++y)
        for(s32 x = (s32)rect.min.x; x < (s32)rect.max.x; ++x)
        {
            set_pixel2(v2s{ x, y }, border_color);
            set_pixel2(v2s{ x, y + height - (s32)outline_thickness }, border_color);
        }
    
    for(s32 y = (s32)rect.min.y + (s32)outline_thickness; y < (s32)rect.max.y - (s32)outline_thickness; ++y)
        for(s32 x = (s32)rect.min.x; x < (s32)rect.min.x + (s32)outline_thickness; ++x)
        {
            set_pixel2(v2s{ x, y }, border_color);
            set_pixel2(v2s{ x + width - (s32)outline_thickness, y }, border_color);
        }
    
    for(s32 y = (s32)rect.min.y + (s32)outline_thickness; y < (s32)rect.max.y - (s32)outline_thickness; ++y)
        for(s32 x = (s32)rect.min.x + (s32)outline_thickness; x < (s32)rect.max.x - (s32)outline_thickness; ++x)
            set_pixel2({x,y}, (x <= fill_width)? fill_color : bg_color);

}


void Pixel_Canvas::draw_mesh(v2f position, Mesh mesh, u32 color)
{
    v2s p1 = (position + mesh.data[mesh.p_count - 1] + 0.5f).As<s32>();     
    
    for(u32 i = 0; i < mesh.p_count; ++i)
    {
        v2s p2 = (position + mesh.data[i] + 0.5f).As<s32>();
        draw_line(p1, p2, color);    
        p1 = p2;
    }
}


void Pixel_Canvas::draw_rect(v2f offset, Rect rect, u32 color)
{
    Assert(rect.min.x < rect.max.x);
    Assert(rect.min.y < rect.max.y);
    
    
    for(s32 x = (s32)rect.min.x; x < (s32)Round(rect.max.x); ++x)
    {
        set_pixel2(v2f(offset + v2f{ (f32)x, rect.min.y }).As<s32>(), color);
        set_pixel2(v2f(offset + v2f{ (f32)x, rect.max.y }).As<s32>(), color);
    }
    
    for(s32 y = (s32)rect.min.y; y < (s32)Round(rect.max.y); ++y)
    {
        set_pixel2(v2f(offset + v2f{rect.min.x, (f32)y }).As<s32>(), color);
        set_pixel2(v2f(offset + v2f{rect.max.x, (f32)y }).As<s32>(), color);
    }
}


void Pixel_Canvas::draw_filled_rect_with_outline(
    Rect rect, 
    u32 outline_thickness, 
    u32 outline_color, 
    u32 internal_color)
{
    // TODO: early out if drawring outside the sreen buffer.
    
    Assert(rect.min.x < rect.max.x);
    Assert(rect.min.y < rect.max.y);
    
    
    s32 height = s32(rect.max.y - rect.min.y);
    s32 width = s32(rect.max.x - rect.min.x);
    
    for(s32 y = (s32)rect.min.y; y < (s32)rect.min.y + (s32)outline_thickness; ++y)
        for(s32 x = (s32)rect.min.x; x < (s32)rect.max.x; ++x)
        {
            set_pixel2(v2s{ x, y }, outline_color);
            set_pixel2(v2s{ x, y + height - (s32)outline_thickness }, outline_color);
        }
    
    for(s32 y = (s32)rect.min.y + outline_thickness; y < (s32)rect.max.y - (s32)outline_thickness; ++y)
        for(s32 x = (s32)rect.min.x; x < (s32)rect.min.x + (s32)outline_thickness; ++x)
        {
            set_pixel2(v2s{ x, y }, outline_color);
            set_pixel2(v2s{ x + width - (s32)outline_thickness, y }, outline_color);
        }
    
    for(s32 y = (s32)rect.min.y + (s32)outline_thickness; y < (s32)rect.max.y - (s32)outline_thickness; ++y)
        for(s32 x = (s32)rect.min.x + (s32)outline_thickness; x < (s32)rect.max.x - (s32)outline_thickness; ++x)
            set_pixel2({x,y}, internal_color);
}



void Pixel_Canvas::draw_filled_rect(v2f offset, Rect rect, u32 color)
{
    Assert(rect.min.x < rect.max.x);
    Assert(rect.min.y < rect.max.y);
    
    //TODO: early out if drawring outside the sreen buffer. 
    
    for(s32 y = (s32)rect.min.y; y < (s32)Round(rect.max.y); ++y)
    {       
        for(s32 x = (s32)rect.min.x; x < (s32)Round(rect.max.x); ++x)
        {
            set_pixel2(v2f(offset + v2f{ (f32)x, (f32)y }).As<s32>(), color);
        }
    }
}
