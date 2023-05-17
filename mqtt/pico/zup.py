import time
from ustruct import unpack, unpack_from
from array import array
from machine import Pin,UART
import time

 
class Zup:
 
    def __init__(self,uart_nb,tx_pin,rx_pin,address,
                 **kwargs):
        self.address = address
        self.uart = UART(uart_nb, baudrate=9600, tx=Pin(tx_pin), rx=Pin(rx_pin))
        self.uart.init(bits=8, parity=None, stop=1) 
        # Access the board
        self.value_read="NONE"
        #for i in range(1000):
        #    self.uart.write("ADR 1\r")
        #    time.sleep_ms(500)
        self.readCommand(":ADR %2d;\r" % self.address)
        #print(self.value_read)
        
    def readCommand(self,cmd):
        """ Send a command and read back data 
 
           
            Returns:
                result[2]=[humidity,temperature]
        """
        # Write the command
        self.uart.write(cmd)
        time.sleep_ms(500)
        self.value_read=self.uart.readline()
        print("Debug zup",self.value_read)
        if (self.value_read != None):
            self.value_read=self.value_read.decode("utf8")
    def ON(self):    
        self.readCommand(":OUT1;")
        self.STATUS()
    def OFF(self):    
        self.readCommand(":OUT0;")
        self.STATUS()
    def IDN(self):
        self.readCommand(":IDN?;\r")
        
    def STATUS(self):
        self.readCommand(":STT?;\r")
        
        ipv=self.value_read.find("SV")+2
        self.vset=float(self.value_read[ipv:ipv+5])
        imv=self.value_read.find("AV")+2
        self.vread=float(self.value_read[imv:imv+5])
        ipc=self.value_read.find("SA")+2
        self.iset=float(self.value_read[ipc:ipc+5])
        imc=self.value_read.find("AA")+2
        self.iread=float(self.value_read[imc:imc+5])
        ist=self.value_read.find("OS")+2
        self.status=int(self.value_read[ist:ist+8],2)
    def process_message(self,msg):
        cmd=msg["command"]
        if (cmd=="ON"):
            self.ON()
        if (cmd=="OFF"):
            self.OFF()
        if (cmd=="STATUS"):
            self.STATUS()
        rep={}
        rep["vset"]=self.vset
        rep["vout"]=self.vread
        rep["iset"]=self.iset
        rep["iout"]=self.iread
        rep["status"]=self.status
        return rep
