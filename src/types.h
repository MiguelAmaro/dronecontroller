#ifndef TYPES_H
#define TYPES_H

// CSTDLIB
#include <stdint.h>
#include <float.h>

#define global
#define fn
#define macfn

// LANG MACRO EXTENTIONS
// NOTE(MIGUEL): I think these convenience loops might be a mistake. lol 
#define VarToStr(symbol) (#symbol)
#undef NULL
#define NULL (void *)0x00
#define struct16 __declspec(align(16)) struct
#define Deref(ptr) (*(ptr))
#define ARG1(a, ...) (a)
#define ARG2(a, b, ...) (b)
#define foreach(a, b, type) for(type a=0;a<b;a++)
#define fromto(start, end, id, idx_type) for(idx_type idx=start;idx<end;idx++)
#define iterate(elem, list, type) for(type *elem = list->First; \
elem!= NULL; \
elem = elem->Next)
#define iteratenext(elem, list, next, type) for(type *elem = list->First; \
elem!= NULL; \
elem = elem->next)
#define Assert(expression) if(!(expression)){ __debugbreak(); } while (0)
//Compile Time Assert
#define CTASTR2(pre,post) pre ## post
#define CTASTR(pre,post) CTASTR2(pre,post)
#define StaticAssert(cond,msg) \
typedef struct { int CTASTR(static_assertion_failed_,msg) : !!(cond); } \
CTASTR(static_assertion_failed_,__COUNTER__)

#define OutofBounds(index, max) (index >= max)
#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))

#define Min(a, b) ((a<b)?a:b)
#define Max(a, b) ((a>b)?a:b)
#define Clamp(x, Low, High) (Min(Max(Low, x), High))

#define     Bytes(size) ((u64)(         (size) * (u64)1))
#define Kilobytes(size) ((u64)(         (size) * (u64)1024))
#define Megabytes(size) ((u64)(Kilobytes(size) * (u64)1024))
#define Gigabytes(size) ((u64)(Megabytes(size) * (u64)1024))
#define Terabytes(size) ((u64)(Gigabytes(size) * (u64)1024))



/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
(((i) & 0x80ll) ? '1' : '0'), \
(((i) & 0x40ll) ? '1' : '0'), \
(((i) & 0x20ll) ? '1' : '0'), \
(((i) & 0x10ll) ? '1' : '0'), \
(((i) & 0x08ll) ? '1' : '0'), \
(((i) & 0x04ll) ? '1' : '0'), \
(((i) & 0x02ll) ? '1' : '0'), \
(((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64    \
PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)
/* --- end macros --- */


// NOTE(allen): Linked List Macros

#define DLLPushBack_NP(f,l,n,next,prev) ((f)==0?\
((f)=(l)=(n),(n)->next=(n)->prev=0):\
((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=0))
#define DLLPushBack(f,l,n) DLLPushBack_NP(f,l,n,next,prev)
#define DLLPushFront(f,l,n) DLLPushBack_NP(l,f,n,prev,next)
#define DLLRemove_NP(f,l,n,next,prev) ((f)==(n)?\
((f)==(l)?\
((f)=(l)=(0)):\
((f)=(f)->next,(f)->prev=0)):\
(l)==(n)?\
((l)=(l)->prev,(l)->next=0):\
((n)->next->prev=(n)->prev,\
(n)->prev->next=(n)->next))
#define DLLInsert_NPZ(f,l,p,n,next,prev,zchk,zset) \
(zchk(f) ? (((f) = (l) = (n)), zset((n)->next), zset((n)->prev)) :\
zchk(p) ? (zset((n)->prev), (n)->next = (f), (zchk(f) ? (0) : ((f)->prev = (n))), (f) = (n)) :\
((zchk((p)->next) ? (0) : (((p)->next->prev) = (n))), (n)->next = (p)->next, (n)->prev = (p), (p)->next = (n),\
((p) == (l) ? (l) = (n) : (0))))
#define DLLPushBack_NPZ(f,l,n,next,prev,zchk,zset) DLLInsert_NPZ(f,l,l,n,next,prev,zchk,zset)
#define DLLRemove_NPZ(f,l,n,next,prev,zchk,zset) (((f)==(n))?\
((f)=(f)->next, (zchk(f) ? (zset(l)) : zset((f)->prev))):\
((l)==(n))?\
((l)=(l)->prev, (zchk(l) ? (zset(f)) : zset((l)->next))):\
((zchk((n)->next) ? (0) : ((n)->next->prev=(n)->prev)),\
(zchk((n)->prev) ? (0) : ((n)->prev->next=(n)->next))))
#define DLLRemove(f,l,n) DLLRemove_NP(f,l,n,next,prev)

#define SLLQueuePush_N(f,l,n,next) ((f)==0?\
(f)=(l)=(n):\
((l)->next=(n),(l)=(n)),\
(n)->next=0)
#define SLLQueuePush(f,l,n) SLLQueuePush_N(f,l,n,next)

#define SLLQueuePushFront_N(f,l,n,next) ((f)==0?\
((f)=(l)=(n),(n)->next=0):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_N(f,l,n,next)

#define SLLQueuePop_N(f,l,next) ((f)==(l)?\
(f)=(l)=0:\
(f)=(f)->next)
#define SLLQueuePop(f,l) SLLQueuePop_N(f,l,next)

#define SLLStackPush_N(f,n,next) ((n)->next=(f),(f)=(n))
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)

#define SLLStackPop_N(f,next) ((f)==0?0:\
(f)=(f)->next)
#define SLLStackPop(f) SLLStackPop_N(f,next)

// Mathematical Constants
#define Pi32     (3.141592653589793f)
#define Golden32 (1.618033988749894f)

typedef   uint8_t  u8;
typedef  uint16_t u16;
typedef  uint32_t u32;
typedef  uint64_t u64;
typedef    int8_t  s8;
typedef   int16_t s16;
typedef   int32_t s32;
typedef   int64_t s64;
typedef   uint8_t  b8;
typedef  uint16_t b16;
typedef  uint32_t b32;
typedef  uint64_t b64;
typedef     float f32;
typedef    double f64;
typedef void voidproc(void);

#include <stdarg.h>

#define F32Max FLT_MAX
#define U32Max UINT32_MAX
#define U64Max UINT64_MAX
#define S32Max INT32_MAX
#define S64Max INT64_MAX

typedef enum axis2 axis2;
enum axis2 { Axis2_X, Axis2_Y, Axis2_Count, };
typedef enum axis3 axis3;
enum axis3 { Axis3_X, Axis3_Y, Axis3_Z, Axis3_Count, };

//TIME
typedef struct datetime datetime;
struct datetime
{ s16 year; u8 mon; u8 day; u8 hour; u8 min; u8 sec; u16 ms; };

//VECTORS
typedef union v2f v2f;
union v2f
{
  struct { f32 x; f32 y; };
  f32 e[2];
};
typedef union v2s v2s;
union v2s
{
  struct { s32 x; s32 y; };
  s32 e[2];
};
typedef union v3f v3f;
union v3f
{
  struct { f32 x; f32 y; f32 z; };
  struct { f32 r; f32 g; f32 b; };
  struct { v2f xy; f32 __ig00; };
  struct { v2f rg; f32 __ig01; };
  struct { f32 __ig02; v2f yz; };
  f32 e[3];
};
typedef union v4f v4f;
union v4f
{
  struct { f32 x; f32 y; f32 z; f32 w; };
  struct { f32 r; f32 g; f32 b; f32 a; };
  struct { v3f xyz; f32 __ig01; };
  struct { v3f rgb; f32 __ig02; };
  f32 e[4];
};
typedef union v4u v4u;
union v4u
{
  struct { u32 x; u32 y; u32 z; u32 w; };
  struct { u32 r; u32 g; u32 b; u32 a; };
  u32 e[4];
};
// Inteivals
typedef union  i2f i2f;
union i2f
{
  struct { f32 minx; f32 miny; f32 maxx; f32 maxy; };
  struct { v2f min ; v2f max ; };
  f32 e[2];
};
typedef union i2s64 i2s64;
union i2s64
{
  struct { s64 minx; s64 miny; s64 max; s64 maxy; };
  s32 e[4];
};
// MATRICES
typedef union m2f m2f;
union m2f
{
  v2f r[2]; f32 e[4]; f32 x[2][2];
  struct {
    f32 _00, _01;
    f32 _10, _11;
  };
};
typedef union m3f m3f;
union m3f
{
  v3f r[3]; f32 e[9]; f32 x[3][3];
};
typedef union m4f m4f;
union m4f
{
  v4f r[4]; f32 e[16]; f32 x[4][4];
  struct {
    f32 _00, _01, _02, _03;
    f32 _10, _11, _12, _13;
    f32 _20, _21, _22, _23;
    f32 _30, _31, _32, _33;
  };
};
//- INITIALIZERS
#define V2f(...) _Generic(ARG1(__VA_ARGS__), \
f32: V2f_fxy, \
v2s: V2f_v2s)(__VA_ARGS__)
fn v2f V2f_fxy(f32 x, f32 y)
{
  v2f Result = { x, y };
  return Result;
}
fn v2f V2f_v2s(v2s a)
{
  v2f Result = { (f32)a.x, (f32)a.y };
  return Result;
}
fn v2s V2s(s32 x, s32 y)
{
  v2s Result = { x, y };
  return Result;
}
fn v3f V3f(f32 x, f32 y, f32 z)
{
  v3f Result = { x, y, z };
  return Result;
}
fn v4f V4f(f32 x, f32 y, f32 z, f32 w)
{
  v4f Result = { x, y, z, w };
  return Result;
}
fn v4u V4u(u32 x, u32 y, u32 z, u32 w)
{
  v4u Result = { x, y, z, w };
  return Result;
}
fn i2f I2f(f32 minx, f32 miny, f32 maxx, f32 maxy)
{
  Assert(minx<=maxx); Assert(miny<=maxy);
  i2f Result = { minx, miny, maxx, maxy };
  return Result;
}
fn m4f M4f(v4f r0, v4f r1, v4f r2, v4f r3)
{
  m4f Result = { r0, r1, r2, r3 };
  return Result;
}
fn b32 IsPowerOfTwo(u64 a)
{
  b32 Result = (a & (a-1)) == 0;
  return Result;
}
fn u32 SafeTruncateu64(u64 Value)
{
  Assert(Value <= 0xffffffff);
  u32 Result = (u32)Value;
  return Result;
}
fn i2f Rect2FromDimPos(v2f Dim, v2f Pos)
{
  // NOTE(MIGUEL): Should if have an enum to spec the meaning of position e.g TopLeft, Centered, ect.
  //               I will assume topleft for now.
  i2f Result = I2f(Pos.x, Pos.y, Pos.x+Dim.x, Pos.y+Dim.y);
  return Result;
}
fn i2f I2fUnion(i2f a, i2f b)
{
  i2f Result = I2f(Min(a.minx, b.minx),
                   Min(a.miny, b.miny),
                   Max(a.maxx, b.maxx),
                   Max(a.maxy, b.maxy));
  return Result;
}
#endif //TYPES_H
