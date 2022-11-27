@echo off

taskkill /f /im main.exe
pushd build

if exist main.exe (start main.exe) else (echo ERROR: "main.exe" does not exist!!!)


popd
