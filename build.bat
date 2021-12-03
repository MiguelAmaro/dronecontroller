@echo off

set OUTPUT=%1

if not exist build mkdir build

rem ====================    PROJECT/FILES      ====================
set PROJECT_NAME=dc
set SOURCES=^
F:\Dev\DroneController\src\win32_%PROJECT_NAME%.c ^
F:\Dev\DroneController\src\win32_opengl.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_opengl.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_memory.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_strings.c ^
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
"..\lib\freetype2\release dll\win64\freetype.lib"

rem ************************************************************
rem **********              START BUILD               **********
rem ************************************************************
pushd build

set path=%path%;F:\Dev\DroneControl\build

if "%OUTPUT%" equ "-exe" (goto :COMPILE_EXE)
if "%OUTPUT%" equ "-dll" (goto :COMPILE_DLL)
goto :COMPILE_EXE rem !!!DEFAULT PATH!!!


rem ====================     WINDOWS          ====================
:COMPILE_EXE
taskkill /f /im win32_dc.exe
pushd build
call cl %MSVC_FLAGS% %MSVC_SEARCH_DIRS% %SOURCES% /link -subsystem:windows -incremental:no %MSVC_LIBS% 
popd

goto :EOF


:COMPILE_DLL
rem ============================================================

pushd build

rem del *.pdb > NUL 2> NUL
rem echo WAITING FOR PDB > lock.tmp

call cl ^
%MSVC_FLAGS% ^
%MSVC_SEARCH_DIRS% ^
F:\Dev\DroneController\src\%PROJECT_NAME%.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_memory.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_strings.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_ui.c ^
F:\Dev\DroneController\src\%PROJECT_NAME%_render_commands.c ^
-Fm%PROJECT_NAME%.map ^
-DLL ^
-LD ^
/link ^
-incremental:no ^
-PDB:%PROJECT_NAME%_%random%.pdb ^
-EXPORT:Update

rem del lock.tmp

popd
goto :EOF

pause
