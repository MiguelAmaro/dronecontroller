#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
HANDLE gConsole = NULL;

void OSInitTimeMeasure(time_measure *Time)
{
  Time->TickFrequency = 0;
  Time->FrameCounter = 0;
  Time->Top  = Time->Entries;
  Time->OnePastLast = Time->Entries + ArrayLength(Time->Entries);
  QueryPerformanceFrequency((LARGE_INTEGER *)&Time->TickFrequency);
  return;
}
void OSTimePrepare(time_measure *Time, str8 FunctionName)
{
  Assert((Time->Top<Time->OnePastLast));
  if(!(Time->Top<Time->OnePastLast)) return;
  time_entry *Entry = Time->Top++;
  time_entry NewEntry = 
  {
    .FunctionName = FunctionName,
    .WorkEndTick = 0,
    .WorkBeginTick = 0,
    .MicrosElapsed = 0.0,
    .TotalMicrosElapsed = 0.0,
    .AvgMicrosElapsed = 0.0, // NOTE(MIGUEL): not supported
  };
  QueryPerformanceCounter((LARGE_INTEGER *)&NewEntry.WorkBeginTick);
  *Entry = NewEntry;
  return;
}
// TODO(MIGUEL): Finish this abominations!
time_entry OSTimeCapture(time_measure *Time)
{
  time_entry Result = {0};
  Assert(!((Time->Top - 1)<Time->Entries));
  if((Time->Top - 1)<Time->Entries) return Result;
  time_entry *Entry = --Time->Top;
  QueryPerformanceCounter((LARGE_INTEGER *)&Entry->WorkEndTick);
#define Microseconds 1000000
  u64 TickDelta = Entry->WorkEndTick-Entry->WorkBeginTick;
  Entry->MicrosElapsed = TickDelta*(f64)Microseconds/Time->TickFrequency;
  Entry->TotalMicrosElapsed += Entry->MicrosElapsed;
  Entry->AvgMicrosElapsed    = Entry->TotalMicrosElapsed/(f64)Time->FrameCounter;
#undef Microseconds
  Result = *Entry;
  return Result;
}
#define OSProfileStart() OSTimePrepare(&TimeMeasure, Str8(__FUNCTION__))
#define OSProfileLinesStart(name) OSTimePrepare(&TimeMeasure, Str8(name))
#define OSProfileEnd()  \
do { \
time_entry __entry = OSTimeCapture(&TimeMeasure); \
arena __lgar = {0}; ArenaLocalInit(__lgar, 1042);  \
ConsoleLog(__lgar, "[%s]elapsed: %lfs \n", __entry.FunctionName.Data, __entry.MicrosElapsed/1000000.0); \
} while(0)
// NOTE(MIGUEL): this is because ConsoleLog is not defined yet
//~ MEMORY
fn void *OSMemoryAlloc(u64 Size)
{
  void *Result = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  Assert(Result);
  return Result;
}
//~ FILE STUFF
fn u64 OSFileGetSize(os_handle File)
{
  HANDLE Handle = (HANDLE)File;
  // TODO(MIGUEL): a random sequence of charaters followed by a comma causes this to fail. debug!!
  LARGE_INTEGER Whatever = { 0 };
  u64 Result = 0;
  b32 Status = GetFileSizeEx(Handle, &Whatever);
  Result = Whatever.QuadPart;
  if(Status == INVALID_FILE_SIZE)
  {
    Assert(Status != INVALID_FILE_SIZE);
  }
  return Result;
}
fn b32 OSFileExists(str8 FileName)
{
  // NOTE(MIGUEL): Can also get file attributes
  WIN32_FILE_ATTRIBUTE_DATA Ignored;
  b32 Result = (0 == GetFileAttributesEx((LPCSTR)FileName.Data,
                                         GetFileExInfoStandard,
                                         &Ignored));
  return Result;
}
fn void OSFileCopy(str8 b, str8 a)
{
  CopyFile((const char *)a.Data,
           (const char *)b.Data, FALSE);
  return;
}
fn str8 OSFileRead(str8 Path, arena *Arena)
{
  str8 Result = {0};
  HANDLE File = CreateFileA((LPCSTR)Path.Data, GENERIC_READ, FILE_SHARE_READ,
                            0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if(File)
  {
    u64 ExpectedSize = OSFileGetSize((os_handle)File); Assert(ExpectedSize<U32Max);
    u64 BytesRead    = 0;
    Result = Str8(Arena, ExpectedSize);
    ReadFile(File, Result.Data, (u32)ExpectedSize, (LPDWORD)&BytesRead, NULL);
    Assert(BytesRead == ExpectedSize);
    Result.Length = BytesRead;
  }
  CloseHandle(File);
  return Result;
}
fn datetime OSFileLastWriteTime(str8 FileName)
{
  /* Maybe usefull code
  WIN32_FIND_DATAA *LineShaderFileInfo = &Renderer->LineShaderFileInfo;
  FindFirstFileA(LineShaderPath,
                 LineShaderFileInfo);
  */
  datetime Result = {0};
  FILETIME   FileTime = {0};
  SYSTEMTIME SysTime  = {0};
  WIN32_FILE_ATTRIBUTE_DATA FileInfo;
  // TODO(MIGUEL): Need to have proper filehadling. Crashing/breaking here not acceptable.
  //               need when there is c hot reloading or ui to change or input file paths
  Assert(GetFileAttributesEx((LPCSTR)FileName.Data, GetFileExInfoStandard, &FileInfo));
  FileTime = FileInfo.ftLastWriteTime;
  FileTimeToSystemTime(&FileTime, &SysTime);
  Result.year = (s16)SysTime.wYear;
  Result.mon  = (u8) SysTime.wMonth;
  //Result.day  = SysTime.wDayOfWeek;
  Result.day  = (u8)SysTime.wDay;
  Result.hour = (u8)SysTime.wHour;
  Result.min  = (u8)SysTime.wMinute;
  Result.sec  = (u8)SysTime.wSecond;
  Result.ms   = (u8)SysTime.wMilliseconds;
  return Result;
}
//~ MODULES
os_module OSModuleLoad(str8 Path)
{
  StrIsNullTerminated(Path);
  HMODULE Module = LoadLibraryA((const char *)Path.Data);
  return (os_module)Module;
}
void OSModuleUnload(os_module Module)
{
  FreeLibrary((HMODULE)Module);
  return;
}
voidproc *OSModuleGetProc(os_module Module, str8 ProcName)
{
  StrIsNullTerminated(ProcName);
  voidproc *Proc = (voidproc *)GetProcAddress((HMODULE)Module, (LPCSTR)ProcName.Data);
  return Proc;
}
//~ CONSOLE STUFF

#define ConsoleLog(...) _Generic(ARG1(__VA_ARGS__), \
arena: OSConsoleLogF, \
char *: OSConsoleLogStrLit)(__VA_ARGS__)
fn u32 OSConsoleLogF(arena Scratch, char *Format, ...)
{
  va_list ArgList;
  va_start(ArgList, Format);
  str8 String = Str8FromArena(&Scratch, vsnprintf(NULL, 0, Format, ArgList) + 1);
  vsnprintf((char *)String.Data, String.Length, Format, ArgList);
  u32 WriteCount = 0;
  WriteConsoleA(gConsole, String.Data, (u32)String.Length, (LPDWORD)&WriteCount, NULL);
  return WriteCount;
}
fn u32 OSConsoleLogStrLit(char *StringLit)
{
  str8 String = Str8((u8 *)StringLit, CStrGetLength(StringLit, 1));
  u32 WriteCount = 0;
  WriteConsoleA(gConsole, String.Data, (u32)String.Length, (LPDWORD)&WriteCount, NULL);
  return WriteCount;
}
fn void OSConsoleCreate(void)
{
  b32 Status = AllocConsole();
  Assert(Status != 0);
  gConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       CONSOLE_TEXTMODE_BUFFER,
                                       NULL);
  COLORREF Colors[16] = {0x00000000, 0x00ffff00, 0x00ff00ff, 0x00ffffff, 0x00ffffff };
  CONSOLE_SCREEN_BUFFER_INFOEX BufferDesc = {
    .cbSize = sizeof(BufferDesc),
    .dwSize = (COORD){100, 4000},
    .dwCursorPosition = (COORD){0, 0},
    .wAttributes = 0 |  1, //Each bit
    .srWindow = (SMALL_RECT){0, 0, 60, 100},
    .dwMaximumWindowSize = (COORD){60, 100},
    //WORD       wPopupAttributes;
    //BOOL       bFullscreenSupported;
  };
  MemoryCopy(Colors, sizeof(Colors), BufferDesc.ColorTable, sizeof(BufferDesc.ColorTable));
  Status = SetConsoleScreenBufferInfoEx((HANDLE)gConsole, &BufferDesc);
  //Assert(Status != 0);
  Status = SetConsoleActiveScreenBuffer((HANDLE)gConsole);
  Assert(Status != 0);
  return;
}
fn static void FatalError(const char* message)
{
  MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
  ExitProcess(0);
}
//~ WINDOW
fn v2s OSWindowGetSize(os_handle Window)
{
  HWND Handle = (HWND)Window;
  RECT Rect = {0};
  GetClientRect(Handle, &Rect); // get current size for window client area
  v2s WindowDim = V2s(Rect.right  - Rect.left, Rect.bottom - Rect.top);
  return WindowDim;
}
//~ EVENTS

fn b32 OSProcessMessges(void)
{
  MSG Msg;
  b32 ShouldQuit = 0;
  while(PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE))
  {
    if (Msg.message == WM_QUIT)
    { 
      ShouldQuit = 1;
      break;
    }
    TranslateMessage(&Msg);
    DispatchMessageW(&Msg);
  }
  return ShouldQuit;
}
static LRESULT CALLBACK WindowEventProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch (msg)
  {
    case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(wnd, msg, wparam, lparam);
}
fn HWND OSWindowCreate(HINSTANCE Instance, v2s Dim, v2s Pos)
{
  // register window class to have custom WindowProc callback
  WNDCLASSEXW WindowClass =
  {
    .cbSize = sizeof(WindowClass),
    .lpfnWndProc = WindowEventProc,
    .hInstance = Instance,
    .hIcon = LoadIcon(NULL, IDI_APPLICATION),
    .hCursor = LoadCursor(NULL, IDC_ARROW),
    .lpszClassName = L"d3d11_window_class",
  };
  ATOM atom = RegisterClassExW(&WindowClass);
  Assert(atom && "Failed to register window class");
  DWORD exstyle = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;
  DWORD style = WS_OVERLAPPEDWINDOW;
  // create window
  HWND Window = CreateWindowExW(exstyle, WindowClass.lpszClassName,
                                L"DroneController", style,
                                Pos.x, Pos.y, Dim.x, Dim.y,
                                NULL, NULL, WindowClass.hInstance, NULL);
  return Window;
}
