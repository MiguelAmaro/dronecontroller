#ifndef DRONECONTROLLER_H
#define DRONECONTROLLER_H

#define ENTITY_MAX_COUNT 256

typedef struct view view;
struct view
{
  v2f Pos;
  v2f Dim;
};
typedef struct app_state app_state;
struct app_state
{
  b32 IsInitialized;
  u8  Throttle;
  f32 DeltaTime;
  
  entity Entities[ENTITY_MAX_COUNT];
  u32    EntityCount;
  u32    EntityMaxCount;
  
  // TODO(MIGUEL): figure out and note wtf any of this 
  //               is for.
  ui_text UIText[UITEXT_MAX_COUNT];
  u32     UITextCount;
  u32     UITextMaxCount;
  
  view Views[16];
  u32  ViewCount;
  u32  ViewMax;
  
  arena UITextArena;
};

typedef struct app_backbuffer app_backbuffer;
struct app_backbuffer
{
  void *Data           ;
  s32   Width          ;
  s32   Height         ;
  s32   BytesPerPixel;
};

typedef struct device_info device_info;
struct device_info
{
  b32                  Connected;
  telem_state          State;
  telem_packet_queues *PacketQueues;
};


#define APP_UPDATE(name) void name(platform *Platform, device_info *Device, render_data *RenderData)
typedef APP_UPDATE(api_app_update);
APP_UPDATE(api_app_update_stub)
{ return; }


#endif //DRONECONTROLLER_H
