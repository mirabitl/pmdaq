from machine import Pin,SPI
import framebuf
import time

DC = 8
RST = 12
MOSI = 11
SCK = 10
CS = 9


class OLED_1inch3(framebuf.FrameBuffer):
    def __init__(self):
        self.width = 128
        self.height = 64
        
        self.cs = Pin(CS,Pin.OUT)
        self.rst = Pin(RST,Pin.OUT)
        
        self.cs(1)
        #self.spi = SPI(1)
        #self.spi = SPI(1,2000_000)
        #self.spi = SPI(1,20000_000,polarity=0, phase=0,sck=Pin(SCK),mosi=Pin(MOSI),miso=None)
        self.spi = SPI(1,10000_000,polarity=0, phase=0,sck=Pin(SCK),mosi=Pin(MOSI),miso=None)
        self.dc = Pin(DC,Pin.OUT)
        self.dc(1)
        self.buffer = bytearray((self.height * self.width) // 8)
        self.buffer =bytearray(1024)
        #// 8)
        super().__init__(self.buffer, self.width, self.height, framebuf.MONO_HMSB)
        self.init_display()
        
        self.white =   0xffff
        self.black =   0x0000
        
    def write_cmd(self, cmd):
        self.cs(1)
        self.dc(0)
        self.cs(0)
        self.spi.write(bytearray([cmd]))
        self.cs(1)

    def write_data(self, buf):
        self.cs(1)
        self.dc(1)
        self.cs(0)
        self.spi.write(bytearray([buf]))
        self.cs(1)

    def init_display(self):
        """Initialize dispaly"""  
        self.rst(1)
        time.sleep_ms(100)
        self.rst(0)
        time.sleep_ms(100)
        self.rst(1)
        time.sleep_ms(100)
        
        self.write_cmd(0xAE)#turn off OLED display

        self.write_cmd(0x00)   #set lower column address
        self.write_cmd(0x10)   #set higher column address 

        self.write_cmd(0xB0)   #set page address 
      
        self.write_cmd(0xdc)    #et display start line 
        self.write_cmd(0x00) 
        self.write_cmd(0x81)    #contract control 
        self.write_cmd(0x6f)    #128
        self.write_cmd(0x21)    # Set Memory addressing mode (0x20/0x21) #
    
        self.write_cmd(0xa0)    #set segment remap 
        self.write_cmd(0xc0)    #Com scan direction
        self.write_cmd(0xa4)   #Disable Entire Display On (0xA4/0xA5) 

        self.write_cmd(0xa6)    #normal / reverse
        self.write_cmd(0xa8)    #multiplex ratio 
        self.write_cmd(0x3f)    #duty = 1/64
  
        self.write_cmd(0xd3)    #set display offset 
        self.write_cmd(0x60)

        self.write_cmd(0xd5)    #set osc division 
        self.write_cmd(0x41)
    
        self.write_cmd(0xd9)    #set pre-charge period
        self.write_cmd(0x22)   

        self.write_cmd(0xdb)    #set vcomh 
        self.write_cmd(0x35)  
    
        self.write_cmd(0xad)    #set charge pump enable 
        self.write_cmd(0x8a)    #Set DC-DC enable (a=0:disable; a=1:enable)
        time.sleep_ms(200)
        self.write_cmd(0XAF)    # Not in c code
    def clear(self):
        for i in range(len(self.buffer)):
            self.buffer[i] = 0
        Width = self.width//8
        Height = self.height 
        self.write_cmd(0xb0)    #Set the row  start address
        for j in range(Height):
            column = 63 - j
            self.write_cmd(0x00 + (column & 0x0f))  #Set column low start address
            self.write_cmd(0x10 + (column >> 4))  #Set column higt start address
            for i in range(Width):
                self.write_data(0x00)
                 
    def reverse(self,temp):
        temp = ((temp & 0x55) << 1) | ((temp & 0xaa) >> 1)
        temp = ((temp & 0x33) << 2) | ((temp & 0xcc) >> 2)
        temp = ((temp & 0x0f) << 4) | ((temp & 0xf0) >> 4)  
        return temp;

    def show(self):
        Width = self.width//8
        Height = self.height 
        self.write_cmd(0xb0)
        for page in range(0,Height):
            self.column = 63 - page              
            self.write_cmd(0x00 + (self.column & 0x0f))
            self.write_cmd(0x10 + (self.column >> 4))
            for num in range(0,Width):
                temp =self.reverse(self.buffer[page*Width+num])
                temp=self.buffer[page*Width+num]
                self.write_data(temp)
    def demo(self):
        self.fill(0x0000) 
        self.show()
        self.rect(0,0,128,64,self.white)
        time.sleep(0.5)
        self.show()
        self.rect(10,22,20,20,self.white)
        time.sleep(0.5)
        self.show()
        self.fill_rect(40,22,20,20,self.white)
        time.sleep(0.5)
        self.show()
        self.rect(70,22,20,20,self.white)
        time.sleep(0.5)
        self.show()
        self.fill_rect(100,22,20,20,self.white)
        time.sleep(0.5)
        self.show()
        time.sleep(1)
    
        self.fill(0x0000)
        self.line(0,0,5,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,20,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,35,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,65,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,95,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,125,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,125,41,self.white)
        self.show()
        time.sleep(0.1)
        self.line(0,0,125,21,self.white)
        self.show()
        time.sleep(0.01)
        self.line(0,0,125,3,self.white)
        self.show()
        time.sleep(0.01)
        
        self.line(127,1,125,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,110,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,95,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,65,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,35,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,1,64,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,1,44,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,1,24,self.white)
        self.show()
        time.sleep(0.01)
        self.line(127,1,1,3,self.white)
        self.show()
        time.sleep(1)
        self.fill(0x0000) 
        self.text("128 x 64 Pixels",1,10,self.white)
        self.text("Pico-OLED-1.3",1,27,self.white)
        self.text("SH1107",1,44,self.white)  
        self.show()
        
        time.sleep(1)
        
        self.fill(0xFFFF)

          
if __name__=='__main__':

    OLED = OLED_1inch3()
    OLED.fill(0x0000) 
    OLED.show()
    OLED.rect(0,0,128,64,OLED.white)
    time.sleep(0.5)
    OLED.show()
    OLED.rect(10,22,20,20,OLED.white)
    time.sleep(0.5)
    OLED.show()
    OLED.fill_rect(40,22,20,20,OLED.white)
    time.sleep(0.5)
    OLED.show()
    OLED.rect(70,22,20,20,OLED.white)
    time.sleep(0.5)
    OLED.show()
    OLED.fill_rect(100,22,20,20,OLED.white)
    time.sleep(0.5)
    OLED.show()
    time.sleep(1)
    
    OLED.fill(0x0000)
    OLED.line(0,0,5,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,20,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,35,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,65,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,95,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,125,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,125,41,OLED.white)
    OLED.show()
    time.sleep(0.1)
    OLED.line(0,0,125,21,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(0,0,125,3,OLED.white)
    OLED.show()
    time.sleep(0.01)
    
    OLED.line(127,1,125,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,110,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,95,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,65,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,35,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,1,64,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,1,44,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,1,24,OLED.white)
    OLED.show()
    time.sleep(0.01)
    OLED.line(127,1,1,3,OLED.white)
    OLED.show()
    time.sleep(1)
    OLED.fill(0x0000) 
    OLED.text("128 x 64 Pixels",1,10,OLED.white)
    OLED.text("Pico-OLED-1.3",1,27,OLED.white)
    OLED.text("SH1107",1,44,OLED.white)  
    OLED.show()
    
    time.sleep(1)
    OLED.fill(0x0000) 
    keyA = Pin(15,Pin.IN,Pin.PULL_UP)
    keyB = Pin(17,Pin.IN,Pin.PULL_UP)
    while(1):
        if keyA.value() == 0:
            OLED.fill_rect(0,0,128,20,OLED.white)
            print("A")
        else :
            OLED.fill_rect(0,0,128,20,OLED.black)
            
            
        if(keyB.value() == 0):
            OLED.fill_rect(0,44,128,20,OLED.white)
            print("B")
        else :
            OLED.fill_rect(0,44,128,20,OLED.black)
        OLED.fill_rect(0,22,128,20,OLED.white)
        OLED.text("press the button",0,28,OLED.black)
            
        OLED.show()
    
    
    time.sleep(1)
    OLED.fill(0xFFFF)





