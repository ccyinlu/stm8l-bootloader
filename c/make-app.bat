@echo off
iccstm8 ./app/test.c -e -Oh --code_model  small --data_model medium --vregs 16 -o ./app/output/test.o
ilinkstm8 --config_def _CSTACK_SIZE=0x100 --config_def _HEAP_SIZE=0x100 --config ./app/lnkstm8l101k3.icf ./app/output/test.o -o ./app/output/test.out
ielftool --bin ./app/output/test.out ./app/output/test.bin
ielftool --ihex ./app/output/test.out ./app/output/test.hex
echo app make and link successful!
pause