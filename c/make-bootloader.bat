@echo off
iccstm8 ./bootloader/bootloader.c -e -Oh --code_model  small --data_model medium --vregs 16 -o ./bootloader/output/bootloader.o
ilinkstm8 --config_def _CSTACK_SIZE=0x100 --config_def _HEAP_SIZE=0x100 --config ./bootloader/lnkstm8l101k3.icf ./bootloader/output/bootloader.o -o ./bootloader/output/bootloader.out
ielftool --bin ./bootloader/output/bootloader.out ./bootloader/output/bootloader.bin
ielftool --ihex ./bootloader/output/bootloader.out ./bootloader/output/bootloader.hex
echo bootloader make and link successful!
pause