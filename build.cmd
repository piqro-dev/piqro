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
	-fenable-matrix ^
	-ffast-math ^
	-fuse-ld=lld ^
	-Wno-c99-designator ^
	-Wno-undefined-internal ^
	-Wno-nan-infinity-disabled ^
	-nostdlib

set wasm_flags=%common_flags% ^
	-O3 ^
	--target=wasm32 ^
	-Xlinker --export-all ^
	-Xlinker --no-entry ^
	-Xlinker --allow-undefined ^
	-DBUILD_UI

if "%ui%"=="true" (
	echo building ui..
	clang src/main.cpp -o bin/index.wasm %wasm_flags% -O3
)

if %errorlevel%==1 (
	exit /b 1
)