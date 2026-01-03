@echo off

if %errorlevel%==0 (
	pushd bin
		index.exe
	popd
)