@echo off
setlocal EnableDelayedExpansion

if !errorlevel!==0 (
	pushd bin
		call ui.exe
	popd
)