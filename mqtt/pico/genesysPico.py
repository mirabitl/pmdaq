import time
from ustruct import unpack, unpack_from
from array import array
from machine import Pin,UART
import time
from genesysInterface import abstractGenesys as gI
 
class genesysPico(gI):
 
    def __init__(self,uart_nb,tx_pin,rx_pin,address,baud=9600,
                 **kwargs):
        self.address = address
        self.uart = UART(uart_nb, baudrate=baud, tx=Pin(tx_pin), rx=Pin(rx_pin))
        print(uart_nb,baud,tx_pin,rx_pin,address)
        self.uart.init(bits=8, parity=None, stop=1) 
        # Access the board
        self.value_read="NONE"
        self.cb={}
        self.cb["SETADDRESS"]=self.setAddress
        self.cb["SETREMOTE"]=self.setRemote
        self.cb["SETVOLTAGE"]=self.setVoltage
        self.cb["SETCURRENT"]=self.setCurrent
        self.cb["CLEAR"]=self.clear
        self.cb["RESET"]=self.reset
        self.cb["SETON"]=self.setOn
        self.cb["SETOFF"]=self.setOff
        self.cb["STATUS"]=self.status
        
        
        gI.__init__(self,address)
        self.setRemote(True)
    def write(self,s):
        self.uart.write(s+"\r")
        time.sleep_ms(500)
        #print("debug ",self.uart.readline())
        
    def readline(self):
        #buf=bytes(1024)
        self.value_read=self.uart.readline()
        #self.value_read=buf
        #print("Debug genesys",buf)
        if (self.value_read != None):
            self.value_read=self.value_read.decode("utf8")
        sgood=self.value_read.split('\r')
        print(sgood)
        if (len(sgood)>1):
            return(sgood[1])
        else:
            return self.value_read

        return self.value_read
    def process_message(self,msg):
        rep={}
        cmd=msg["command"]
        params=None
        if "params" in msg:
            params=msg["params"]
        if not cmd in self.cb.keys():
            print("unknown command",cmd)
            return rep
        if params == None:
            return self.cb[cmd]()
        else:
            return self.cb[cmd](params)
        return rep
