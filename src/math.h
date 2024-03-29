#ifndef MATH_H
#define MATH_H


static f32 Cosine(f32 Theta)
{
  f32 Result = cosf(Theta);
  return Result;
}

static f32 Sine(f32 Theta)
{
  f32 Result = sinf(Theta);
  return Result;
}
//- RECTANGLES 

typedef union r2f r2f;
union r2f
{
  struct
  {
    v2f min;
    v2f max;
  };
  v2f e[2];
};

typedef union r2s r2s;
union r2s
{
  struct
  {
    v2s pos;
    v2s dim;
  };
  struct
  {
    v2s min;
    v2s max;
  };
  v2s e[2];
};


//- MATRICES 


//~ OPERATIORS & FUNCTIONS

//- VECTOR 2D 

fn v2f v2fInit(f32 x, f32 y)
{
  v2f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  
  return Result;
}

fn f32 v2fInner(v2f A, v2f B)
{
  f32 Result = 0.0f;
  
  Result = (A.x * B.x +
            A.y * B.y);
  
  return Result;
}


fn v2f v2fSub(v2f A, v2f B)
{
  v2f Result = { 0 };
  
  Result = v2fInit(A.x - B.x,
                   A.y - B.y);
  
  return Result;
}

fn v2f v2fAdd(v2f A, v2f B)
{
  v2f Result = { 0 };
  
  Result = v2fInit(A.x + B.x,
                   A.y + B.y);
  
  return Result;
}

fn v2f v2fAddxy(v2f A, f32 x, f32 y)
{
  v2f Result = { 0 };
  
  Result = v2fInit(A.x + x,
                   A.y + y);
  
  return Result;
}


fn v2f v2fAddConstant(v2f *A, v2f *B)
{
  v2f Result = { 0 };
  
  Result = v2fInit(A->x + B->x,
                   A->y + B->y);
  
  return Result;
}

fn v2s v2s32Init(s32 x, s32 y)
{
  v2s Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  
  return Result;
}

//- VECTOR 3D 

fn v3f v3fInit(f32 x, f32 y, f32 z)
{
  v3f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  
  return Result;
}

//- VECTOR 4D 
fn v4f v4fInit(f32 x, f32 y, f32 z, f32 w)
{
  v4f Result = { 0 };
  
  Result.x = x;
  Result.y = y;
  Result.z = z;
  Result.w = w;
  
  return Result;
}

//- RECTANGLE 2D 

fn r2f r2fInit(r2f *Rect, v2f *Dim, v2f *Pos)
{
  r2f Result = { 0 };
  
  Result.min.x = Pos->x - (Dim->x / 2);
  Result.min.y = Pos->y - (Dim->y / 2);
  Result.max.x = Pos->x + (Dim->x / 2);
  Result.max.y = Pos->y + (Dim->y / 2);
  
  if(Rect) *Rect = Result;
  
  return Result;
}

fn r2f r2fInitFromPosRadius(r2f *Rect, v2f *Pos, f32 Radius)
{
  r2f Result = { 0 };
  
  Result.min.x = Pos->x - Radius;
  Result.min.y = Pos->y - Radius;
  Result.max.x = Pos->x + Radius;
  Result.max.y = Pos->y + Radius;
  
  if(Rect) *Rect = Result;
  
  return Result;
}

fn b32 r2fIsInRect(r2f *Bounds, v2f *Point)
{
  b32 Result = ((Point->x <= Bounds->max.x) &&
                (Point->y <= Bounds->max.y) &&
                (Point->x >  Bounds->min.x) &&
                (Point->y >  Bounds->min.y));
  
  return Result;
}

fn r2f r2fDimPosFromMinMax(r2f *Rect, v2f *Dim, v2f *Pos)
{
  r2f Result = { 0 };
  
  *Dim = v2fInit(Rect->max.x - Rect->min.x, Rect->max.y - Rect->min.y);
  *Pos = v2fInit(Rect->min.x + (Dim->x * 0.5f),
                 Rect->min.y + (Dim->y * 0.5f));
  
  return Result;
}

fn void r2fAddTo(r2f *Bounds, f32 Value)
{
  r2f *Result = Bounds;
  
  Result->min.x = Bounds->min.x - Value;
  Result->min.y = Bounds->min.y - Value;
  Result->max.x = Bounds->max.x + Value;
  Result->max.y = Bounds->max.y + Value;
  
  return;
}

fn void r2sInit(r2s *Rect, v2s *Dim, v2s *Pos)
{
  Rect->min.x = Pos->x - (Dim->x / 2);
  Rect->min.y = Pos->y - (Dim->y / 2);
  Rect->max.x = Pos->x + (Dim->x / 2);
  Rect->max.y = Pos->y + (Dim->y / 2);
  
  return;
}

fn b32 r2s32IsInRect(r2s *Bounds, v2s *Point)
{
  return ((Point->x <= Bounds->max.x) &&
          (Point->y <= Bounds->max.y) &&
          (Point->x >  Bounds->min.x) &&
          (Point->y >  Bounds->min.y));
}


//- MATRIX 3D 
fn m3f m3fMultiply(m3f A, m3f B)
{
  m3f Result = { 0 };
  
  for(u32 ScanRow = 0; ScanRow < 3; ScanRow++)
  {
    for(u32 ScanCol = 0; ScanCol < 3; ScanCol++)
    {
      f32 *Entry = &Result.r[ScanRow].e[ScanCol];
      
      for(u32 ScanElement = 0; ScanElement < 3; ScanElement++)
      {
        *Entry += A.r[ScanRow].e[ScanElement] * B.r[ScanElement].e[ScanCol];
      }
    }
  }
  
  return Result;
}

fn m3f
m3fIdentity(void)
{
  m3f Result = { 0 };
  
  Result.r[0] = v3fInit(1.0f, 0.0f, 0.0f);
  Result.r[1] = v3fInit(0.0f, 1.0f, 0.0f);
  Result.r[2] = v3fInit(0.0f, 0.0f, 1.0f);
  
  return Result;
}

fn m3f
m3fScale(f32 x, f32 y, f32 z)
{
  m3f Result = { 0 };
  
  Result.r[0] = v3fInit(x   , 0.0f, 0.0f);
  Result.r[1] = v3fInit(0.0f, y   , 0.0f);
  Result.r[2] = v3fInit(0.0f, 0.0f, 1.0f);
  
  return Result;
}


fn m3f
m3fRotate(f32 x, f32 y, f32 z)
{
  m3f Result = { 0 };
  
  m3f RotationX = m3fIdentity();
  if(x != 0.0f)
  {
    RotationX.r[0] = v3fInit(1.0f,      0.0f,      0.0f);
    RotationX.r[1] = v3fInit(0.0f, Cosine(x),  -Sine(x));
    RotationX.r[2] = v3fInit(0.0f,   Sine(x), Cosine(x));
  }
  
  m3f RotationY = m3fIdentity();
  if(y != 0.0f)
  {
    RotationY.r[0] = v3fInit(Cosine(y), 0.0f,   Sine(y));
    RotationY.r[1] = v3fInit(     0.0f, 1.0f,      0.0f);
    RotationY.r[2] = v3fInit( -Sine(y), 0.0f, Cosine(y));
  }
  
  
  m3f RotationZ = m3fIdentity();
  if(z != 0.0f)
  {
    RotationZ.r[0] = v3fInit(Cosine(z),  -Sine(z), 0.0f);
    RotationZ.r[1] = v3fInit(  Sine(z), Cosine(z), 0.0f);
    RotationZ.r[2] = v3fInit(     0.0f,      0.0f, 1.0f);
  }
  
  Result = m3fMultiply(RotationX, m3fMultiply(RotationY, RotationZ));
  
  return Result;
}

fn m3f m3fTranslate(v3f PosDelta)
{
  m3f Result = { 0 };
#if 0
  Result.r[0] = v3fInit(1.0f, 0.0f, PosDelta.x);
  Result.r[1] = v3fInit(0.0f, 1.0f, PosDelta.y);
  Result.r[2] = v3fInit(0.0f, 0.0f,       1.0f);
#else
  Result.r[0] = v3fInit(      1.0f,       0.0f, 0.0f);
  Result.r[1] = v3fInit(      0.0f,       1.0f, 0.0f);
  Result.r[3] = v3fInit(PosDelta.x, PosDelta.y, 1.0f);
#endif
  return Result;
}

fn m3f
m3fOrtho(f32 LeftPlane,
         f32 RightPlane,
         f32 BottomPlane,
         f32 TopPlane)
{
  m3f Result = { 0 };
#if 0
  // NORMALIZING X
  Result.r[0].e[0] = 2.0f / (RightPlane - LeftPlane);
  
  // NORMALIZING Y
  Result.r[1].e[1] = 2.0f / (TopPlane - BottomPlane);
  
  // DISREGARDING Z
  Result.r[2].e[2] = 1.0f;
#else
  
  // NORMALIZING X
  Result.r[0].e[0] = 2.0f / (RightPlane - LeftPlane);
  Result.r[0].e[2] = -1.0f * ((RightPlane + LeftPlane) / (RightPlane - LeftPlane));
  
  // NORMALIZING Y
  Result.r[1].e[1] = 2.0f / (TopPlane - BottomPlane);
  Result.r[1].e[2] = -1.0f * ((TopPlane + BottomPlane) / (TopPlane - BottomPlane));
  
  // DISREGARDING Z
  Result.r[2].e[2] = 1.0f;
#endif
  
  return Result;
}

//- MATRIX 4D 
fn m4f m4fMultiply(m4f A, m4f B)
{
  m4f Result = { 0 };
  
  for(u32 ScanRow = 0; ScanRow < 4; ScanRow++)
  {
    for(u32 ScanCol = 0; ScanCol < 4; ScanCol++)
    {
      f32 *Entry = &Result.r[ScanRow].e[ScanCol];
      
      for(u32 ScanElement = 0; ScanElement < 4; ScanElement++)
      {
        *Entry += A.r[ScanRow].e[ScanElement] * B.r[ScanElement].e[ScanCol];
      }
    }
  }
  
  return Result;
}

fn m4f
m4fIdentity(void)
{
  m4f Result = { 0 };
  
  Result.r[3] = v4fInit(0.0f, 0.0f, 0.0f, 1.0f);
  Result.r[2] = v4fInit(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[1] = v4fInit(0.0f, 1.0f, 0.0f, 0.0f);
  Result.r[0] = v4fInit(1.0f, 0.0f, 0.0f, 0.0f);
  
  return Result;
}

fn m4f
m4fScale(f32 x, f32 y, f32 z)
{
  m4f Result = { 0 };
  
  Result.r[3] = v4fInit(0.0f, 0.0f, 0.0f, 1.0f);
  Result.r[2] = v4fInit(0.0f, 0.0f, z   , 0.0f);
  Result.r[1] = v4fInit(0.0f, y   , 0.0f, 0.0f);
  Result.r[0] = v4fInit(x   , 0.0f, 0.0f, 0.0f);
  
  return Result;
}


fn m4f
m4fRotate(f32 x, f32 y, f32 z)
{
  m4f Result = { 0 };
  
  m4f RotationX = m4fIdentity();
  if(x != 0.0f)
  {
    RotationX.r[0] = v4fInit(1.0f,      0.0f,      0.0f, 0.0f);
    RotationX.r[1] = v4fInit(0.0f, Cosine(x),  -Sine(x), 0.0f);
    RotationX.r[2] = v4fInit(0.0f,   Sine(x), Cosine(x), 0.0f);
    RotationX.r[3] = v4fInit(0.0f,      0.0f,      0.0f, 1.0f);
  }
  
  m4f RotationY = m4fIdentity();
  if(y != 0.0f)
  {
    RotationY.r[0] = v4fInit(Cosine(y), 0.0f,   Sine(y), 0.0f);
    RotationY.r[1] = v4fInit(     0.0f, 1.0f,      0.0f, 0.0f);
    RotationY.r[2] = v4fInit( -Sine(y), 0.0f, Cosine(y), 0.0f);
    RotationY.r[3] = v4fInit(     0.0f, 0.0f,      0.0f, 1.0f);
  }
  
  
  m4f RotationZ = m4fIdentity();
  if(z != 0.0f)
  {
    RotationZ.r[0] = v4fInit(Cosine(z),  -Sine(z), 0.0f, 0.0f);
    RotationZ.r[1] = v4fInit(  Sine(z), Cosine(z), 0.0f, 0.0f);
    RotationZ.r[2] = v4fInit(     0.0f,      0.0f, 1.0f, 0.0f);
    RotationZ.r[3] = v4fInit(     0.0f,      0.0f, 0.0f, 1.0f);
  }
  
  Result = m4fMultiply(RotationX, m4fMultiply(RotationY, RotationZ));
  
  return Result;
}

fn m4f
m4fTranslate(v3f PosDelta)
{
  m4f Result = { 0 };
  
  Result.r[3] = v4fInit(PosDelta.x, PosDelta.y, PosDelta.z, 1.0f);
  Result.r[2] = v4fInit(      0.0f,       0.0f,       1.0f, 0.0f);
  Result.r[1] = v4fInit(      0.0f,       1.0f,       0.0f, 0.0f);
  Result.r[0] = v4fInit(      1.0f,       0.0f,       0.0f, 0.0f);
  
  return Result;
}
fn m4f
m4fViewport(v2f WindowDim)
{
  m4f Result = { 0 };
  
  Result.r[0] = v4fInit(WindowDim.x / 2.0f, 0.0f, 0.0f, (WindowDim.x - 1.0f) / 2.0f);
  Result.r[1] = v4fInit(0.0f, WindowDim.y / 2.0f, 0.0f, (WindowDim.y - 1.0f) / 2.0f);
  Result.r[2] = v4fInit(0.0f, 0.0f, 1.0f, 0.0f);
  Result.r[3] = v4fInit(0.0f, 0.0f, 0.0f, 1.0f);
  
  return Result;
}
fn m4f
m4fOrtho(f32 LeftPlane,
         f32 RightPlane,
         f32 BottomPlane,
         f32 TopPlane,
         f32 NearPlane,
         f32 FarPlane)
{
  m4f Result = { 0 };
  
  //v0: 0  1  2  3
  //v1: 4  5  6  7
  //v2: 8  9 10 11
  //v3:12 12 14 15
  
  
  Result.r[0].e[0] =  2.0f / (RightPlane - LeftPlane);
  Result.r[1].e[1] =  2.0f / (TopPlane - BottomPlane);
  Result.r[2].e[2] = -2.0f / (FarPlane -   NearPlane);
  
  Result.r[3].e[0] = -1.0f * ((RightPlane + LeftPlane  ) / (RightPlane -   LeftPlane));
  Result.r[3].e[1] = -1.0f * ((TopPlane   + BottomPlane) / (TopPlane   - BottomPlane));
  Result.r[3].e[2] = -1.0f * ((FarPlane   + NearPlane  ) / (FarPlane   -   NearPlane));
  Result.r[3].e[3] =  1.0f;
  
  return Result;
}
#endif //MATH_H
