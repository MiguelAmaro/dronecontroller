#if RENDERER_OPENGL

app_state *AppState = (app_state *)g_Platform.PermanentStorage;
#if 0
entity *Entity = AppState->Entities;
for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++, Entity++)
{
    m4f32 ThrottleWidgetProjection = m4f32Orthographic(0.0f, g_Platform.WindowWidth,
                                                       0.0f, g_Platform.WindowHeight,
                                                       0.1f, 100.0f);
    
    
    m4f32 ThrottleWidgetTransform = m4f32Identity();
    m4f32 Trans  = m4f32Translate(v3f32Init(Entity->Pos.x, Entity->Pos.y, 0.0f));
    m4f32 Scale  = m4f32Scale(Entity->Dim.x / 2.0f, Entity->Dim.y / 2.0f, 1.0f);
    m4f32 Rotate = m4f32Identity();
    
    m4f32 World = m4f32Multiply(Scale, Trans);
    ThrottleWidgetTransform = m4f32Multiply(World, ThrottleWidgetProjection);
    
    GL_Call(glUseProgram(Sprite->ShaderID));
    GL_Call(glUniformMatrix4fv(Sprite->UThrottleTransform, 1, 0, ThrottleWidgetTransform.e));
    GL_Call(glUniform2fv(Sprite->UWindowSize , 1, v2f32Init(g_Platform.WindowWidth,
                                                            g_Platform.WindowHeight).c));
    GL_Call(glUniform2fv(Sprite->UThrottlePos , 1, Entity->Pos.c));
    GL_Call(glUniform2fv(Sprite->UThrottleSize, 1, Entity->Dim.c));
    GL_Call(glUniform1f(Sprite->UThrottle, g_Platform.Controls[0].NormThrottlePos));
    GL_Call(glUniform1f(Sprite->UDeltaTime, AppState->DeltaTime));
    
    GL_Call(glEnable(GL_BLEND));
    GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    GL_Call(glBindVertexArray(Sprite->VertexAttribID));
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_Call(glBindVertexArray(0));
}
#endif


#if 0
u8 *Param = "Thread Started !!!\n\r";

Win32State->CommThreadInfo.LogicalThreadIndex = 0;
DWORD ThreadID;
Win32State->CommThreadInfo.Handle = CreateThread(0, 0,
                                                 win32_SerialPort_RecieveData,
                                                 Param,
                                                 0,
                                                 &Win32State->CommThreadInfo.ID);
#endif


if(BytesRead == TelemetryPacket.Header.PayloadSize)
{
    MemoryCopy(Buffer, 256, TelemetryPacket.Payload, 256);
    
    TelemetryEnqueuePacket(Device, Telem_QueueRecieve, TelemetryPacket);
}

MemorySet(&Device->Overlapped,
          sizeof(Device->Overlapped),
          0);

MemorySet(Buffer, 256, 0);


AppState->DeltaTime += Platform->CurrentTime - Platform->LastTime;
f32 MoveSpeed  = -200.0f *  AppState->DeltaTime;

//~ INPUT RESPONSE
if(Platform->Controls[0].AlphaKeys[Key_q].IsDown)
{
    Platform->QuitApp = 1;
}

if(Platform->Controls[0].AlphaKeys[Key_o].IsReleasedNow)
{
    Entity_Create(AppState, MousePos, v2f32Init(400.0f, 60.0f), Entity_output);
}

// TODO(MIGUEL): move this back to win32
if(Platform->Controls[0].AlphaKeys[Key_c].IsReleasedNow)
{
    // TODO(MIGUEL): Should connect to a board
    
}

if(Platform->Controls[0].AlphaKeys[Key_w].IsReleasedNow)
{
    printf("I made a ui element(guage). \n");
    Entity_Create(AppState, MousePos, v2f32Init(400.0f, 60.0f), Entity_guage);
}

#if 0
if(Platform->Controls[0].AlphaKeys[Key_d].IsReleasedNow)
{
    printf("I made a ui element(guage). \n");
    Entity_Create(AppState, MousePos, v2f32Init(400.0f, 60.0f), Entity_guage);
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
          v2f32Init(Platform->WindowWidth * 0.2f,
                    Platform->WindowHeight * 0.8f),
          0.75f, v3f32Init(1.0f, 1.0f, 0.0f),
          MainView->Pos,
          MainView->Dim);

PushLabel(RenderData,
          TelemetryStatus,
          v2f32Init(Platform->WindowWidth * 0.2f,
                    Platform->WindowHeight * 0.5f),
          0.5f, v3f32Init(1.0f, 1.0f, 1.0f),
          MainView->Pos,
          MainView->Dim);


view View = AppState->Views[0];
u8 *MessageDump = (u8 *)MemoryArenaPushBlock(&TextArena, 4096);
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

v2f32 DumpDelta = Dump->Pos; 

r2f32 DumpRect = r2f32Init(0, &Dump->Dim, &Dump->Pos);
UIBoundingBox(RenderData, DumpRect, View.Pos, View.Dim);
if (UIDragXY(&DumpRect, &DumpDelta, &Dump))
{
    DumpRect.min = v2f32Add(DumpRect.min, v2f32Sub(DumpDelta, Dump->Pos));
    DumpRect.max = v2f32Add(DumpRect.max, v2f32Sub(DumpDelta, Dump->Pos));
}

r2f32ConvertToDimPos(&DumpRect, &Dump->Dim, &Dump->Pos);

PushLabel(RenderData,
          PacketDump,
          Dump->Pos,
          0.4, v3f32Init(1.0f, 1.0f, 1.0f),
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
            
            v2f32 DeltaPos = Entity->Pos; 
            
            
            r2f32 EntityRect = r2f32Init(0, &Entity->Dim, &Entity->Pos);
            if (UIDragXY(&EntityRect, &DeltaPos, Entity))
            {
                EntityRect.min = v2f32Add(EntityRect.min, v2f32Sub(DeltaPos, Entity->Pos));
                EntityRect.max = v2f32Add(EntityRect.max, v2f32Sub(DeltaPos, Entity->Pos));
            }
            
            r2f32ConvertToDimPos(&EntityRect, &Entity->Dim, &Entity->Pos);
            
            PushLabel(RenderData,
                      DroneMsg,
                      Entity->Pos, 0.75f, v3f32Init(1.0f, 1.0f, 0.0f),
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


