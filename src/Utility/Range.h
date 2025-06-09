
#pragma once


struct Range
{
    f32 min;
    f32 max;
};


static inline bool Is_Valid(Range range)
{
    bool result = range.max > range.min;
    return result;
}


static inline f32 Span(Range range)
{
    Assert(Is_Valid(range));
    f32 result = range.max - range.min;
    return result;
}


static inline f32 Random_From_Range(Range range, Random_Machine* rm = &s_global_random_machine)
{
    Assert(Is_Valid(range));
     
    f32 result = Span(range) * rm->random_f32() + range.min;
    return result;
}