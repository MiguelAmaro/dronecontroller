#ifndef ENTITY_H
#define ENTITY_H
typedef enum entity_type entity_type;
enum entity_type
{
  Entity_guage,
  Entity_output,
  Entity_hd,
};
typedef struct entity entity;
struct entity
{
  u32 Index;
  entity_type Type;
  v2f Dim;
  v2f Pos;
  u8 HotAlpha;
  v4f Color;
  u32 RenderInfoID;
};
#endif //ENTITY_H
