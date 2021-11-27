/* date = September 1st 2021 10:39 pm */

#ifndef DRONECONTROLLER_MATH_H
#define DRONECONTROLLER_MATH_H

#include "dc_types.h"


#ifndef max
#define max(a,b)  ((a) > (b) ? (a) : (b))
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif


typedef union v2f32 v2f32;
union v2f32
{
    struct
    {
        f32 x;
        f32 y;
    };
    f32 c[2];
};

typedef union v2s32 v2s32;
union v2s32
{
    struct
    {
        s32 x;
        s32 y;
    };
    s32 c[2];
};


typedef union v3f32 v3f32;
union v3f32
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };
    f32 c[3];
};

typedef union v4f32 v4f32;
union v4f32
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 c[4];
};

//- RECTANGLES 

typedef union rect_v2f32 rect_v2f32;
union rect_v2f32
{
    struct
    {
        v2f32 min;
        v2f32 max;
    };
    v2f32 e[2];
};

typedef union rect_v2s32 rect_v2s32;
union rect_v2s32
{
    struct
    {
        v2s32 min;
        v2s32 max;
    };
    v2s32 e[2];
};


//- MATRICES 

typedef union m2f32 m2f32;
union m2f32
{
    v2f32 r[2];
    f32   e[4];
    f32   x[2][2];
};

typedef union m3f32 m3f32;
union m3f32
{
    v3f32 r[3];
    f32   e[9];
    f32   x[3][3];
};

typedef union m4f32 m4f32;
union m4f32
{
    v4f32 r[4];
    f32   e[16];
    f32   x[4][4];
};

//~ OPERATIORS & FUNCTIONS

//- VECTOR 2D 

internaldef v2f32 v2f32Init(f32 x, f32 y)
{
    v2f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

internaldef f32 v2f32Inner(v2f32 A, v2f32 B)
{
    f32 Result = 0.0f;
    
    Result = (A.x * B.x +
              A.y * B.y);
    
    return Result;
}


internaldef v2f32 v2f32Sub(v2f32 *A, v2f32 *B)
{
    v2f32 Result = { 0 };
    
    Result = v2f32Init(A->x - B->x,
                       A->y - B->y);
    
    return Result;
}

internaldef v2f32 v2f32Add(v2f32 *A, v2f32 *B)
{
    v2f32 Result = { 0 };
    
    Result = v2f32Init(A->x + B->x,
                       A->y + B->y);
    
    return Result;
}

internaldef v2f32 v2f32AddConstant(v2f32 *A, v2f32 *B)
{
    v2f32 Result = { 0 };
    
    Result = v2f32Init(A->x + B->x,
                       A->y + B->y);
    
    return Result;
}

internaldef v2s32 v2s32Init(s32 x, s32 y)
{
    v2s32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    
    return Result;
}

//- VECTOR 3D 

internaldef v3f32 v3f32Init(f32 x, f32 y, f32 z)
{
    v3f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    
    return Result;
}

//- VECTOR 4D 
internaldef v4f32 v4f32Init(f32 x, f32 y, f32 z, f32 w)
{
    v4f32 Result = { 0 };
    
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    
    return Result;
}

//- RECTANGLE 2D 

internaldef rect_v2f32 rect_v2f32Init(rect_v2f32 *Rect, v2f32 *Dim, v2f32 *Pos)
{
    rect_v2f32 Result = { 0 };
    
    Result.min.x = Pos->x - (Dim->x / 2);
    Result.min.y = Pos->y - (Dim->y / 2);
    Result.max.x = Pos->x + (Dim->x / 2);
    Result.max.y = Pos->y + (Dim->y / 2);
    
    if(Rect) *Rect = Result;
    
    return Result;
}

internaldef b32 rect_v2f32IsInRect(rect_v2f32 *Bounds, v2f32 *Point)
{
    b32 Result = ((Point->x <= Bounds->max.x) &&
                  (Point->y <= Bounds->max.y) &&
                  (Point->x >  Bounds->min.x) &&
                  (Point->y >  Bounds->min.y));
    
    return Result;
}

internaldef void rect_v2f32AddTo(rect_v2f32 *Bounds, f32 Value)
{
    rect_v2f32 *Result = Bounds;
    
    Result->min.x = Bounds->min.x - Value;
    Result->min.y = Bounds->min.y - Value;
    Result->max.x = Bounds->max.x + Value;
    Result->max.y = Bounds->max.y + Value;
    
    return;
}

internaldef void rect_v2s32Init(rect_v2s32 *Rect, v2s32 *Dim, v2s32 *Pos)
{
    Rect->min.x = Pos->x - (Dim->x / 2);
    Rect->min.y = Pos->y - (Dim->y / 2);
    Rect->max.x = Pos->x + (Dim->x / 2);
    Rect->max.y = Pos->y + (Dim->y / 2);
    
    return;
}

internaldef b32 rect_v2s32IsInRect(rect_v2s32 *Bounds, v2s32 *Point)
{
    return ((Point->x <= Bounds->max.x) &&
            (Point->y <= Bounds->max.y) &&
            (Point->x >  Bounds->min.x) &&
            (Point->y >  Bounds->min.y));
}


//- MATRIX 3D 
internaldef m3f32 m3f32Multiply(m3f32 A, m3f32 B)
{
    m3f32 Result = { 0 };
    
    for(u32 ScanRow = 0; ScanRow < 3; ScanRow++)
    {
        for(u32 ScanCol = 0; ScanCol < 3; ScanCol++)
        {
            f32 *Entry = &Result.r[ScanRow].c[ScanCol];
            
            for(u32 ScanElement = 0; ScanElement < 3; ScanElement++)
            {
                *Entry += A.r[ScanRow].c[ScanElement] * B.r[ScanElement].c[ScanCol];
            }
        }
    }
    
    return Result;
}

internaldef m3f32
m3f32Identity(void)
{
    m3f32 Result = { 0 };
    
    Result.r[0] = v3f32Init(1.0f, 0.0f, 0.0f);
    Result.r[1] = v3f32Init(0.0f, 1.0f, 0.0f);
    Result.r[2] = v3f32Init(0.0f, 0.0f, 1.0f);
    
    return Result;
}

internaldef m3f32
m3f32Scale(f32 x, f32 y, f32 z)
{
    m3f32 Result = { 0 };
    
    Result.r[0] = v3f32Init(x   , 0.0f, 0.0f);
    Result.r[1] = v3f32Init(0.0f, y   , 0.0f);
    Result.r[2] = v3f32Init(0.0f, 0.0f, 1.0f);
    
    return Result;
}


internaldef m3f32
m3f32Rotation(f32 x, f32 y, f32 z)
{
    m3f32 Result = { 0 };
    
    m3f32 RotationX = m3f32Identity();
    if(x != 0.0f)
    {
        RotationX.r[0] = v3f32Init(1.0f,      0.0f,      0.0f);
        RotationX.r[1] = v3f32Init(0.0f, Cosine(x),  -Sine(x));
        RotationX.r[2] = v3f32Init(0.0f,   Sine(x), Cosine(x));
    }
    
    m3f32 RotationY = m3f32Identity();
    if(y != 0.0f)
    {
        RotationY.r[0] = v3f32Init(Cosine(y), 0.0f,   Sine(y));
        RotationY.r[1] = v3f32Init(     0.0f, 1.0f,      0.0f);
        RotationY.r[2] = v3f32Init( -Sine(y), 0.0f, Cosine(y));
    }
    
    
    m3f32 RotationZ = m3f32Identity();
    if(z != 0.0f)
    {
        RotationZ.r[0] = v3f32Init(Cosine(z),  -Sine(z), 0.0f);
        RotationZ.r[1] = v3f32Init(  Sine(z), Cosine(z), 0.0f);
        RotationZ.r[2] = v3f32Init(     0.0f,      0.0f, 1.0f);
    }
    
    Result = m3f32Multiply(RotationX, m3f32Multiply(RotationY, RotationZ));
    
    return Result;
}

internaldef m3f32
m3f32Translation(v3f32 PosDelta)
{
    m3f32 Result = { 0 };
#if 0
    Result.r[0] = v3f32Init(1.0f, 0.0f, PosDelta.x);
    Result.r[1] = v3f32Init(0.0f, 1.0f, PosDelta.y);
    Result.r[2] = v3f32Init(0.0f, 0.0f,       1.0f);
#else
    Result.r[0] = v3f32Init(      1.0f,       0.0f, 0.0f);
    Result.r[1] = v3f32Init(      0.0f,       1.0f, 0.0f);
    Result.r[3] = v3f32Init(PosDelta.x, PosDelta.y, 1.0f);
#endif
    return Result;
}

internaldef m3f32
m3f32Orthographic(f32 LeftPlane,
                  f32 RightPlane,
                  f32 BottomPlane,
                  f32 TopPlane)
{
    m3f32 Result = { 0 };
#if 0
    // NORMALIZING X
    Result.r[0].c[0] = 2.0f / (RightPlane - LeftPlane);
    
    // NORMALIZING Y
    Result.r[1].c[1] = 2.0f / (TopPlane - BottomPlane);
    
    // DISREGARDING Z
    Result.r[2].c[2] = 1.0f;
#else
    
    // NORMALIZING X
    Result.r[0].c[0] = 2.0f / (RightPlane - LeftPlane);
    Result.r[0].c[2] = -1.0f * ((RightPlane + LeftPlane) / (RightPlane - LeftPlane));
    
    // NORMALIZING Y
    Result.r[1].c[1] = 2.0f / (TopPlane - BottomPlane);
    Result.r[1].c[2] = -1.0f * ((TopPlane + BottomPlane) / (TopPlane - BottomPlane));
    
    // DISREGARDING Z
    Result.r[2].c[2] = 1.0f;
#endif
    
    return Result;
}

//- MATRIX 4D 
internaldef m4f32 m4f32Multiply(m4f32 A, m4f32 B)
{
    m4f32 Result = { 0 };
    
    for(u32 ScanRow = 0; ScanRow < 4; ScanRow++)
    {
        for(u32 ScanCol = 0; ScanCol < 4; ScanCol++)
        {
            f32 *Entry = &Result.r[ScanRow].c[ScanCol];
            
            for(u32 ScanElement = 0; ScanElement < 4; ScanElement++)
            {
                *Entry += A.r[ScanRow].c[ScanElement] * B.r[ScanElement].c[ScanCol];
            }
        }
    }
    
    return Result;
}

internaldef m4f32
m4f32Identity(void)
{
    m4f32 Result = { 0 };
    
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, 1.0f, 0.0f);
    Result.r[1] = v4f32Init(0.0f, 1.0f, 0.0f, 0.0f);
    Result.r[0] = v4f32Init(1.0f, 0.0f, 0.0f, 0.0f);
    
    return Result;
}

internaldef m4f32
m4f32Scale(f32 x, f32 y, f32 z)
{
    m4f32 Result = { 0 };
    
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, z   , 0.0f);
    Result.r[1] = v4f32Init(0.0f, y   , 0.0f, 0.0f);
    Result.r[0] = v4f32Init(x   , 0.0f, 0.0f, 0.0f);
    
    return Result;
}


internaldef m4f32
m4f32Rotation(f32 x, f32 y, f32 z)
{
    m4f32 Result = { 0 };
    
    m4f32 RotationX = m4f32Identity();
    if(x != 0.0f)
    {
        RotationX.r[0] = v4f32Init(1.0f,      0.0f,      0.0f, 0.0f);
        RotationX.r[1] = v4f32Init(0.0f, Cosine(x),  -Sine(x), 0.0f);
        RotationX.r[2] = v4f32Init(0.0f,   Sine(x), Cosine(x), 0.0f);
        RotationX.r[3] = v4f32Init(0.0f,      0.0f,      0.0f, 1.0f);
    }
    
    m4f32 RotationY = m4f32Identity();
    if(y != 0.0f)
    {
        RotationY.r[0] = v4f32Init(Cosine(y), 0.0f,   Sine(y), 0.0f);
        RotationY.r[1] = v4f32Init(     0.0f, 1.0f,      0.0f, 0.0f);
        RotationY.r[2] = v4f32Init( -Sine(y), 0.0f, Cosine(y), 0.0f);
        RotationY.r[3] = v4f32Init(     0.0f, 0.0f,      0.0f, 1.0f);
    }
    
    
    m4f32 RotationZ = m4f32Identity();
    if(z != 0.0f)
    {
        RotationZ.r[0] = v4f32Init(Cosine(z),  -Sine(z), 0.0f, 0.0f);
        RotationZ.r[1] = v4f32Init(  Sine(z), Cosine(z), 0.0f, 0.0f);
        RotationZ.r[2] = v4f32Init(     0.0f,      0.0f, 1.0f, 0.0f);
        RotationZ.r[3] = v4f32Init(     0.0f,      0.0f, 0.0f, 1.0f);
    }
    
    Result = m4f32Multiply(RotationX, m4f32Multiply(RotationY, RotationZ));
    
    return Result;
}

internaldef m4f32
m4f32Translate(v3f32 PosDelta)
{
    m4f32 Result = { 0 };
    
    Result.r[3] = v4f32Init(PosDelta.x, PosDelta.y, PosDelta.z, 1.0f);
    Result.r[2] = v4f32Init(      0.0f,       0.0f,       1.0f, 0.0f);
    Result.r[1] = v4f32Init(      0.0f,       1.0f,       0.0f, 0.0f);
    Result.r[0] = v4f32Init(      1.0f,       0.0f,       0.0f, 0.0f);
    
    return Result;
}

internaldef m4f32
m4f32Viewport(v2f32 WindowDim)
{
    m4f32 Result = { 0 };
    
    Result.r[0] = v4f32Init(WindowDim.x / 2.0f, 0.0f, 0.0f, (WindowDim.x - 1.0f) / 2.0f);
    Result.r[1] = v4f32Init(0.0f, WindowDim.y / 2.0f, 0.0f, (WindowDim.y - 1.0f) / 2.0f);
    Result.r[2] = v4f32Init(0.0f, 0.0f, 1.0f, 0.0f);
    Result.r[3] = v4f32Init(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Result;
}


internaldef m4f32
m4f32Orthographic(f32 LeftPlane,
                  f32 RightPlane,
                  f32 BottomPlane,
                  f32 TopPlane,
                  f32 NearPlane,
                  f32 FarPlane)
{
    m4f32 Result = { 0 };
    
    //v0: 0  1  2  3
    //v1: 4  5  6  7
    //v2: 8  9 10 11
    //v3:12 12 14 15
    
    
    Result.r[0].c[0] =  2.0f / (RightPlane - LeftPlane);
    Result.r[1].c[1] =  2.0f / (TopPlane - BottomPlane);
    Result.r[2].c[2] = -2.0f / (FarPlane -   NearPlane);
    
    Result.r[3].c[0] = -1.0f * ((RightPlane + LeftPlane  ) / (RightPlane -   LeftPlane));
    Result.r[3].c[1] = -1.0f * ((TopPlane   + BottomPlane) / (TopPlane   - BottomPlane));
    Result.r[3].c[2] = -1.0f * ((FarPlane   + NearPlane  ) / (FarPlane   -   NearPlane));
    Result.r[3].c[3] =  1.0f;
    
    return Result;
}

#endif //DRONECONTROLLER_MATH_H
