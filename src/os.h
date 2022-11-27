#ifndef OS_H
#define OS_H

typedef u64 os_handle;
typedef u64 os_handle;
typedef u64 os_window;

typedef struct time_entry time_entry;
struct time_entry
{
  u64 WorkBeginTick;
  u64 WorkEndTick;
  f64 MicrosElapsed;
  f64 TotalMicrosElapsed;
  f64 AvgMicrosElapsed;
  u64 CallCount;
  str8 FunctionName;
};
typedef struct time_measure time_measure;
struct time_measure
{
  u64 TickFrequency;
  u64 FrameCounter;
  time_entry Entries[256];
  time_entry *Top;
  time_entry *OnePastLast;
};
typedef os_handle os_module;
void OSInitTimeMeasure(time_measure *Time);
void OSTimePrepare(time_measure *Time, str8 FunctionName);
// TODO(MIGUEL): Finish this abominations!
time_entry OSTimeCapture(time_measure *Time);
time_measure TimeMeasure = {0};
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
fn void *OSMemoryAlloc(u64 Size);
//~ FILE STUFF
fn u64      OSFileGetSize(os_handle File);
fn str8     OSFileRead(str8 Path, arena *Arena);
fn datetime OSFileLastWriteTime(str8 FileName);
//~ CONSOLE STUFF

#define ConsoleLog(...) _Generic(ARG1(__VA_ARGS__), \
arena: OSConsoleLogF, \
char *: OSConsoleLogStrLit)(__VA_ARGS__)

fn u32 OSConsoleLogF(arena Scratch, char *Format, ...);
fn u32 OSConsoleLogStrLit(char *StringLit);
fn void OSConsoleCreate(void);
fn static void FatalError(const char* message);
//~ WINDOW
fn v2s OSWindowGetSize(os_handle Window);
//~ EVENTS
fn b32 OSProcessMessges(void);
//fn HWND OSWindowCreate(HINSTANCE Instance, v2s Dim, v2s Pos);

#endif //OS_H
