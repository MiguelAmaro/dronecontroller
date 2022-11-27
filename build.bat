@echo off
if not exist build mkdir build
rem// del *.pdb

set OUTPUT=%1

rem PROJECT/FILES
rem ============================================================
set PROJ_DIR= %cd%
set PROJ_NAME=main
set SOURCES=%PROJ_DIR%/src/%PROJ_NAME%.c

rem MSVC FLAGS
rem ============================================================
set WARNINGS= -wd4700
set MSVC_FLAGS= %WARNINGS% -std:c11 -nologo -Zi ^
-I ..\lib\ -I ..\lib\freetype2\include\ -I%PROJ_DIR%\src\
set LIBRARIES= user32.lib gdi32.lib dinput8.lib opengl32.lib kernel32.lib ws2_32.lib shell32.lib ^
dxguid.lib d3d11.lib dxgi.lib d3dcompiler.lib "..\lib\freetype2\release dll\win64\freetype.lib"

rem START BUILD
rem ============================================================
set path= %PROJ_DIR%\build\;%path%
pushd build
if "%OUTPUT%" equ "-exe" (goto :COMPILE_EXE)
if "%OUTPUT%" equ "-dll" (goto :COMPILE_DLL)
goto :COMPILE_EXE rem !!!DEFAULT PATH!!!

:COMPILE_EXE
rem ============================================================
taskkill /f /im %PROJ_NAME%.exe
call cl %MSVC_FLAGS%  %SOURCES% /link -subsystem:windows -incremental:no %LIBRARIES% 
goto :eof

:COMPILE_DLL
rem ============================================================
rem del *.pdb > NUL 2> NUL
rem echo WAITING FOR PDB > lock.tmp
call cl %MSVC_FLAGS% -Fm%PROJ_NAME%.map -DLL -LD /link -incremental:no %LIBRARIES% -PDB:%PROJ_NAME%_%random%.pdb -EXPORT:Update
rem del lock.tmp
goto :eof

:eof
popd

pause
