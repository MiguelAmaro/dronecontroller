@ECHO OFF
IF NOT EXIST build MKDIR build

SET Project_Name= FlightControl

REM ====================    PROJECT/FILES      ====================
SET Project_Name=FlightControl
SET Sources= ..\src\Win32_%Project_Name%.c ..\src\%Project_Name%_OpenGL.c ..\lib\glad\src\glad.c

REM ====================    COMPILER(MSVC)     ====================
SET Compiler_Common= -nologo
SET Warning= -wd4700
SET Compiler_Flags= %Compiler_Common% -Zi %Warning%  
SET Include_Directories= -I ..\lib\
SET Libraries= User32.lib Gdi32.lib Dinput8.lib Dxguid.lib Opengl32.lib Kernel32.lib Ws2_32.lib Shell32.lib ..\lib\bin\cglm.lib



ECHO ************************************************************
ECHO **********              START BUILD               **********
ECHO ************************************************************
PUSHD build

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
SET path=F:\Dev\FlightStick_Win32\build;%path%

ECHO ====================     WINDOWS          ====================
CALL cl %Compiler_Flags% %Include_Directories% %Sources% /link %Libraries% 

POPD
