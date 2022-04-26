/* date = November 25th 2021 1:31 pm */

#ifndef DC_TYPES_H
#define DC_TYPES_H

#include <stdint.h>


#define ASSERT(Expression) if(!(Expression)){ __debugbreak(); }
#define ArrayCount(array) (sizeof(array) / sizeof(array[0]))

enum
{
  true  = 1,
  false = 0,
};

#define NULLPTR       ((void *)0x00)
#define readonly        const
#define internaldef     static
#define local_persist   static
#define global          static

#define KILOBYTES(size) (size * 1024)
#define MEGABYTES(size) (KILOBYTES(size) * 1024)
#define GIGABYTES(size) (MEGABYTES(size) * 1024)

typedef uint8_t  u8 ;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef  int8_t  s8 ;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;

typedef  int8_t  b8 ;
typedef  int16_t b16;
typedef  int32_t b32;
typedef  int64_t b64;

typedef float f32;
typedef float f64;

#endif //DC_TYPES_H
