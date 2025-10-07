@setlocal EnableDelayedExpansion
@echo off

set flash_size=0x200000

set ram_size=0x40000

set flags=^
-D USE_PICO=1 ^
-D USE_USB_STDIO=1 ^
-D RP2040=1 ^
-D FLASHSIZE=%flash_size% ^
-D RAMSIZE=%ram_size% ^
-D USE_FLOATLIBC=0 ^
-D USE_DOUBLELIBC=0 ^
-D _STDIO_H_ ^
-I sdk ^
-I sdk/_sdk ^
-march=armv6-m ^
-mcpu=cortex-m0plus ^
-mthumb ^
-ffunction-sections ^
-fdata-sections  ^
-funsigned-char ^
-nostartfiles ^
--specs=nosys.specs ^
-Os

set c_flags=%flags% -std=gnu11

set cxx_flags=%flags% -std=gnu++2c -fno-use-cxa-atexit

set ld_flags=^
-Wl,--gc-sections ^
-Wl,--build-id=none ^
-Wl,--defsym=FLASHSIZE=%flash_size% ^
-Wl,--defsym=RAMSIZE=%ram_size% ^
-Wl,--wrap=printf ^
-Wl,--wrap=vprintf ^
-Wl,--wrap=sprintf ^
-Wl,--wrap=snprintf ^
-Wl,--wrap=vsnprintf ^
-Wl,--wrap=vsprintf ^
-Wl,--wrap=puts ^
-Wl,--wrap=putchar ^
-Wl,--wrap=getchar ^
-Wl,--wrap=__aeabi_dcmpun ^
-Wl,--wrap=copysign ^
-Wl,--wrap=__aeabi_dadd ^
-Wl,--wrap=__aeabi_dsub ^
-Wl,--wrap=__aeabi_drsub ^
-Wl,--wrap=__aeabi_dmul ^
-Wl,--wrap=__aeabi_ddiv ^
-Wl,--wrap=__aeabi_cdrcmple ^
-Wl,--wrap=__aeabi_cdcmple ^
-Wl,--wrap=__aeabi_cdcmpeq ^
-Wl,--wrap=__aeabi_dcmpeq ^
-Wl,--wrap=__aeabi_dcmplt ^
-Wl,--wrap=__aeabi_dcmple ^
-Wl,--wrap=__aeabi_dcmpge ^
-Wl,--wrap=__aeabi_dcmpgt ^
-Wl,--wrap=__aeabi_ui2d ^
-Wl,--wrap=__aeabi_i2d ^
-Wl,--wrap=__aeabi_ul2d ^
-Wl,--wrap=__aeabi_l2d ^
-Wl,--wrap=__aeabi_d2iz ^
-Wl,--wrap=__aeabi_d2uiz ^
-Wl,--wrap=__aeabi_d2lz ^
-Wl,--wrap=__aeabi_d2ulz ^
-Wl,--wrap=sqrt ^
-Wl,--wrap=sin ^
-Wl,--wrap=cos ^
-Wl,--wrap=sincos ^
-Wl,--wrap=tan ^
-Wl,--wrap=atan2 ^
-Wl,--wrap=exp ^
-Wl,--wrap=log ^
-Wl,--wrap=__aeabi_d2f ^
-Wl,--wrap=__aeabi_fcmpun ^
-Wl,--wrap=copysignf ^
-Wl,--wrap=__aeabi_fadd ^
-Wl,--wrap=__aeabi_fsub ^
-Wl,--wrap=__aeabi_frsub ^
-Wl,--wrap=__aeabi_fmul ^
-Wl,--wrap=__aeabi_fdiv ^
-Wl,--wrap=__aeabi_cfrcmple ^
-Wl,--wrap=__aeabi_cfcmple ^
-Wl,--wrap=__aeabi_cfcmpeq ^
-Wl,--wrap=__aeabi_fcmpeq ^
-Wl,--wrap=__aeabi_fcmplt ^
-Wl,--wrap=__aeabi_fcmple ^
-Wl,--wrap=__aeabi_fcmpge ^
-Wl,--wrap=__aeabi_fcmpgt ^
-Wl,--wrap=__aeabi_ui2f ^
-Wl,--wrap=__aeabi_i2f ^
-Wl,--wrap=__aeabi_ul2f ^
-Wl,--wrap=__aeabi_l2f ^
-Wl,--wrap=__aeabi_f2iz ^
-Wl,--wrap=__aeabi_f2uiz ^
-Wl,--wrap=__aeabi_f2lz ^
-Wl,--wrap=__aeabi_f2ulz ^
-Wl,--wrap=sqrtf ^
-Wl,--wrap=sinf ^
-Wl,--wrap=cosf ^
-Wl,--wrap=sincosf ^
-Wl,--wrap=tanf ^
-Wl,--wrap=atan2f ^
-Wl,--wrap=expf ^
-Wl,--wrap=logf ^
-Wl,--wrap=__aeabi_f2d ^
-Wl,--wrap=memset ^
-Wl,--wrap=__aeabi_memset ^
-Wl,--wrap=__aeabi_memset4 ^
-Wl,--wrap=__aeabi_memset8 ^
-Wl,--wrap=memcpy ^
-Wl,--wrap=__aeabi_memcpy ^
-Wl,--wrap=__aeabi_memcpy4 ^
-Wl,--wrap=__aeabi_memcpy8 ^
-Wl,--wrap=tanh ^
-Wl,--wrap=asinh ^
-Wl,--wrap=acosh ^
-Wl,--wrap=atanh ^
-Wl,--wrap=exp2 ^
-Wl,--wrap=log2 ^
-Wl,--wrap=exp10 ^
-Wl,--wrap=log10 ^
-Wl,--wrap=expm1 ^
-Wl,--wrap=log1p ^
-Wl,--wrap=fma ^
-Wl,--wrap=powint ^
-Wl,--wrap=pow ^
-Wl,--wrap=__aeabi_idiv ^
-Wl,--wrap=__aeabi_idivmod ^
-Wl,--wrap=__aeabi_uidiv ^
-Wl,--wrap=__aeabi_uidivmod ^
-Wl,--wrap=__aeabi_ldiv ^
-Wl,--wrap=__aeabi_ldivmod ^
-Wl,--wrap=__aeabi_uldiv ^
-Wl,--wrap=__aeabi_uldivmod ^
-Wl,--wrap=__aeabi_lmul ^
-Wl,-script=sdk\memmap_rp2040_default.ld

set sdk_src=^
sdk\_sdk\crt0_rp2040.S ^
sdk\_sdk\src\sdk_divider.S ^
sdk\_sdk\src\sdk_memops_asm.S ^
sdk\_sdk\src\sdk_double_m0.S ^
sdk\_sdk\src\sdk_float_m0.S ^
sdk\_boot2\boot2_w25q080_RP2040_bin.S ^
sdk\sdk_all.c

set src=main.cpp

for %%f in (%sdk_src%) do set objs=!objs! bin\%%~nf.o
for %%f in (%src%) do set objs=!objs! bin\%%~nf.o

set out=bin\piqro

if not exist bin mkdir bin

for %%a in (%*) do set "%%a=1"

if [%sdk%]==[1] (
	for %%f in (%sdk_src%) do (
		echo   CC %%f
		arm-none-eabi-gcc %%f %c_flags% -c -o bin\%%~nf.o
	)
)

if [%piqro%]==[1] (
	for %%f in (%src%) do (
		if [%%~xf]==[.cpp] (
			echo   CXX %%f
			arm-none-eabi-g++ %%f %cxx_flags% -c -o bin\%%~nf.o
		)
		if [%%~xf]==[.c] (
			echo   CC %%f
			arm-none-eabi-gcc %%f %c_flags% -c -o bin\%%~nf.o
		)
	)
)

arm-none-eabi-gcc %flags% %ld_flags% %objs% -o %out%.elf

arm-none-eabi-objdump -D %out%.elf > %out%.list
arm-none-eabi-objcopy -O binary %out%.elf %out%.bin
picotool uf2 convert %out%.bin %out%.uf2 --family rp2040
del %out%.elf %out%.list %out%.bin