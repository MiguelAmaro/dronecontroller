@echo off

taskkill /f /im win32_dc.exe
pushd build

if exist win32_dc.exe (start win32_dc.exe) else (echo ERROR: "win32_dc.exe" does not exist!!!)


popd
