import time
from ustruct import unpack, unpack_from
from array import array
from machine import Pin,UART
import time

 
class Genesys:
 
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
        self.readCommand("ADR %2d\r" % self.address)
        #print(self.value_read)
        # Set it remote
        self.readCommand("RMT 1\r")
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
        print("Debug genesys",self.value_read)
        if (self.value_read != None):
            self.value_read=self.value_read.decode("utf8")
    def ON(self):    
        self.readCommand("OUT 1\r")
        self.STATUS()
    def OFF(self):    
        self.readCommand("OUT 0\r")
        self.STATUS()
    def IDN(self):
        self.readCommand("IDN?\r")
        
    def STATUS(self):
        self.readCommand("STT?\r")
        
        ipv=self.value_read.find("PV(")+3
        self.vset=float(self.value_read[ipv:ipv+6])
        imv=self.value_read.find("MV(")+3
        self.vread=float(self.value_read[imv:imv+6])
        ipc=self.value_read.find("PC(")+3
        self.iset=float(self.value_read[ipc:ipc+6])
        imc=self.value_read.find("MC(")+3
        self.iread=float(self.value_read[imc:imc+6])
        self.readCommand("OUT?\r")
        self.status=self.value_read[:len(self.value_read)-1]

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
