import time
from ustruct import unpack, unpack_from
from array import array
from machine import Pin,UART
import time

 
class Cpwplus:
 
    def __init__(self,uart_nb,tx_pin,rx_pin,baud,
                 **kwargs):
        self.uart = UART(uart_nb, baudrate=baud, tx=Pin(tx_pin), rx=Pin(rx_pin))
        self.uart.init(bits=8, parity=None, stop=1) 
        # Access the board
        self.value_read="NONE"
        #for i in range(1000):
        #    self.uart.write("ADR 1\r")
        #    time.sleep_ms(500)
        self.readCommand("N")
        #print(self.value_read)
        # Set it remote
        #print(self.value_read)
        
    def readCommand(self,cmd):
        """ Send a command and read back data 
 
           
            Returns:
                result[2]=[humidity,temperature]
        """
        # Write the command
        self.uart.write(cmd)
        time.sleep_ms(700)
        self.value_read=self.uart.readline()
        print("Debug cpwplus",self.value_read)
        if (self.value_read != None):
            self.value_read=self.value_read.decode("utf8")
    def ZERO(self):    
        self.readCommand("Z\r")
    def NET(self):    
        self.readCommand("N\r\n")
    def GROSS(self):
        self.readCommand("G\r")
        
    def TARE(self):
        self.readCommand("T\r")

    def status(self):
        rep={}
        self.NET()
        if (self.value_read == None):
            rep["net"]=-1
        else:
            #print(self.value_read)
            ideb=self.value_read.find("+")+1
            ifin=self.value_read.find("kg")-1
            rep["net"]=float(self.value_read[ideb:ifin])
        return rep
    def process_message(self,cmd):
        rep={}
        if (cmd=="ZERO"):
            self.ZERO()
            rep["zero"]=self.value_read

        if (cmd=="NET"):
            self.NET()
            rep["net"]=self.value_read
        if (cmd=="GROSS"):
            self.GROSS()
            rep["gross"]=self.value_read
        if (cmd=="TARE"):
            self.TARE()
            rep["tare"]=self.value_read
        if (cmd=="STATUS"):
            self.NET()
            if (self.value_read == None):
                rep["net"]=-1
            else:
                #print(self.value_read)
                ideb=self.value_read.find("+")+1
                ifin=self.value_read.find("kg")-1
                rep["net"]=float(self.value_read[ideb:ifin])
            
       
        return rep
