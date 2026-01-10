@echo off
setlocal EnableDelayedExpansion

where /q clang.exe

if %errorlevel%==1 (
	echo Clang wasn't found, install it, then ensure it is available in your PATH and try again.
	exit /b 1
)

for %%a in (%*) do set "%%a=true"

set common_flags=-std=gnu++2c ^
	-Isrc ^
	-ffast-math ^
	-ferror-limit=1000 ^
	-flto ^
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

set cli_flags=%common_flags% ^
	-Xlinker -subsystem:console ^
	-fuse-ld=lld

set cli_libs= ^
	-lmsvcrt ^
	-lucrt ^
	-lvcruntime ^
	-lkernel32

if "%cli%"=="true" (
	set flags=%cli_flags% %cli_libs%

	if "%release%"=="true" ( set flags=!flags! -Oz ) else ( set flags=!flags! -g3 )

	echo building cli...
	clang src/cli/main.cpp -o bin/index.exe !flags!

	goto exit
)

if "%wasm%"=="true" (
	set flags=%wasm_flags%

	if "%release%"=="true" ( set flags=!flags! -Oz ) else ( set flags=!flags! -g3 )

	echo building wasm...
	clang src/wasm/main.cpp -o bin/index.wasm !flags!

	goto exit
)

if "%1"=="" ( 
	echo usage: [%0] [targets...] [release]
	echo.
	echo possible targets:
	echo - cli
	echo - wasm
)

:exit
exit /b %errorlevel%