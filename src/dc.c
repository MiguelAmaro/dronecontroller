
#include "dc.h"

#include "dc_ui.h"
#include "dc_math.h"
#include "dc_strings.h"
#include "dc_entity.h"
#include <stdio.h>
#include <math.h>


internaldef
u32 Entity_Create(app_state *AppState, v2f Pos , v2f Dim, entity_type Type)
{
  ASSERT(AppState->EntityCount < ENTITY_MAX_COUNT);
  
  entity *Entity = AppState->Entities + AppState->EntityCount;
  
  Entity->Index = AppState->EntityCount;
  Entity->Type = Type;
  Entity->Dim = Dim;
  Entity->Pos = Pos;
  
  return AppState->EntityCount++;
}

void
App_Init(platform *Platform_)
{
  return;
}

APP_UPDATE(Update)
{
  app_state *AppState = (app_state *)Platform->PermanentStorage;
  
  //~ MEMORY & INITIALIZATION
  memory_arena TextArena = MemoryArenaInit(0, MEGABYTES(10),
                                           Platform->TransientStorage);
  
  v2f MousePos = v2fInit(Platform->Controls->MousePos.x,
                         ((f32)Platform->WindowHeight - Platform->Controls->MousePos.y));
  
  if(!AppState->IsInitialized)
  {
    // TODO(MIGUEL): Debug the Render text first causes a crash problem.
    printf("I made a ui element(guage). \n");
    Entity_Create(AppState,
                  v2fInit(Platform->WindowWidth * 0.2f,
                          Platform->WindowHeight * 0.3f),
                  v2fInit(400.0f, 60.0f), Entity_guage);
    
    Entity_Create(AppState,
                  v2fInit(Platform->WindowWidth * 0.1f,
                          400.0f),
                  v2fInit(800.0f, 100.0f), Entity_hd);
    
    AppState->ViewMax = 16;
    AppState->ViewCount = 0;
    
    
    //everything else
    AppState->Views[1].Pos = v2fInit(0, Platform->WindowHeight);
    AppState->Views[1].Dim = v2fInit(1200, Platform->WindowHeight * 0.6f);
    AppState->ViewCount++;
    
    // NOTE(MIGUEL): This is for hex dump & is test code
    AppState->Views[0].Pos = v2fInit(0, Platform->WindowHeight * 0.4f);
    AppState->Views[0].Dim = v2fInit(Platform->WindowWidth, Platform->WindowHeight * 0.4f);
    AppState->ViewCount++;
    
    AppState->IsInitialized = 1;
  }
  
  AppState->Views[1].Pos = v2fInit(0, Platform->WindowHeight);
  AppState->Views[1].Dim = v2fInit(Platform->WindowWidth, Platform->WindowHeight * 0.6f);
  
  AppState->Views[0].Pos = v2fInit(0, Platform->WindowHeight * 0.4f);
  AppState->Views[0].Dim = v2fInit(Platform->WindowWidth, Platform->WindowHeight * 0.4f);
  
  PushClear(RenderData, v4fInit(0.12f, 0.12f, 0.12f, 1.0f));
  
  UIBeginFrame(Platform);
  
  AppState->DeltaTime += Platform->CurrentTime - Platform->LastTime;
  f32 MoveSpeed  = -200.0f *  AppState->DeltaTime;
  
  //~ INPUT RESPONSE
  if(Platform->Controls[0].AlphaKeys[Key_q].IsDown)
  {
    Platform->QuitApp = 1;
  }
  
  if(Platform->Controls[0].AlphaKeys[Key_o].IsReleasedNow)
  {
    Entity_Create(AppState, MousePos, v2fInit(400.0f, 60.0f), Entity_output);
  }
  
  // TODO(MIGUEL): move this back to win32
  if(Platform->Controls[0].AlphaKeys[Key_c].IsReleasedNow)
  {
    // TODO(MIGUEL): Should connect to a board
    
  }
  
  if(Platform->Controls[0].AlphaKeys[Key_w].IsReleasedNow)
  {
    printf("I made a ui element(guage). \n");
    Entity_Create(AppState, MousePos, v2fInit(400.0f, 60.0f), Entity_guage);
  }
  
#if 0
  if(Platform->Controls[0].AlphaKeys[Key_d].IsReleasedNow)
  {
    printf("I made a ui element(guage). \n");
    Entity_Create(AppState, MousePos, v2fInit(400.0f, 60.0f), Entity_guage);
  }
#endif
  
  if(Platform->Controls[0].AlphaKeys[Key_v].IsReleasedNow)
  {
    //CreateHorizontalPanel();
  }
  if(Platform->Controls[0].AlphaKeys[Key_h].IsReleasedNow)
  {
    //CreateVerticalPanel();
  }
  
  if(Platform->Controls[0].AlphaKeys[Key_s].IsDown)
  {
    entity *Entity = AppState->Entities;
    
    Entity->Pos.x  = Platform->WindowWidth  / 2.0f;
    Entity->Pos.y  = Platform->WindowHeight / 2.0f;
  }
  
  view *MainView = AppState->Views + (AppState->ViewCount - 1);
  
  telem_packet Packet = TelemetryDequeuePacket(Device->PacketQueues,
                                               Telem_QueueRecieve);
  
  str8 DroneMsg = str8InitFromArenaFormated(&TextArena,
                                            "Drone Message: %s",
                                            Packet.Payload);
  
  str8 ConnectStat = str8InitFromArenaFormated(&TextArena,
                                               " MCU %s",
                                               Device->Connected==1?
                                               "Connected": "Disconnected");
  
  
  // NOTE(MIGUEL): Should I create some sort of string table.
  u8 *TelemStatus;
  u8 ConnLost[] = "Connection Lost";
  u8 PacketRx[] = "Receiving Packet...";
  u8 Unknown[]  = "State Unknown...";
  switch(Device->State)
  {
    case Telem_NoConnection:
    {
      TelemStatus = ConnLost;
    } break;
    case Telem_ReceivingPacket:
    {
      TelemStatus = PacketRx;
    } break;
    default:
    {
      TelemStatus = "State Unknown...";
    } break;
  }
  
  str8 TelemetryStatus = str8InitFromArenaFormated(&TextArena,
                                                   "Telemetry: %s",
                                                   TelemetryStatus);
  PushLabel(RenderData,
            ConnectStat,
            v2fInit(Platform->WindowWidth * 0.2f,
                    Platform->WindowHeight * 0.8f),
            0.75f, v3fInit(1.0f, 1.0f, 0.0f),
            MainView->Pos,
            MainView->Dim);
  
  PushLabel(RenderData,
            TelemetryStatus,
            v2fInit(Platform->WindowWidth * 0.2f,
                    Platform->WindowHeight * 0.5f),
            0.5f, v3fInit(1.0f, 1.0f, 1.0f),
            MainView->Pos,
            MainView->Dim);
  
  
  u8 *MessageDump = (u8 *)MemoryArenaPushBlock(&TextArena, 4096);
  view View = AppState->Views[0];
  u32 MessageDumpSize = MemoryHexDump(MessageDump, 4096,
                                      Packet.Payload,
                                      TELEM_PAYLOAD_MAXSIZE,
                                      32, 8, 4);
  
  str8 PacketDump = str8Init(MessageDump, MessageDumpSize);
  
  entity *Dump;
  for(u32 i = 0; i < AppState->EntityCount; i++)
  {
    if(AppState->Entities[i].Type == Entity_hd)
    {
      Dump = AppState->Entities + i;
    }
  }
  
  v2f DumpDelta = Dump->Pos; 
  
  r2f DumpRect = r2fInit(0, &Dump->Dim, &Dump->Pos);
  UIBoundingBox(RenderData, DumpRect, View.Pos, View.Dim);
  if (UIDragXY(&DumpRect, &DumpDelta, &Dump))
  {
    DumpRect.min = v2fAdd(DumpRect.min, v2fSub(DumpDelta, Dump->Pos));
    DumpRect.max = v2fAdd(DumpRect.max, v2fSub(DumpDelta, Dump->Pos));
  }
  
  r2fDimPosFromMinMax(&DumpRect, &Dump->Dim, &Dump->Pos);
  
  PushLabel2(RenderData,
             PacketDump,
             Dump->Pos,
             0.4, v3fInit(1.0f, 1.0f, 1.0f),
             View.Pos, View.Dim);
  
  
  //~ UPDATE
  entity *Entity = AppState->Entities;
  for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
  {
    switch(Entity->Type)
    {
      case Entity_output:
      {
        
        ASSERT(DroneMsg.Count != 4294967295);
        
        v2f DeltaPos = Entity->Pos; 
        
        
        r2f EntityRect = r2fInit(0, &Entity->Dim, &Entity->Pos);
        if (UIDragXY(&EntityRect, &DeltaPos, Entity))
        {
          EntityRect.min = v2fAdd(EntityRect.min, v2fSub(DeltaPos, Entity->Pos));
          EntityRect.max = v2fAdd(EntityRect.max, v2fSub(DeltaPos, Entity->Pos));
        }
        
        r2fDimPosFromMinMax(&EntityRect, &Entity->Dim, &Entity->Pos);
        
        PushLabel(RenderData,
                  DroneMsg,
                  Entity->Pos, 0.75f, v3fInit(1.0f, 1.0f, 0.0f),
                  MainView->Pos,
                  MainView->Dim);
        
      } break;
      case Entity_guage:
      {
        UIProccessGuage(Entity,
                        AppState->DeltaTime,
                        Platform->Controls->NormThrottlePos,
                        &TextArena,
                        RenderData,
                        MainView->Pos,
                        MainView->Dim);
      } break;
    }
    
  }
  UIProccessGuage(Entity,
                  AppState->DeltaTime,
                  Platform->Controls->NormThrottlePos,
                  &TextArena,
                  RenderData,
                  MainView->Pos,
                  MainView->Dim);
  
  ui_text *UIText = AppState->UIText;
  for(u32 UITextIndex = 0; UITextIndex < AppState->UITextCount; UITextIndex++, UIText++)
  {
    
    entity *Entities = AppState->Entities + UIText->EntityID;
    
    UIText->Pos = Entity->Pos;
    
  }
  
  UIEndFrame();
  
  return;
}
