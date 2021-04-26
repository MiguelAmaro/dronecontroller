@ECHO OFF
IF NOT EXIST build MKDIR build

REM ====================    PROJECT/FILES      ====================
SET Project_Name=fc
SET Sources= ..\src\win32_%Project_Name%.c ..\src\%Project_Name%_opengl.c ..\lib\glad\src\glad.c

REM ====================    COMPILER(MSVC)     ====================
SET Compiler_Common= -nologo

SET Warning= -wd4700

SET Compiler_Flags= %Compiler_Common% -Zi %Warning%  

SET Include_Directories= ^
-I ..\lib\ ^
-I ..\lib\freetype2\include\

SET Libraries= ^
User32.lib ^
Gdi32.lib ^
Dinput8.lib ^
Dxguid.lib ^
Opengl32.lib ^
Kernel32.lib ^
Ws2_32.lib ^
Shell32.lib ^
..\lib\bin\cglm.lib ^
..\lib\freetype2\dlls\win64\freetype.lib



REM ************************************************************
REM **********              START BUILD               **********
REM ************************************************************
PUSHD build

SET PATH=%PATH%;F:\Dev\FlightControl\build

ECHO ====================     WINDOWS          ====================
CALL cl %Compiler_Flags% %Include_Directories% %Sources% /link %Libraries% 

POPD

PAUSE
