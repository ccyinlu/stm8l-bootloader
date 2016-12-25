# reference for stm8lxx-bootloader
__further explanation and description for the customized bootloader__

## **TWO MODES: REMAP & OVERLAP**
#### the key point for a customized IAP is about the handling for interrupt vectors
* REMAP

|![remap mode][remap mode]|
|:------:|
|principle for remap mode interrupt vectors|

1. for detail information about the standard customized bootloader, go to read [AN2659][AN2659]
2. the first 128 bytes (`0x8000-0x8080`) contains 32 fix interrupt vectors
3. the first interrupt vector (`0x8000-0x8003`) is for the hardware reset interrupt and it should be kept unchanged
4. the last 31 interrupt vectors (`0x8004-0x8080`) should be redirected to `0x8284-0x82ff`, the value for `0x8004-0x8080` is calculated by `0x8200 << 16 & (0x8280 + n*4), n=1,2,...,31`
5. in file `lnkstm8l103k3.icf`, change `define region NearFunCode = [from 0x8280 to 0x9FFF]`, compile and get the app bin
6. memcyp the app bin including the interrupt vector table and the code to the flash from the address `0x8280`, the code actually starts from `0x8300`
7. when bootloader finished, the program will jump to the app `JP $8280`


* OVERLAP

|![overlap mode][overlap mode]|
|:------:|
|principle for overlap mode interrupt vectors|

1. the first 128 bytes (`0x8000-0x8080`) contains 32 fix interrupt vectors
2. the first interrupt vector (`0x8000-0x8003`) is for the hardware reset interrupt and it should be kept unchanged
3. the last 31 interrupt vectors (`0x8004-0x8080`) should be changed according to the app's 1-31th interrupt vectors
4. store the reset interrupt vector for app at the end of the user-bootloader `0x827c-0x827f`
5. in file `lnkstm8l103k3.icf`, change `define region NearFunCode = [from 0x8200 to 0x9FFF]`, compile and get the app bin
6. just memcyp the app bin code except the interrupt vector table to the flash from the address `0x8280`
7. when bootloader finished, the program will jump to the app `JP $827c`


* comparison between `REMAP` & `OVERLAP`
    1. `REMAP` mode is the standard mode provide by the official manual reference [AN2659][AN2659], so it is easy to understand and realize
    2. in `REMAP` mode, the content for the bootloader including both the `interrupt vectors` and `user-bootloader code` is constant, so you can define this area as `UBC` and design protection mechanism to prevent write operation during `IAP`;
    while in `OVERLAP` mode, as the last 31 interrupt vectors vary as the app code, it can not easily be protected
    3. compared to `OVERLAP` mode, you will waste `124` bytes storing the redirecting addresses, which is fatal when the `flash resource` is very limited
    4. compared to `OVERLAP` mode, you will waste at least a `CPU cycle` when handling the interrupt service. When an interrupt happens, the PC first jump to the fix interrupt vector, then jump to the redirected address, and then jump to the entry of the ISR, which is fatal when the interrupt service is very `time limited`.

## **TIPs**
1. when flashing the program, you can use `byte programming`, `fast byte programming`, `word programming`, `fast word programming`, `block programming` and `fast block programming`.  
the `time consumption` for `byte programming` and `block programming` scales greatly. For a binary file whose size is about 6K bytes, you may need minutes to finish programming using `byte programming`, at the meantime, 2-3 seconds using `block programming`  
When using `block programming`, you should map the code into the RAM, for example:
    ```c
    IN_RAM(void FLASH_ProgBlock(uint16_t StartAddress, uint8_t *Buffer));
    ```
for further information about the programming guide, refer to [AN2659][AN2659], [PM0047][PM0047], [UM0560][UM0560] and [SWIM][SWIM]

2. you should pay attention to the size of the bootloader.bin, in this example, we reserve 640bytes for bootloader. If you use more complicated algorithms such as `transfer verification`, `flash verification` or `CRC verification`, the size of the bootloader.bin will increase.  
When the space revered for the bootloader is not enough, fatal error will happen.

## **Q & A**
1. In the `OVERLAP` mode, the space address storing the app reset interrupt can be changed?  
>Absolutely yes!  
Actually, when the bootloader ends, it will jump to the space address storing the app reset interrupt vector which stores the entry for main function. You can even just fetch the entry for main function, and put it into the `jump space address`, this will save a jump cycle.  
In this example, we just choose the `jump space address` at the end of the `user-bootloader` `0x807c`. You can choose wherever you want just to avoid the occupation.
2. Can the interrupt enabled during the bootloader?  
>Generally not!  
But if you change the bootloader a little, it can!  
The reason why we should not enable interrupt during the bootloader is that we have damaged the interrupt vector, both in `REMAP` or `OVERLAP` mode after we rewrite the interrupt vector table.  
In `REMAP` mode, if you enable specified interrupt during bootloader, you can make a branch like this:
```c
if(booting){
    "excute the ISR for booting"
}else{
    "jump to the specified redirected address for app"
}
```
>`booting` is a global boolen variable indicating that whether the process now is in `boot` or `app`


[remap mode]:./img/remap_mode.jpg "remap mode"
[overlap mode]:./img/overlap_mode.jpg "overlap mode"
[AN2659]:./doc/AN2659_Stm8_IAP_using_a_customized_user_bootloader.pdf.pdf
[PM0047]:./doc/pm0047_programming_manual.pdf
[UM0560]:./doc/um0560-stm8_bootloader.pdf
[SWIM]:./doc/STM8-SWIM_communication_protocal_and_debug_mode.pdf
