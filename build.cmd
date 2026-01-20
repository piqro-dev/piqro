@echo off
setlocal EnableDelayedExpansion

where /q clang.exe

if %errorlevel%==1 (
	echo Clang wasn't found, install it, then ensure it is available in your PATH and try again.
	exit /b 1
)

for %%a in (%*) do set "%%a=true"

set common_flags=-std=c23 ^
	-Isrc ^
	-ferror-limit=1000 ^
	-flto ^
	-Wno-undefined-internal ^
	-Wno-c99-designator ^
	-Wno-nan-infinity-disabled ^
	-Wno-tautological-compare ^
	-nostdlib

set cli_flags=%common_flags% ^
	-Xlinker -subsystem:console ^
	-fuse-ld=lld ^
	-D_CRT_SECURE_NO_WARNINGS

set cli_libs= ^
	-lmsvcrt ^
	-lucrt ^
	-lvcruntime ^
	-lkernel32

set web_flags=%common_flags% ^
	-Wno-incompatible-library-redeclaration ^
	--target=wasm32 ^
	-Xlinker --export-all ^
	-Xlinker --no-entry ^
	-Xlinker --allow-undefined

if "%cli%"=="true" (
	set flags=%cli_flags% %cli_libs%

	if "%release%"=="true" ( set flags=!flags! -Oz ) else ( set flags=!flags! -g3 )

	echo building cli...
	clang src/cli/main.c -o bin/cli.exe !flags!

	goto exit
)

if "%web%"=="true" (
	set flags=%web_flags% -Oz 

	echo building web...
	clang src/web/main.c -o bin/piqro.wasm !flags!

	goto exit
)

if "%1"=="" ( 
	echo usage: [%0] [targets...] [release]
	echo.
	echo possible targets:
	echo - cli
	echo - web
)

:exit
exit /b %errorlevel%