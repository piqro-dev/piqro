clang main.cpp -c -o main.o -nostdlib --target=arm-none-eabi -mcpu=cortex-m0+ -mthumb -Oz
ld.lld --entry 0x20040001 -T boot_stage2.ld main.o -o main.elf
llvm-objdump -D main.elf > main.list
llvm-objcopy -O binary main.elf main.bin
picotool uf2 convert main.bin main.uf2 -o 0x20040000 --family rp2040
rm -rf main.elf main.list main.bin main.o
