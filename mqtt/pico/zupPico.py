import time
from ustruct import unpack, unpack_from
from array import array
from machine import Pin,UART
import time
from zupInterface import abstractZup as zI
 
class zupPico(zI):
 
    def __init__(self,uart_nb,tx_pin,rx_pin,address,baud=9600,
                 **kwargs):
        self.address = address
        self.uart = UART(uart_nb, baudrate=baud, tx=Pin(tx_pin), rx=Pin(rx_pin))
        self.uart.init(bits=8, parity=None, stop=1) 
        # Access the board
        self.value_read="NONE"
        self.cb={}
        self.cb["SETADDRESS"]=self.setAddress
        self.cb["SETREMOTE"]=self.setRemote
        self.cb["SETVOLTAGE"]=self.setVoltage
        self.cb["SETCURRENT"]=self.setCurrent
        self.cb["SETON"]=self.setOn
        self.cb["SETOFF"]=self.setOff
        self.cb["STATUS"]=self.status
        
        
        zI.__init__(self,address)
        
        #while True:
        #    print("testing")
        #    self.write(":STT?;")
        #    st=self.readline()
        #    print(st)
        
    def write(self,s):
        #print("writing ",s)
        self.uart.write(s)
        time.sleep_ms(100)
    def readline(self):
        self.value_read=self.uart.readline()
        #print("Debug zup",self.value_read)
        if (self.value_read != None):
            self.value_read=self.value_read.decode("utf8")
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
