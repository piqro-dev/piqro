@echo off

if %errorlevel%==0 (
	pushd bin
		cli.exe
	popd
)