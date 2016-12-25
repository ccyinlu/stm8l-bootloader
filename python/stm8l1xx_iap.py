'''
* filename:     bootloader.c
* description:  bootloader for low density stm8l1xx series
* author:       ethan
* change history:
*   time        version
* 2016-12-12    v1.0
'''
from mySerial import mySerial
import sys
import os

# choose one of the option below
USING_INTERRUPT_ADDRESS_REMAP = True
USING_INTERRUPT_ADDRESS_OVERLAPPED = False

USING_TRANSFER_VERIFICATION = False
USING_PROGRAM_VERIFICATION = False
if USING_TRANSFER_VERIFICATION:
    BOOT_TRANSFER_VERIFICATION_OK = 0xa0
    BOOT_TRANSFER_VERIFICATION_ERR = 0xa1

if USING_PROGRAM_VERIFICATION:
    BOOT_FLASH_VERIFICATION_OK = 0xa2
    BOOT_FLASH_VERIFICATION_ERR = 0xa3

BOOT_UNKNOWN_ERR = 0xa4
BOOT_HEAD = 0xa5
BOOT_READ = 0xa6
BOOT_WRITE = 0xa7
BOOT_GO = 0xa8
BOOT_OK = 0xa9
BOOT_HEAD_OK = 0xaa

BOOT_HEAD_MAIN = 0xab

PAGE_SIZE = 64
APP_INTERRUPT_SATRT_ADDRESS = 0x8280
if USING_INTERRUPT_ADDRESS_REMAP:
    APP_FLASH_PAGE_ID_START = 0x0c
else:
    APP_FLASH_PAGE_ID_START = 0x0a


arg_count = len(sys.argv)

serial_com = None
file_name = None

if arg_count == 1:
    print('missing the paramenter com port and file name')
    sys.exit(0)
elif arg_count == 2:
    print('missing the parameter file name')
    sys.exit(0)
else:
    serial_com = sys.argv[1]
    file_name = sys.argv[2]

print('serial com port: %s'%serial_com)
print('file name:       %s'%file_name)

# read and parse the file
file = open(file_name,'rb')
file_size = os.path.getsize(file_name)
print('file size:       %s bytes'%file_size)

try:
    # read all the data from the file
    data = file.read()
finally:
    file.close()

#check the data length by the file size
if not len(data) == file_size:
    print("data length error")

# you need to convert the bytes to bytearray because that the bytes can no be altered
data = bytearray(data)

# init the serial port
mySer = mySerial(57600,serial_com)

# open the serial port
mySer.open()

boot_head_ack = False

try:
    # transfer the data to the stm8 via the serial port
    # set the serial operation non-block
    mySer.ser.timeout = 0.5
    #1: send the head and wait for the ack (block operation)
    while boot_head_ack == False:
        mySer.write(BOOT_HEAD)

        #2: recv the head ack ok
        ch = mySer.read_nonblock()
        if ch == BOOT_HEAD_OK:
            boot_head_ack = True
            print('BOOT_HEAD_OK received')
        elif ch == -1:
            boot_head_ack = False
            # print("BOOT_HEAD ACK Timeout")
        else:
            print('error, BOOT_HEAD_OK not received, character: 0x%x'%ch)
            sys.exit(0)

    #2: handle the interrupt table
    bootloader_interrupt = bytearray(128)
    if USING_INTERRUPT_ADDRESS_REMAP:
        # prepare the remapped interrupt
        for i in range(0,32):
            bootloader_interrupt[i*4] = 0x82
            bootloader_interrupt[i*4 + 1] = 0x00
            bootloader_interrupt[i*4 + 2] = (APP_INTERRUPT_SATRT_ADDRESS + i*4) >> 8
            bootloader_interrupt[i*4 + 3] = (APP_INTERRUPT_SATRT_ADDRESS + i*4) & 0x00ff
    if USING_INTERRUPT_ADDRESS_OVERLAPPED:
        # prepare the overlapped interrupt
        for i in range(0,128):
            bootloader_interrupt[i] = data[i]

    # read the RST interrupt address from the bootloader
    for i in range(0, 4):
        mySer.write(BOOT_READ)
        mySer.write(i)
        bootloader_interrupt[i] = mySer.read_block()
    print("RESET Vector address in the file:    0x%02x%02x%02x%02x" % (data[0], data[1], data[2], data[3]))
    print("RESET Vector address in the chip:    0x%02x%02x%02x%02x" % (bootloader_interrupt[0], bootloader_interrupt[1], bootloader_interrupt[2], bootloader_interrupt[3]))
    print("reset vector address modified successful")

    # program the flash every page(64 bytes)
    # without transfer verification and flash program verification
    mySer.flashBlock_block(BOOT_WRITE, BOOT_OK, 0x00, PAGE_SIZE, 0x00, bootloader_interrupt)
    mySer.flashBlock_block(BOOT_WRITE, BOOT_OK, 0x01, PAGE_SIZE, 0x01, bootloader_interrupt)
    print("bootloader interrupt vector programmed successful")

    if USING_INTERRUPT_ADDRESS_OVERLAPPED:
        #3: program the app main address
        mySer.write(BOOT_HEAD_MAIN)
        for i in range(0,4):
            mySer.write(data[i])
        if not mySer.read_block() == BOOT_OK:
            print("program main address write fail")
            print("process ended")
            sys.exit(0)
        else:
            print("program main address write success")

    if USING_INTERRUPT_ADDRESS_REMAP:
        #4: program the app interrupt table
        mySer.flashBlock_block(BOOT_WRITE, BOOT_OK, APP_FLASH_PAGE_ID_START - 2, PAGE_SIZE, 0x00, data)
        mySer.flashBlock_block(BOOT_WRITE, BOOT_OK, APP_FLASH_PAGE_ID_START - 1, PAGE_SIZE, 0x01, data)

    #5: program the app program
    # calculate the pages of the data.bin
    pages_num = (int)(file_size/PAGE_SIZE) + 1
    for i in range(0,pages_num-2):
        mySer.flashBlock_block(BOOT_WRITE, BOOT_OK, i + APP_FLASH_PAGE_ID_START, PAGE_SIZE, i + 2, data)

    #6: goto the app
    mySer.write(BOOT_GO)

    '''
    #7: print what you have received
    while True:
        print("rece: 0x%02x"%mySer.read_block())
    '''
    # print("IAP finished, abort the process")
    # sys.exit(0)
    # just for test to see if the test.bin work
    while True:
        sys.stdout.write("%c"%mySer.read_block())
        sys.stdout.flush()

finally:
    mySer.close()
