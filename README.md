# stm8l-bootloader
bootloader of IAP(In-Application Programming) for ultra-low power stm8lxx series  
Two modes for booting is provided: `REMAP` and `OVERLAP`  
Any problems or questions, welcome contacting me via `ccyinlu@gmail.com`

## Usage
### Step I **environment preparation**
* get the compile tool `iccstm8`, link tool `ilinkstm8` and converter tool `ielftool`
    * install [IAR IDE][IAR IDE] for stm8
    * add the tool path `C:\Program Files (x86)\IAR Systems\Embedded Workbench 6.0 Evaluation\stm8\bin` to the environment variable `PATH` ( if you change your installation settings, change the tool path accordingly)
* get the hardware [st-link][ST-LINK] to flash the program
    * install the [driver][ST-LINK Driver] for st-link
* use [STVP][STVP] or [IAR IDE][IAR IDE] to flash the program
* install python3 (you may be confronted with compatibility problems using python2)
    * install [pySerial][pySerial] module
    ```bash
    cd ./python/pySerial/pyserial-master
    sudo python3 setup.py install
    ```

### Step II **configuration**
* you have two mode to choose: `REMAP` or `OVERLAP`(the mode will be explained further in [reference](./REFERENCE.md))
    * **`REMAP`** mode
    ```
    file ./c/bootloader/bootloader.h
    // choose one of the option below
    #define   INTERRUPT_ADDRESS_REMAP
    //#define   INTERRUPT_ADDRESS_OVERLAPPED

    file ./python/stm8lxx_iap.py
    # choose one of the option below
    USING_INTERRUPT_ADDRESS_REMAP = True
    USING_INTERRUPT_ADDRESS_OVERLAPPED = False

    file ./c/app/lnkstm8l103k3.icf
    define region NearFuncCode = [from 0x8280 to 0x9FFF];
    define region FarFuncCode = [from 0x8280 to 0x9FFF];
    define region HugeFuncCode = [from 0x8280 to 0x9FFF];
    ```
    * **`OVERLAP`** mode
    ```
    file ./c/bootloader/bootloader.h
    // choose one of the option below
    //#define   INTERRUPT_ADDRESS_REMAP
    #define   INTERRUPT_ADDRESS_OVERLAPPED

    file ./python/stm8lxx_iap.py
    # choose one of the option below
    USING_INTERRUPT_ADDRESS_REMAP = False
    USING_INTERRUPT_ADDRESS_OVERLAPPED = True

    file ./c/app/lnkstm8l103k3.icf
    define region NearFuncCode = [from 0x8200 to 0x9FFF];
    define region FarFuncCode = [from 0x8200 to 0x9FFF];
    define region HugeFuncCode = [from 0x8200 to 0x9FFF];
    ```
* you can specify the address of the program storing the app reset interrupt address in `OVERLAP` mode (you should avoid the address both for the bootloader and app), using 0x827c as an example:
```
file ./c/bootloader/bootloader.c
#ifdef INTERRUPT_ADDRESS_OVERLAPPED
          asm("JP $827c");
#endif
#ifdef INTERRUPT_ADDRESS_REMAP
          asm("JP $8280");
#endif
file ./c/bootloader/bootloader.h
#define   FLASH_APP_MAIN        0x827c
```

### Step III **build bootloader & app**
```
run ./c/make-bootloader.bat  
you will find bootloader.bin and bootloader.hex under ./c/bootloader/output  
run ./c/make-app.bat
you will find test.bin under ./c/app/output
```

> for you own app binary file, use the compile tool or IDE to get the binary file, replace the test.bin

### Step IV **burn bootloader & flash app**
* using STVP or IAR IDE to burn bootloader.bin to stm8lxx chip
* run the python script to flash app bin to stm8lxx chip
    * use `/dev/ttyS2` as the example for UART (`com*` in windows system) and `test.bin` for app binary file
    ```
    cd ./python
    sudo python3 stm8lxx_iap.py /dev/ttyS2 ../c/app/output/test.bin
    ```
    * reset stm8lxx chip
* here is the example of the process:
```
ccyinlu@orangepipcplus:~/Git/stm8l-bootloader/python$ sudo python stm8l1xx_iap.py /dev/ttyS2 ../c/app/output/test.bin
serial com port: /dev/ttyS2
file name:       ../c/app/output/test.bin
file size:       351 bytes
BOOT_HEAD_OK received
RESET Vector address in the file:    0x820083b9
RESET Vector address in the chip:    0x8200819d
reset vector address modified successful
flash page 0 write successful
flash page 1 write successful
bootloader interrupt vector programmed successful
flash page 10 write successful
flash page 11 write successful
flash page 12 write successful
flash page 13 write successful
flash page 14 write successful
flash page 15 write successful
hello world
```


## **What's more?**
Want more information of customized bootloader and tips for this project?  
Go to read the [reference](./REFERENCE.md)

## Something related
Thanks for the work [hairBoot][hairboot] of [Zepan][Zepan].

------------------------------------------

[IAR IDE]:http://pan.baidu.com/s/1boDIRwF "download link of IAR IDE for stm8"
[ST-LINK]:https://detail.tmall.com/item.htm?spm=a230r.1.14.6.qu72qT&id=14634569468&cm_id=140105335569ed55e27b&abbucket=8 "example link for ST-LINK"
[ST-LINK Driver]:http://pan.baidu.com/s/1qXBq3vu "driver for st-link "
[STVP]:http://pan.baidu.com/s/1nuBwAxV "download link of STVP"
[pySerial]:/python/pySerial/pyserial-master/ "path of the pySerial module"
[hairboot]:https://github.com/Zepan/hairBoot "hairboot"
[Zepan]:https://github.com/Zepan "github homepage for Zepan"