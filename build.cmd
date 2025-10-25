@echo off

if [%1]==[install] (
	if not exist bin mkdir bin

	pushd bin
		
	if not exist pico-sdk      git clone https://github.com/raspberrypi/pico-sdk --depth=1 --recursive 
	if not exist picovga-cmake git clone https://github.com/codaris/picovga-cmake --depth=1 --recursive
		
	cmake .. -G Ninja && ninja

	move compile_commands.json ../compile_commands.json

	popd
) 

if [%1]==[piqro] (
	pushd bin

	ninja
	move compile_commands.json ../compile_commands.json
	
	popd
)

if [%1]==[] (
	echo usage: build.cmd ^<target^>
	echo.
	echo targets: 
	echo.
	echo - install
	echo - piqro
)