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
