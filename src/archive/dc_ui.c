#define UI_TEAL 0.75f, 1.00f, 1.0f, 0.4f
#define UI_PINK 1.00f, 0.75f, 1.0f, 0.4f


// TODO(MIGUEL): Z ordering of ui elements

void UIBeginFrame(platform *Platform)
{
  // at start of frame, clear old state and compute
  // relative-to-previous-frame state (like mouse deltas)
  UIState.Mouse    = v2fInit(Platform->Controls[0].MousePos.x,
                             (f32)Platform->WindowHeight - 
                             Platform->Controls[0].MousePos.y);
  
  UIState.LeftDown = Platform->Controls[0].MouseLeftButtonDown;
  UIState.LeftUp   = !Platform->Controls[0].MouseLeftButtonDown;
  
  
  // what's hot this frame is whatever was last in hot-to-be in last frame
  // (painter's algorithm--last-most widget is on top)
  UIState.Hot = UIState.WillBeHot;
  
  UIState.WillBeHot.Item   = NULL;
  UIState.WillBeHot.Parent = NULL;
  UIState.WillBeHot.Index  = 0;
  
  if (UIState.LastValid)
  {
    UIState.DeltaMouse = v2fSub(UIState.Mouse, UIState.LastMouse);
  }
  else
  {
    UIState.DeltaMouse = v2fInit(0, 0);
  }
  
  UIState.WentActive = 0;
  UIState.IsActive   = 0;
  UIState.IsHot      = 0;
  
  // pad edges by default spacing
  UILayout.PaddingX = UILayout.SpacingW;
  UILayout.PaddingY = UILayout.SpacingH;
  
  return;
}

void UIClear(void)
{
  UIState.LeftDown   = UIState.LeftUp   = 0;
  UIState.RightDown  = UIState.RightUp  = 0;
  UIState.MiddleDown = UIState.MiddleUp = 0;
}

void UIEndFrame(void)
{
  UIState.LastMouse = UIState.Mouse;
  UIState.LastValid = 1;
  
  // if we don't clear it, we can better handle button up/down in single frame
  // but then anything which doesn't get handled will break... we need to copy
  // button state, and if it's unchanged, clear, I think
  UIClear();
}

internaldef b32
UIAnyActive(void)
{
  b32 Result = (UIState.Active.Item != NULL);
  
  return Result;
}

internaldef
void UIClearActive(void)
{
  UIState.Active.Item   = NULL;
  UIState.Active.Parent = NULL;
  UIState.Active.Index  = 0;
  
  // mark all UI for this frame as processed
  UIClear();
  
  //PlatformCapture(1);
}

internaldef void 
UISetActive(void *ID)
{
  UIState.Active.Item   = ID;
  UIState.Active.Parent = UIState.CurParent;
  UIState.Active.Index  = UIState.CurIndex;
  
  UIState.WentActive = 1;
  
  //PlatformCapture(1);
}

internaldef void
UISetHot(void *ID)
{
  UIState.WillBeHot.Item   = ID;
  UIState.WillBeHot.Index  = UIState.CurIndex;
  UIState.WillBeHot.Parent = UIState.CurParent;
  
  return;
}

inline b32
UIIsHot(void *ID)
{
  ID = UIState.CurID ? UIState.CurID : ID;
  
  b32 Result = ((ID               ) == (UIState.Hot).Item  &&
                (UIState.CurIndex ) == (UIState.Hot).Index &&
                (UIState.CurParent) == (UIState.Hot).Parent);
  
  return Result;
}

inline b32
UIIsActive(void *ID)
{
  ID = UIState.CurID ? UIState.CurID : ID;
  
  b32 Result = ((ID               ) == (UIState.Active).Item  &&
                (UIState.CurIndex ) == (UIState.Active).Index &&
                (UIState.CurParent) == (UIState.Active).Parent);
  
  return Result;
}

b32 UIButtonLogic(void *ID, b32 Over)
{
  b32 Result = 0;
  
  // note that this logic happens correctly for button down then
  // up in one frame -- but not up then down
  
  // process down
  if (!UIAnyActive())
  {
    if (Over)                            UISetHot(ID);
    if (UIIsHot(ID) && UIState.LeftDown) UISetActive(ID);
    
  }
  
  // if button is active, then react on left up
  if (UIIsActive(ID))
  {
    UIState.IsActive = 1;
    
    if (Over) UISetHot(ID);
    
    if (UIState.LeftUp)
    {
      if (UIIsHot(ID)) Result = 1;
      
      UIClearActive();
    }
  }
  
  if (UIIsHot(ID)) UIState.IsHot = 1;
  
  return Result;   
}


// same as above, but return True on the down event
internaldef b32
UIButtonLogicDown(void *ID, b32 Over)
{
  b32 Result = 0;
  
  // note that this logic happens correctly for button down then
  // up in one frame -- but not up then down
  
  // process down
  if (!UIAnyActive())
  {
    if (Over) UISetHot(ID);
    
    if (UIIsHot(ID) && UIState.LeftDown)
    {
      UISetActive(ID);
      Result = 1;
    }
  }
  
  // if button is active, then react on left up
  if (UIIsActive(ID))
  {
    UIState.IsActive = 0;
    
    if (Over) UISetHot(ID);
    
    if (UIState.LeftUp) 
    {
      UIClearActive();
    }
  }
  
  if (UIIsHot(ID)) UIState.IsHot = 1;
  
  return Result;   
}


// a generic draggable rectangle... if you want its position clamped, do so yourself
b32
UIDragXY(r2f *Bounds, v2f *Pos, void *ID)
{
  b32 Result = 0;
  
  v2f DeltaPos = { 0 };
  
  if (UIButtonLogicDown(ID, r2fIsInRect(Bounds, &UIState.Mouse)))
  {
    
    UIState.Drag = v2fSub(*Pos, UIState.Mouse);
  }
  
  if (UIIsActive(ID))
  {
    if ((UIState.Mouse.x + UIState.Drag.x != Pos->x) || 
        (UIState.Mouse.y + UIState.Drag.y != Pos->y))
    {
      DeltaPos = v2fAdd(UIState.Mouse, UIState.Drag);
      
      *Pos = DeltaPos;
      
      Result = 1;
    }
  }
  
  return Result;
}

void
UIBoundingBox(render_data *RenderData, r2f EntityRect,
              v2f ViewPos, v2f ViewDim)
{
  v4f Color = v4fInit(1.0f, 0.0f, 0.0f, 1.0f);
  
  f32 EntityWidth  = (EntityRect.max.x - EntityRect.min.x);
  f32 EntityHeight = (EntityRect.max.y - EntityRect.min.y);
  
  f32 HalfWidth  = (EntityWidth / 2.0f);
  f32 HalfHeight = (EntityHeight / 2.0f);
  
  
  v2f LeftPos = v2fInit(EntityRect.min.x - 4.0f, EntityRect.min.y + HalfHeight);
  v2f LeftDim = v2fInit(1.0f, EntityHeight + 8.0f);
  
  v2f RightPos = v2fInit(EntityRect.max.x + 4.0f, EntityRect.min.y + HalfHeight);
  v2f RightDim = v2fInit(1.0f, EntityHeight + 8.0f);
  
  v2f TopPos = v2fInit(EntityRect.min.x + HalfWidth, EntityRect.max.y + 4.0f);
  v2f TopDim = v2fInit(EntityWidth + 8.0f, 1.0f);
  
  v2f BottomPos = v2fInit(EntityRect.min.x + HalfWidth, EntityRect.min.y - 4.0f);
  v2f BottomDim = v2fInit(EntityWidth + 8.0f, 1.0f);
  
  PushRect(RenderData, LeftPos, LeftDim  , Color, ViewPos, ViewDim);
  PushRect(RenderData, RightPos, RightDim  , Color, ViewPos, ViewDim);
  PushRect(RenderData, TopPos, TopDim  , Color, ViewPos, ViewDim);
  PushRect(RenderData, BottomPos, BottomDim  , Color, ViewPos, ViewDim);
  
  return;
}

internaldef b32
UIDragEdgeX (f32 *X, f32 HandleWidth, f32 MinY, f32 MaxY, void *ID)
{
  b32 Result = 0;
  // @TODO: copy above uiButtonDownLogic drag offseting code
  
  r2f EdgeButtonBounds =
  { 
    *X - (HandleWidth * 0.5f),
    MinY,
    *X + (HandleWidth * 0.5f),
    MaxY,
  };
  
  UIButtonLogic(ID, r2fIsInRect(&EdgeButtonBounds, &UIState.Mouse));
  
  if (UIIsActive(ID) && (UIState.Mouse.x != *X))
  { 
    *X = UIState.Mouse.x; 
    Result = 1; 
  }
  
  return Result;
}

internaldef b32
UIDragEdgeY (f32 *Y, f32 HandleWidth, f32 MinX, f32 MaxX, void *ID)
{
  b32 Result = 0;
  // @TODO: copy above uiButtonDownLogic drag offseting code
  
  r2f EdgeButtonBounds =
  { 
    .min.x = MinX,
    .min.y = *Y - (HandleWidth * 0.5f),
    .max.x = MaxX,
    .max.y = *Y + (HandleWidth * 0.5f),
  };
  
  UIButtonLogic(ID, r2fIsInRect(&EdgeButtonBounds, &UIState.Mouse));
  
  if (UIIsActive(ID) && (UIState.Mouse.y != *Y))
  { 
    *Y = UIState.Mouse.y; 
    Result = 1; 
  }
  
  return Result;
}

internaldef b32
UIDragCorner(v2f *Corner, f32 HandleWidth, void *ID)
{
  b32 Result = 0;
  
  r2f CornerButtonBounds = r2fInitFromPosRadius(0, Corner, HandleWidth / 2.0f);
  
  
  if (UIButtonLogicDown(ID, r2fIsInRect(&CornerButtonBounds, &UIState.Mouse)))
  {
    UIState.Drag = v2fSub(*Corner, UIState.Mouse);
  }
  
  if (UIIsActive(ID))
  {
    if ((UIState.Mouse.x + UIState.Drag.x != Corner->x) || 
        (UIState.Mouse.y + UIState.Drag.y != Corner->y))
    {
      *Corner = v2fAdd(UIState.Mouse, UIState.Drag);
      
      Result = 1;
    }
  }
  
  return Result;
}

void *RelID(void *OwnerID, void *SubID)
{
  void *Result = (void *)((u64)OwnerID ^
                          (u64)SubID);
  
  return Result;
}

void
UIProccessGuage(entity *Entity,
                f32 dt,
                f32 NormThrottlePos,
                memory_arena *TextArena,
                render_data *RenderData,
                v2f ViewPos,
                v2f ViewDim)
{
  
  // TODO(MIGUEL): Set limits on scaling and sizing
  b32 CenterHot;
  int HandleWidth = 5;
  // allow resizing from any edge or corner
  
  UIState.IsHot    = 0; // check whether any edges are currently being hovered
  UIState.IsActive = 0;
  
  /*
     *            H
*        |<----->|
     *   ___   _______
       *    ^   |       |
  *    |   |       |
  *  W |   |   +   |
  *    |   |   ^   |
     *   _v_  |___|___| 
     *            |
     *          (X, Y)
     *
*            (x1, y1)
     *         _______v
       *        |       |
  *        |       |
  *        |   +   |
  *        |       |
     *        |_______| 
     *        ^
     *     (x0, y0)
*
*
      */
  
  r2f EntityRect = r2fInit(0, &Entity->Dim, &Entity->Pos);
  {
    v2f DeltaPos = Entity->Pos; 
    
    if (UIDragXY(&EntityRect, &DeltaPos, Entity))
    {
      EntityRect.min = v2fAdd(EntityRect.min, v2fSub(DeltaPos, Entity->Pos));
      EntityRect.max = v2fAdd(EntityRect.max, v2fSub(DeltaPos, Entity->Pos));
    }
  }
  
  CenterHot = UIState.IsHot;
  UIState.IsHot = 0;
  
  // Dragging Edges
  UIDragEdgeX(&EntityRect.min.x, HandleWidth, EntityRect.min.y, EntityRect.max.y, 
              RelID(Entity, &EntityRect.min.x));
  UIDragEdgeX(&EntityRect.max.x, HandleWidth, EntityRect.min.y, EntityRect.max.y, 
              RelID(Entity, &EntityRect.max.x));
  
  UIDragEdgeY(&EntityRect.min.y, HandleWidth, EntityRect.min.x, EntityRect.max.x, 
              RelID(Entity, &EntityRect.min.y));
  UIDragEdgeY(&EntityRect.max.y, HandleWidth, EntityRect.min.x, EntityRect.max.x, 
              RelID(Entity, &EntityRect.max.y));
  
  UIState.CurIndex = 1; // change index id so that we can reuse same pointers as new handles
  HandleWidth      = 9; // corners have larger handles
  
  UIDragCorner(&EntityRect.min, HandleWidth,
               RelID(Entity, &EntityRect.min.x));
  UIDragCorner(&EntityRect.max, HandleWidth,
               RelID(Entity, &EntityRect.min.y));
  
  UIDragCorner(&EntityRect.min, HandleWidth,
               RelID(Entity, &EntityRect.max.y));
  UIDragCorner(&EntityRect.max, HandleWidth,
               RelID(Entity, &EntityRect.max.x));
  
  UIState.CurIndex = 0;
  
  v4f Color = { 0 };
  if (!CenterHot && !UIState.IsHot)
  {
    Color = v4fInit(UI_TEAL);
  }
  else
  {
    Color = v4fInit(UI_PINK);
  }
  
  if (UIState.IsHot)
  { 
    
    f32 EntityWidth  = (EntityRect.max.x - EntityRect.min.x);
    f32 EntityHeight = (EntityRect.max.y - EntityRect.min.y);
    
    f32 HalfWidth  = (EntityWidth / 2.0f);
    f32 HalfHeight = (EntityHeight / 2.0f);
    
    v2f LeftPos = v2fInit(EntityRect.min.x - 4.0f, EntityRect.min.y + HalfHeight);
    v2f LeftDim = v2fInit(1.0f, EntityHeight + 8.0f);
    
    v2f RightPos = v2fInit(EntityRect.max.x + 4.0f, EntityRect.min.y + HalfHeight);
    v2f RightDim = v2fInit(1.0f, EntityHeight + 8.0f);
    
    v2f TopPos = v2fInit(EntityRect.min.x + HalfWidth, EntityRect.max.y + 4.0f);
    v2f TopDim = v2fInit(EntityWidth + 8.0f, 1.0f);
    
    v2f BottomPos = v2fInit(EntityRect.min.x + HalfWidth, EntityRect.min.y - 4.0f);
    v2f BottomDim = v2fInit(EntityWidth + 8.0f, 1.0f);
    
    PushRect(RenderData, LeftPos, LeftDim  , Color, ViewPos, ViewDim);
    PushRect(RenderData, RightPos, RightDim  , Color, ViewPos, ViewDim);
    PushRect(RenderData, TopPos, TopDim  , Color, ViewPos, ViewDim);
    PushRect(RenderData, BottomPos, BottomDim  , Color, ViewPos, ViewDim);
  }
  else
  {
    //RenderRect(EntityRect.min.x,EntityRect.min.y, EntityRect.max.x,EntityRect.max.y, RGB_GREY(0));
  }
  
  if (UIState.IsActive)
  {
    g_SelectedEntity = Entity->Index;
  }
  
  r2fDimPosFromMinMax(&EntityRect, &Entity->Dim, &Entity->Pos);
  
  PushGuage(RenderData,
            Entity->Pos,
            Entity->Dim,
            NormThrottlePos,
            ViewPos,
            ViewDim);
  
  str8 Test = str8InitFromArenaFormated(TextArena,
                                        " Throttle: %2.2f%",
                                        100.0f * NormThrottlePos);
  
  PushLabel(RenderData,
            Test,
            v2fAddxy(Entity->Pos, (-Entity->Dim.x / 2.0f), -60.0f),
            0.5f,
            v3fInit(1.0f, 1.0f, 1.0f),
            ViewPos,
            ViewDim);
  
  return;
}

#if 0
internaldef void
UICreateGuage(app_state *AppState, v2f Pos, v2f Dim, memory_arena *Arena)
{
  u8 *StartupMsg = "Initializing...";
  
  str8 String = str8Init(StartupMsg, sizeof(StartupMsg));
  
  u32 EntityIndex = Entity_Create(AppState,
                                  Pos,
                                  Dim,
                                  Entity_guage);
  
  UITextPush(AppState, Arena, String,
             EntityIndex, Pos, 0.5f,
             v3f32Init(1.0f, 1.0f, 1.0f));
  
  MemoryCopy(StartupMsg, sizeof(StartupMsg) - 1,
             String.Data ,String.Count);
  
  return;
}
#endif