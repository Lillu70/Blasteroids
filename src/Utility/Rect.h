

// ===================================
// Copyright (c) 2024 by Valtteri Kois
// All rights reserved.
// ===================================

#pragma once

struct Rect
{
    v2f min = {};
    v2f max = {};
};


static inline void Add_Offset_To_Rect(v2f offset, Rect* rect)
{
	rect->min += offset;
	rect->max += offset;	
}


static inline bool Is_Rect_Valid(Rect rect)
{
    bool result = rect.min.x < rect.max.x && rect.min.y < rect.max.y;
    return result;
}


static inline Rect Create_Rect_Center(v2f pos, v2f dim)
{
    dim *= 0.5f;
    Rect result = {pos - dim, pos + dim};
    Assert(Is_Rect_Valid(result));
    
    return result;
}


static inline Rect Create_Rect_Min_Max(v2f min, v2f max)
{
    Rect result = { min, max };
    Assert(Is_Rect_Valid(result));
    
    return result;
}


static inline Rect Rect_Zero_One()
{
    Rect result = {{0.f, 0.f}, {1.f, 1.f}};
    return result;
}


static inline Rect Create_Rect_Min_Dim(v2f min, v2f dim)
{
    Rect result = { min, min + dim };
    Assert(Is_Rect_Valid(result));
    
    return result;
}


static inline Rect Create_Rect_Center_HZ(v2f pos, v2f dim)
{
    dim *= 0.5f;
    Rect result = {pos - dim, pos + dim};
    return result;
}


static inline Rect Create_Rect_Min_Max_HZ(v2f min, v2f max)
{
    Rect result = { min, max };
    return result;
}


static inline Rect Create_Rect_Min_Dim_HZ(v2f min, v2f dim)
{
    Rect result = { min, min + dim };
    return result;
}


static inline bool Is_Point_Inside_Rect(v2f p, Rect rect)
{
    bool result = p.x >= rect.min.x && p.y >= rect.min.y && p.x < rect.max.x && p.y < rect.max.y;
    return result;
}


static inline v2f Get_Rect_Dimensions(Rect rect)
{
    v2f result = rect.max - rect.min;
    
    return result;
}


static inline Rect Expand_Rect(Rect rect, f32 expand_factor)
{
    Rect result = Rect{rect.min - expand_factor, rect.max + expand_factor};
    Assert(Is_Rect_Valid(result));
    
    return result;
}


static inline Rect Expand_Rect(Rect rect, v2f expand_factor)
{
    Rect result = Rect{rect.min - expand_factor, rect.max + expand_factor};
    Assert(Is_Rect_Valid(result));
    
    return result;
}


static inline Rect Shrink_Rect(Rect rect, f32 shrink_factor)
{
    Rect result = Rect{rect.min + shrink_factor, rect.max - shrink_factor};
    Assert(Is_Rect_Valid(result));
    
    return result;
}


static inline bool Rects_Overlap(Rect a_rect, Rect b_rect)
{
    Assert(a_rect.min.x < a_rect.max.x);
    Assert(a_rect.min.y < a_rect.max.y);
    
    Assert(b_rect.min.x < b_rect.max.x);
    Assert(b_rect.min.y < b_rect.max.y);
    
    
    if(a_rect.max.x < b_rect.min.x)
        return false;
    
    if(a_rect.max.y < b_rect.min.y)
        return false;
    
    if(a_rect.min.x > b_rect.max.x)
        return false;
    
    if(a_rect.min.y > b_rect.max.y)
        return false;
    
    return true;
}
