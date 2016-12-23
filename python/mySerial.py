import serial
import sys

class mySerial:
    def __init__(self):
        self.ser = serial.Serial()
        self.ser.baudrate = 115200
        self.ser.port = 'com1'
        # timeout parameter, unit seconds(s), can be float and small than one
        self.ser.timeout = 1
        self.progBlockTimeOut = 1
        self.isOpen = False

    def __init__(self, baudrate, port):
        self.ser = serial.Serial()
        self.ser.baudrate = baudrate
        self.ser.port = port
        # timeout parameter, unit seconds(s), can be float and small than one
        self.ser.timeout = 1
        self.progBlockTimeOut = 1
        self.isOpen = False

    def open(self):
        if self.isOpen:
            print("%s is already opend"%self.ser.port)
        else:
            if not self.ser.isOpen():
                self.ser.open()
            self.isOpen = True

    def close(self):
        if self.isOpen:
            self.ser.close()
            self.isOpen = False
        else:
            print("%s is already closed"%self.ser.port)

    # non-block read
    def read_nonblock(self,size=1):
        if self.isOpen:
            if size == 1:
                ch = self.ser.read()
                if len(ch) == 1:
                    return ch[0]
                elif len(ch) == 0:
                    return -1
                else:
                    print("receive error, abort the process")
                    sys.exit(0)
            else:
                return self.ser.read(size)
        else:
            print("%s is not opened, go to open it first"%self.ser.port)
            sys.exit(0)

    def read_block(self,size = 1):
        if size == 1:
            flag = True
            while flag:
                ch = self.read_nonblock()
                if ch == -1:
                    flag = True
                else:
                    flag = False
            return ch
        else:
            self.read_nonblock(size)


    def write(self,buffer,size=1):
        if self.isOpen:
            if isinstance(buffer,bytes):
                data = buffer[1:size]
            else:
                data = [buffer]

            self.ser.write(data)
        else:
            print("%s is not opened, go to open it first" % self.ser.port)
            sys.exit(0)

    def flashBlock_nonblock(self,write_tag, ok_tag, flash_page_id, flash_page_size, data_page_id, data):
        # program the flash every page(64 bytes)
        # without transfer verification and flash program verification
        # 1> send the BOOT_WRITE byte
        # 2> send the page id
        # 3> send the 64 bytes of the block data
        # 4> recv the BOOT_OK
        data_size = len(data)
        self.write(write_tag)
        self.write(flash_page_id)
        for i in range(data_page_id * flash_page_size, (data_page_id + 1) * flash_page_size):
            if i < data_size:
                self.write(data[i])
            else:
                self.write(0)
        retryCounts = (int)(self.progBlockTimeOut/self.ser.timeout)
        i = 0
        while i<retryCounts:
            ch = self.read_nonblock()
            if ch == ok_tag:
                print("flash page %d write successful" % flash_page_id)
                return
            elif ch == -1:
                print("flash page %d write ack timeout" % flash_page_id)
                i = i + 1
            else:
                print("flash page %d write failed" % flash_page_id)
                print("process ended")
                sys.exit(0)
        print("flash block write timeout, retry: %d times, abort the process"%i)
        sys.exit(0)

    def flashBlock_block(self, write_tag, ok_tag, flash_page_id, flash_page_size, data_page_id, data):
        # program the flash every page(64 bytes)
        # without transfer verification and flash program verification
        # 1> send the BOOT_WRITE byte
        # 2> send the page id
        # 3> send the 64 bytes of the block data
        # 4> recv the BOOT_OK
        data_size = len(data)
        self.write(write_tag)
        self.write(flash_page_id)
        for i in range(data_page_id * flash_page_size, (data_page_id + 1) * flash_page_size):
            if i < data_size:
                self.write(data[i])
            else:
                self.write(0)

        ch = self.read_nonblock()
        if ch == ok_tag:
            print("flash page %d write successful" % flash_page_id)
            return
        else:
            print("flash page %d write failed" % flash_page_id)
            print("process ended")
            sys.exit(0)

