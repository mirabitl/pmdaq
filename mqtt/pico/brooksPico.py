import time
from array import array
from machine import Pin,UART
from brooksInterface2 import abstractBrooks as bI
"""
Doc a faire
""" 
class  brooksPico(bI):
 
    def __init__(self,uart_nb,tx_pin,rx_pin,device_id=0,baud=19200,rst_pin=4,
                 **kwargs):
        self.device_id = device_id
        self.uart = UART(uart_nb, baudrate=baud, tx=Pin(tx_pin), rx=Pin(rx_pin))
        self.uart.init(bits=8, parity=1, stop=1,timeout=1000) 
        # Access the board
        self.value_read="NONE"
        self.cb={}
        self.cb["SETFLOW"]=self.set_flow
        self.cb["STATUS"]=self.status
        self.cb["VIEW"]=self.view
        self.rst = Pin(rst_pin,mode=Pin.OUT)
        self.rst.off()
        
        bI.__init__(self,device_id)
        
    def view(self):
        return {"id":"brooks","cmds":list(self.cb.keys())}            
    def writeCommand(self,cmd,tempo=0.01):
        #print("calling ",cmd)
        tmax=len(cmd)*10/19200.
        if (self.DEBUG):
            s=""
            for i in range(len(cmd)):
                s=s+"%.2x " % cmd[i]
        
            print("calling ",tmax,s)
        
        self.rst.on();
        n=self.uart.write(cmd);
        time.sleep(tmax*1.25);
        self.rst.off()
        

    def read_one_byte(self):
        return self.uart.read(1)

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
