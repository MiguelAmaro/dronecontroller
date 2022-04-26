@echo off

set OUTPUT=%1

if not exist build mkdir build

rem ====================    PROJECT/FILES      ====================
set PROJECT_DIR= %cd%
set PROJECT_NAME=dc
set SOURCES=^
%PROJECT_DIR%\src\win32_%PROJECT_NAME%.c ^
%PROJECT_DIR%\src\win32_d3d11.c ^
%PROJECT_DIR%\src\win32_opengl.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_opengl.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_memory.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_strings.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_render_commands.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_ui.c ^
%PROJECT_DIR%\lib\glad\src\glad.c

rem ====================    COMPILER(MSVC)     ====================
set MSVC_WARNINGS= -wd4700

set MSVC_FLAGS= %MSVC_WARNINGS% -std:c11 -nologo -Zi -W3 -D_CRT_SECURE_NO_WARNINGS

set MSVC_SEARCH_DIRS= ^
-I ..\lib\ ^
-I ..\lib\freetype2\include\

rem ====================     LINKER(MSVC)       ====================
set MSVC_LIBS= ^
user32.lib ^
gdi32.lib ^
dinput8.lib ^
opengl32.lib ^
kernel32.lib ^
ws2_32.lib ^
shell32.lib ^
dxguid.lib ^
d3d11.lib ^
dxgi.lib ^
d3dcompiler.lib ^
"..\lib\freetype2\release dll\win64\freetype.lib"

rem ************************************************************
rem **********              START BUILD               **********
rem ************************************************************
set path=%path%;%PROJECT_DIR%\build\
if "%OUTPUT%" equ "-exe" (goto :COMPILE_EXE)
if "%OUTPUT%" equ "-dll" (goto :COMPILE_DLL)
goto :COMPILE_EXE rem !!!DEFAULT PATH!!!


rem ====================     WINDOWS          ====================
:COMPILE_EXE
taskkill /f /im win32_dc.exe

pushd build
call cl %MSVC_FLAGS% %MSVC_SEARCH_DIRS% %SOURCES% /link -subsystem:windows -incremental:no %MSVC_LIBS% 
popd

goto :eof


:COMPILE_DLL
rem ============================================================
pushd build

rem del *.pdb > NUL 2> NUL
rem echo WAITING FOR PDB > lock.tmp

call cl ^
%MSVC_FLAGS% ^
%MSVC_SEARCH_DIRS% ^
%PROJECT_DIR%\src\%PROJECT_NAME%.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_memory.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_strings.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_ui.c ^
%PROJECT_DIR%\src\%PROJECT_NAME%_render_commands.c ^
-Fm%PROJECT_NAME%.map ^
-DLL ^
-LD ^
/link ^
-incremental:no ^
%MSVC_LIBS% ^
-PDB:%PROJECT_NAME%_%random%.pdb ^
-EXPORT:Update

rem del lock.tmp

popd
goto :eof

:eof

pause
