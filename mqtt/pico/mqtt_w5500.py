from umqtt.simple import MQTTClient
from usocket import socket
from machine import Pin,SPI,ADC,I2C
import network
import time
import oled
import bme280
import hih81310
import json
from writer import Writer
import freesans20  # Font to use

#mqtt config
mqtt_server = '192.168.11.1'
client_id = 'wiz'
topic_pub = b'hello'
topic_msg = b'Hello Pico'

last_message = 0
message_interval = 5
counter = 0

#W5x00 chip init
def w5x00_init():
    spi=SPI(0,2_000_000, mosi=Pin(19),miso=Pin(16),sck=Pin(18))
    nic = network.WIZNET5K(spi,Pin(17),Pin(20)) #spi,cs,reset pin
    nic.active(True)
    
    #None DHCP
    nic.ifconfig(('192.168.11.2','255.255.255.0','192.168.11.1','8.8.8.8'))
    
    #DHCP
    #nic.ifconfig('dhcp')
    print('IP address :', nic.ifconfig())
    
    while not nic.isconnected():
        print("waiting for connection ...")
        time.sleep(1)
        #print(nic.regs())

def oled_init():
    oledd = oled.OLED_1inch3()
    oledd.clear()
    time.sleep_ms(100)

    oledd.fill(0)
    wri = Writer(oledd, freesans20)  # verbose = False to suppress console output
    Writer.set_textpos(oledd, 0, 0)  # In case a previous test has altered this
    wri.printstring('PM Slow\nInitialising')
    #ledd.text("128 x 64 Pixels",1,10,oledd.white)
    #oledd.text("Pico-OLED-1.3",1,27,oledd.white)
    #oledd.text("SH1107",1,44,oledd.white)  
    oledd.show()
    time.sleep(1)
    #wri = Writer(oledd, freesans20)  # verbose = False to suppress console output
    #Writer.set_textpos(oledd, 0, 0)  # In case a previous test has altered this
    #wri.printstring('Sunday\n12 Aug 2018\n10.30am')
    #oledd.show()
    #time.sleep(5)

    return (oledd,wri)

def bme_init():
    i2c0=I2C(0,sda=Pin(4), scl=Pin(5), freq=100000)    #initializing the I2C method
    bme = bme280.BME280(i2c=i2c0)
    return bme

def hih_init():
    i2c1=I2C(1,sda=Pin(26), scl=Pin(27), freq=100000)    #initializing the I2C method 
    hih=hih81310.HIH81310(i2c=i2c1)
    return hih

def mqtt_connect():
    client = MQTTClient(client_id, mqtt_server, keepalive=60)
    client.connect()
    print('Connected to %s MQTT Broker'%(mqtt_server))
    return client

#reconnect & reset
def reconnect():
    print('Failed to connected to MQTT Broker. Reconnecting...')
    time.sleep(2)
    #machine.reset()

def internal_temperature():
    conversion_factor = 3.3 / (65535)
    sensor_temp = ADC(4)
    reading = sensor_temp.read_u16() * conversion_factor 
    temperature = 27 - (reading - 0.706)/0.001721
    return temperature

def main():
    oledd,wri=oled_init()
    w5x00_init()
    time.sleep(2)
    client=None
    while True:
        try:
            client = mqtt_connect()
            break
        except OSError as e:
            reconnect()
 


    hih=hih_init()
    bme=bme_init()
    while True:
        # Internal temperature
        temp=internal_temperature()
        print(temp)
        res={}
        res["T"]=temp
        topic_pub = b'pico_w5500/rp2040'
        tmsg=json.dumps(res)
        client.publish(topic_pub, tmsg.encode("utf8"))
        oledd.clear()
        oledd.fill(0)
        oledd.show()
        time.sleep_ms(100)
        #stext='T2040: %.1f' % temp
        #st1l="{:<512}".format(stext)
        #oledd.text(st1l.encode("utf8"), 1, 10,oledd.white)
        Writer.set_textpos(oledd, 0, 0)
        wri.printstring('RP2040\nT: %.1f C' % temp)
        oledd.show()
        time.sleep(2)
        # bme data
        t, p, h = bme.read_hrvalues()
        res={}
        res["T"]=t
        res["P"]=p
        res["H"]=h
        print(bme.values)
        oledd.clear()
        oledd.fill(0)
        oledd.show()
        time.sleep_ms(100)
        #st1="T:%.1f H:%.1f" % (t,h)
        #st1l="{:<512}".format(st1)
        #oledd.text(st1l, 1, 10,oledd.white)
        #oledd.show()
        #time.sleep_ms(100)
        #st1="P: %.1f" % (p)
        #st1l="{:<512}".format(st1)
        #oledd.text(st1l.encode("utf8"), 1, 27,oledd.white)
        Writer.set_textpos(oledd, 0, 0)
        wri.printstring('H: %.1f \nP: %.1f hPa\nT: %.1f C' % (h,p,t))
        oledd.show()
        topic_pub = b'pico_w5500/bme280'
        tmsg=json.dumps(res)
        client.publish(topic_pub, tmsg.encode("utf8"))
        time.sleep(3)
        # HIH data
        hum,temp=hih.read_sensor()
        print("HIH %f %f \r" % (hum,temp))
        oledd.clear()
        oledd.fill(0)
        oledd.show()
        time.sleep_ms(100)
        #st1="T:%.1f H:%.1f" % (temp,hum)
        #st1l="{:<512}".format(st1)
        
        
        #oledd.text(st1l.encode("utf8"), 1,10,oledd.white)
        Writer.set_textpos(oledd, 0, 0)
        wri.printstring('HIH\nH: %.1f \nT: %.1f C' % (hum,temp))
        oledd.show()
        res={}
        res["T"]=temp
        res["H"]=hum
        topic_pub = b'pico_w5500/hih'
        tmsg=json.dumps(res)
        client.publish(topic_pub, tmsg.encode("utf8"))
        time.sleep(3)
    client.disconnect()

if __name__ == "__main__":
    main()


