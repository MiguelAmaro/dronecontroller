@ECHO OFF

SET Debug_File= .\debug\FlightControl.rdbg
SET Executable= .\build\Win32_FlightControl.exe

CALL F:\Dev_Tools\RemedyBG\release_0.3.5.0\remedybg.exe %Debug_File%

PAUSE



