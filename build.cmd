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
	-Wno-c99-designator ^
	-Wno-undefined-internal ^
	-Wno-nan-infinity-disabled ^
	-nostdlib

set wasm_flags=%common_flags% ^
	--target=wasm32 ^
	-Xlinker --export-all ^
	-Xlinker --no-entry ^
	-Xlinker --allow-undefined

set win32_flags=%common_flags% ^
	-Xlinker -subsystem:console ^
	-fuse-ld=lld ^
	-O3 -ftime-trace

set win32_libs= ^
	-lmsvcrt ^
	-lucrt ^
	-lvcruntime ^
	-lkernel32

if "%win32%"=="true" (
	echo building win32...
	clang src/main.cpp -o bin/index.exe %win32_flags% %win32_libs%
) else if "%wasm%"=="true" (
	echo building wasm...
	clang src/main.cpp -o bin/index.wasm %wasm_flags%   
) else (
	if %1 neq "" (
		echo '%1' isn't a correct target
	) else (
		echo yo I need you to pass something to build
	)
	echo.
	echo possible targets:
	echo - wasm
	echo - win32
)

if %errorlevel%==1 (
	exit /b 1
)