/* date = September 1st 2021 10:39 pm */

#ifndef DRONECONTROLLER_MATH_H
#define DRONECONTROLLER_MATH_H


typedef union v2 v2;
union v2
{
    struct
    {
        f32 X;
        f32 Y;
    };
    f32 E[2];
};

typedef union v4 v4;
union v4
{
    struct
    {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };
    f32 E[4];
};

typedef union m4 m4;
union m4
{
    struct
    {
        v4 Col1;
        v4 Col2;
        v4 Col3;
        v4 Col4;
    };
    v4  V[4];
    f32 E[4];
};

typedef union rect_v2 rect_v2;
union rect_v2
{
    struct
    {
        v2 Min;
        v2 Max;
    };
    v2 V[2];
};

internal void rect_v2_Init(rect_v2 *Rect, v2 *Dim, v2 *Pos)
{
    Rect->Min.X = Pos->X - (Dim->X / 2);
    Rect->Min.Y = Pos->Y - (Dim->Y / 2);
    Rect->Max.X = Pos->X + (Dim->X / 2);
    Rect->Max.Y = Pos->Y + (Dim->Y / 2);
    
    return;
}

internal b32 rect_v2_IsInRect(rect_v2 *Bounds, v2 *Point)
{
    return ((Point->X <= Bounds->Max.X) &&
            (Point->Y <= Bounds->Max.Y) &&
            (Point->X >  Bounds->Min.X) &&
            (Point->Y >  Bounds->Min.Y));
}



#endif //DRONECONTROLLER_MATH_H
