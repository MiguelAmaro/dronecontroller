@echo off

if not exist build mkdir build

rem ====================    PROJECT/FILES      ====================
set PROJECT_NAME=dc
set SOURCES=^
F:\Dev\DroneController\src\win32_%PROJECT_NAME%.c ^
F:\Dev\DroneController\src\win32_opengl.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_opengl.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_memory.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_render_commands.c ^
F:\Dev\DroneController\lib\glad\src\glad.c

rem ====================    COMPILER(MSVC)     ====================
set MSVC_WARNINGS= -wd4700

set MSVC_FLAGS= %MSVC_WARNINGS% -nologo -Zi

set MSVC_SEARCH_DIRS= ^
-I ..\lib\ ^
-I ..\lib\freetype2\include\

rem ====================     LINKER(MSVC)       ====================
set MSVC_LIBS= ^
user32.lib ^
gdi32.lib ^
dinput8.lib ^
dxguid.lib ^
opengl32.lib ^
kernel32.lib ^
ws2_32.lib ^
shell32.lib ^
..\lib\freetype2\dlls\win64\freetype.lib

rem ************************************************************
rem **********              START BUILD               **********
rem ************************************************************
pushd build

set path=%path%;F:\Dev\DroneControl\build

taskkill /f /im win32_dc.exe

rem ====================     WINDOWS          ====================
call cl %MSVC_FLAGS% %MSVC_SEARCH_DIRS% %SOURCES% /link %MSVC_LIBS% 

popd

pause
