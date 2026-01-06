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
	-ferror-limit=100 ^
	-Wno-undefined-internal ^
	-Wno-c99-designator ^
	-Wno-nan-infinity-disabled ^
	-Wno-tautological-compare ^
	-nostdlib

set wasm_flags=%common_flags% ^
	--target=wasm32 ^
	-Xlinker --export-all ^
	-Xlinker --no-entry ^
	-Xlinker --allow-undefined

set win32_flags=%common_flags% ^
	-Xlinker -subsystem:console ^
	-fuse-ld=lld

set win32_libs= ^
	-lmsvcrt ^
	-lucrt ^
	-lvcruntime ^
	-lkernel32

if "%win32%"=="true" (
	set flags=%win32_flags% %win32_libs%

	if "%release%"=="true" ( set flags=!flags! -Oz ) else ( set flags=!flags! -g3 )

	echo Building win32...
	clang src/main.cpp -o bin/index.exe !flags!

	goto exit
)

if "%wasm%"=="true" (
	set flags=%wasm_flags%

	if "%release%"=="true" ( set flags=!flags! -Oz ) else ( set flags=!flags! -g3 )

	echo Building wasm...
	clang src/main.cpp -o bin/index.wasm !flags!

	goto exit
)

if "%1"=="" ( 
	echo Usage: [%0] [targets...] [release]
	echo.
	echo Possible targets:
	echo - wasm
	echo - win32 
)

:exit
exit /b %errorlevel%