#ifndef UI_H
#define UI_H

#define UITEXT_MAX_COUNT 256
#define ID2(x)   (1 + (u8 *)(x))
#define ID3(x)   (2 + (u8 *)(x))
#define ID4(x)   (3 + (u8 *)(x))


typedef struct ui_context ui_context;
struct ui_context
{
  void *Item;
  void *Parent;
  s32   Index;
};

typedef struct ui_state ui_state;
struct ui_state
{
  // Mouse buttons
  b32 LeftUp;
  b32 LeftDown;
  
  b32 RightUp;
  b32 RightDown;
  
  b32 MiddleUp;
  b32 MiddleDown;
  
  v2f Mouse;
  v2f DeltaMouse;
  
  ui_context Active   ;
  ui_context Hot      ;
  ui_context WillBeHot;
  
  void *CurParent;
  s32   CurIndex;
  void *CurID;
  
  // this is set by whichever widget is hot/active; you can watch
  // for it to check
  b32 IsHot;
  b32 IsActive;
  
  // true on the first fraMousee a widget becoMousees active, usefuL for
  // Layering widget iMousepLeMouseentations
  b32 WentActive;
  
  // INTERNAL USE ONLY
  b32   LastValid;
  v2f LastMouse;
  
  v2f Drag; // drag offsets
};

typedef struct ui_layout ui_layout;
struct ui_layout
{
  u32 ForegroundColor;
  u32 BackgroundColor;
  u32 BackgroundColorDark;
  u32 BackgroundColorLight;
  
  s32 SpacingW;
  s32 SpacingH;
  
  s32 ForceW;
  s32 ForceWOnce;
  
  s32 ButtonPaddingW;
  s32 ButtonPaddingH;
  
  s32 SliderTabW;
  s32 SliderTabH;
  s32 SliderSlotH;
  s32 SliderSlotDefaultW; // only used if width can't be computed otherwise
  
  s32 SliderDotSpacing;
  
  s32 PaddingX;
  s32 PaddingY;
};
global ui_state  UIState;
global ui_layout UILayout;
global u32 g_SelectedEntity;
typedef struct ui_text ui_text;
struct ui_text
{
  u32 EntityID;
  v3f Color;
  str8 String;
  v2f Pos;
  f32 Scale;
};

void UIBeginFrame(platform *Platform);
void UIEndFrame(void);
void UIClear(void);
b32  UIButtonLogic(void *ID, b32 Over);
void UIProccessGuage(entity *Entity, f32 dt, f32 NormThrottlePos, arena *TextArena, render_data *RenderData, v2f ViewPos, v2f ViewDim);

b32 UIDragXY(r2f *Bounds, v2f *Pos, void *ID);

#endif //UI_H
