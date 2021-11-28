#include "dc.h"
#include "dc_ui.h"
#include "dc_math.h"

internaldef
void UIBeginFrame(platform *Platform)
{
    // at start of frame, clear old state and compute
    // relative-to-previous-frame state (like mouse deltas)
    UIState.Mouse = Platform->Controls[0].MousePos;
    
    // what's hot this frame is whatever was last in hot-to-be in last frame
    // (painter's algorithm--last-most widget is on top)
    UIState.Hot = UIState.WillBeHot;
    
    UIState.WillBeHot.Item   = NULL;
    UIState.WillBeHot.Parent = NULL;
    UIState.WillBeHot.Index  = 0;
    
    if (UIState.LastValid)
    {
        UIState.DeltaMouse = v2f32Sub(&UIState.Mouse, &UIState.LastMouse);
    }
    else
    {
        UIState.DeltaMouse = v2f32Init(0, 0);
    }
    
    UIState.WentActive = 0;
    UIState.IsActive   = 0;
    UIState.IsHot      = 0;
    
    // pad edges by default spacing
    UILayout.PaddingX = UILayout.SpacingW;
    UILayout.PaddingY = UILayout.SpacingH;
    
    return;
}

internaldef
void UIClear(void)
{
    UIState.LeftDown   = UIState.LeftUp   = 0;
    UIState.RightDown  = UIState.RightUp  = 0;
    UIState.MiddleDown = UIState.MiddleUp = 0;
}

internaldef
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

internaldef b32
UIButtonLogic(void *ID, b32 Over)
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
internaldef b32
UIDragXY(v2f32 *Pos, v2f32 *Dim, void *ID)
{
    b32 Result = 0;
    
    rect_v2f32 Bounds = rect_v2f32Init(0, Dim, Pos);
    
    if (UIButtonLogicDown(ID, rect_v2f32IsInRect(&Bounds, &UIState.Mouse)))
    {
        UIState.Drag = v2f32Sub(Pos, &UIState.Mouse);
    }
    
    if (UIIsActive(ID))
    {
        if ((UIState.Mouse.x + UIState.Drag.x != Pos->x) || 
            (UIState.Mouse.y + UIState.Drag.y != Pos->y))
        {
            *Pos = v2f32Add(&UIState.Mouse, &UIState.Drag);
            
            Result = 1;
        }
    }
    
    return Result;
}

internaldef b32
UIDragX (v2f32 *Pos, v2f32 *Dim, void *ID)
{
    b32 Result = 0;
    // @TODO: copy above uiButtonDownLogic drag offseting code
    rect_v2f32 Bounds = rect_v2f32Init(0, Dim, Pos);
    
    UIButtonLogic(ID, rect_v2f32IsInRect(&Bounds, &UIState.Mouse));
    
    if (UIIsActive(ID) && (UIState.Mouse.x != Pos->x))
    { 
        Pos->x = UIState.Mouse.x; 
        Result = 1; 
    }
    
    return Result;
}

internaldef b32
UIDragY (v2f32 *Pos, v2f32 *Dim, void *ID)
{
    b32 Result = 0;
    rect_v2f32 Bounds = rect_v2f32Init(0, Dim, Pos);
    
    UIButtonLogic(ID, rect_v2f32IsInRect(&Bounds, &UIState.Mouse));
    
    if (UIIsActive(ID) && (UIState.Mouse.y != Pos->y))
    { 
        Pos->y = UIState.Mouse.y; 
        Result = 1; 
    }
    
    return Result;
}

#if 0
internaldef void
UITextPush(app_state *AppState, memory_arena *Arena,
           str8 String, u32 EntityIndex,
           v2f32 Pos, f32 Scale, v3f32 Color)
{
    ui_text *UIText = AppState->UIText + AppState->UITextCount++;
    
    UIText->Pos      = Pos;
    UIText->Color    = Color;
    UIText->Scale    = Scale;
    UIText->EntityID = EntityIndex;
    
    UIText->String.Data  = MEMORY_ARENA_PUSH_ARRAY(Arena, String.Count, u8);
    UIText->String.Count = String.Count;
    
    MemoryCopy(String.Data, String.Count,
               UIText->String.Data, UIText->String.Count);
    
    return;
}
#endif

internaldef void
UIProccessGuage(entity *Entity, float dt)
{
    b32 CenterHot;
    int HandleWidth = 5;
    // allow resizing from any edge or corner
    
    UIState.IsHot    = 0; // check whether any edges are currently being hovered
    UIState.IsActive = 0;
    
    rect_v2f32 EntityRect;
    
    rect_v2f32Init(&EntityRect, &Entity->Dim, &Entity->Pos);
    // drag center region to move whole thing
    {
        int x = (EntityRect.min.x + EntityRect.max.x)/2;
        int y = (EntityRect.min.y + EntityRect.max.y)/2;
        int xp = x;
        int yp = y;
        
        // r aliases with &EntityRect.min.x, so use ID2(r)
        if (UIDragXY(&Entity->Pos, &Entity->Dim, ID2(Entity)))
        {
            // move both corners by same amount == dragging
            EntityRect.min.x += (x - xp);
            EntityRect.min.y += (y - yp);
            EntityRect.max.x += (x - xp);
            EntityRect.max.y += (y - yp);
        }
    }
    
    CenterHot = UIState.IsHot;
    UIState.IsHot = 0;
    
    // edges first, so corners are on top
    //UIDragX (&EntityRect.min.x,HandleWidth, EntityRect.min.y,EntityRect.max.y,   &EntityRect.min.x);
    //UIDragX (&EntityRect.max.x,HandleWidth, EntityRect.min.y,EntityRect.max.y,   &EntityRect.max.x);
    
    //UIDragY (EntityRect.min.x,EntityRect.max.x, &EntityRect.min.y,HandleWidth,   &EntityRect.min.y);
    //UIDragY (EntityRect.min.x,EntityRect.max.x, &EntityRect.max.y,HandleWidth,   &EntityRect.max.y);
    
    UIState.CurIndex = 1; // change index id so that we can reuse same pointers as new handles
    HandleWidth = 9; // corners have larger handles
    
    //UIDragXY(&EntityRect.min.x,HandleWidth, &EntityRect.min.y,HandleWidth, &EntityRect.min.x);
    //UIDragXY(&EntityRect.max.x,HandleWidth, &EntityRect.min.y,HandleWidth, &EntityRect.min.y);
    
    //UIDragXY(&EntityRect.min.x,HandleWidth, &EntityRect.max.y,HandleWidth, &EntityRect.max.y);
    //UIDragXY(&EntityRect.max.x,HandleWidth, &EntityRect.max.y,HandleWidth, &EntityRect.max.x);
    UIState.CurIndex = 0;
    
    if (!CenterHot && !UIState.IsHot)
    {
        Entity->Color.a = max(Entity->Color.a - 80 * dt, 200);
    }
    else
    {
        Entity->Color.a = min(Entity->Color.a + 80 * dt, 255);
    }
    
    //RenderRect(EntityRect.min.x,EntityRect.min.y,
    //EntityRect.max.x,EntityRect.max.y, &Entity->Color);
    
    if (UIState.IsHot)
    { 
        // highlight borders when they're draggable
        rect_v2f32 BlackInnerBorder, WhiteBorderFill, BlackOuterBorder;
        
        
        
        //RenderRect(EntityRect.min.x  ,EntityRect.min.y  , EntityRect.max.x  ,EntityRect.max.y  , RGB_GREY(0));
        //RenderRect(EntityRect.min.x+1,EntityRect.min.y+1, EntityRect.max.x-1,EntityRect.max.y-1, RGB_GREY(255));
        //RenderRect(EntityRect.min.x+2,EntityRect.min.y+2, EntityRect.max.x-2,EntityRect.max.y-2, RGB_GREY(0));
    }
    else
    {
        //RenderRect(EntityRect.min.x,EntityRect.min.y, EntityRect.max.x,EntityRect.max.y, RGB_GREY(0));
    }
    
    if (UIState.IsActive)
    {
        g_SelectedEntity = Entity->Index;
    }
    
    return;
}

#if 0
internaldef void
UICreateGuage(app_state *AppState, v2f32 Pos, v2f32 Dim, memory_arena *Arena)
{
    u8 *StartupMsg = "Initializing...";
    
    str8 String = str8Init(StartupMsg, sizeof(StartupMsg));
    
    u32 EntityIndex = Entity_Create(AppState,
                                    Pos,
                                    Dim,
                                    Entity_guage);
    
    UITextPush(AppState, Arena, String,
               EntityIndex, Pos, 0.5f,
               v3f32Init(1.0f, 0.0f, 1.0f));
    
    MemoryCopy(StartupMsg, sizeof(StartupMsg) - 1,
               String.Data ,String.Count);
    
    return;
}
#endif