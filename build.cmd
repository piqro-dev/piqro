@echo off
setlocal EnableDelayedExpansion

where /q clang.exe

if %errorlevel%==1 (
	echo Clang wasn't found. Install it, then ensure it is available in your PATH and try again.
	exit /b 1
)

for %%a in (%*) do set "%%a=true"

set common_flags=-std=gnu++2c ^
	-Isrc ^
	-ffast-math ^
	-fuse-ld=lld ^
	-Wno-c99-designator ^
	-Wno-undefined-internal ^
	-Wno-nan-infinity-disabled ^
	-nostdlib

set ui_windows_flags=%common_flags% ^
	-Xlinker -subsystem:console ^
	-O2 ^
	-DBUILD_UI

if "%ui%"=="true" (
	clang src/main.cpp -o bin/ui.exe %ui_windows_flags% 
)

if %errorlevel%==1 (
	exit /b 1
)