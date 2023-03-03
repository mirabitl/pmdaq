from machine import Pin, I2C
import oled
import time
import bme280
import hih81310
i2c0=I2C(0,sda=Pin(4), scl=Pin(5), freq=100000)    #initializing the I2C method
i2c1=I2C(1,sda=Pin(26), scl=Pin(27), freq=100000)    #initializing the I2C method 
 

oledd = oled.OLED_1inch3()
bme = bme280.BME280(i2c=i2c0)
hih=hih81310.HIH81310(i2c=i2c1)

oledd.fill(0x0000) 
oledd.text("128 x 64 Pixels",1,10,oledd.white)
oledd.text("Pico-OLED-1.3",1,27,oledd.white)
oledd.text("SH1107",1,44,oledd.white)  
oledd.show()
time.sleep(5)

while True:
   

    #oledd.demo()
    print(bme.values)
    oledd.init_display()
    oledd.fill(0)
    #oledd.text("Un essai",5,8)
    #oledd.text("Temp: %s" % str(bme.values[0]), 1, 10,oledd.white)
    #oledd.text("PA: %s" % str(bme.values[1]), 1, 27,oledd.white)
    #oledd.text("Hum: %s" % str(bme.values[2]), 1, 44,oledd.white)
    oledd.show()
    time.sleep(4)
    hum,temp=hih.read_sensor()
    print("HIH %f %f \n" % (hum,temp))
    oledd.fill(0x0000)
    oledd.text("Gas HIH81310", 1, 10,oledd.white)
    oledd.text("Temp: %.1f" % temp, 1,27,oledd.white)
    oledd.text("Hum: %.1f" % hum, 1, 44,oledd.white)
    oledd.show()
    time.sleep(2)
    
